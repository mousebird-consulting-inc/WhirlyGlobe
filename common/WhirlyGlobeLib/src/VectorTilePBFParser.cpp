/*  VectorTilePBFParser.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 10/30/20.
 *  Copyright 2011-2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "VectorTilePBFParser.h"

#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "VectorObject.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"

#import "vector_tile.pb.h"
#import "maply_pb_decode.h"

#import <vector>
#import <string>

namespace WhirlyKit
{

namespace {
    // Fixed strings to avoid allocations on every call to, e.g., Dictionary::setString
    const std::string layerNameKey("layer_name");       //NOLINT
    const std::string geometryTypeKey("geometry_type"); //NOLINT
    const std::string layerOrderKey("layer_order");     //NOLINT
}

const vector_tile_Tile_Layer VectorTilePBFParser::_defaultLayer = {
    /* name       */ { &VectorTilePBFParser::stringDecode,    nullptr },
    /* features   */ { &VectorTilePBFParser::featureDecode,   nullptr },
    /* keys       */ { &VectorTilePBFParser::stringVecDecode, nullptr },
    /* values     */ { &VectorTilePBFParser::valueVecDecode,  nullptr },
    /* has_extent */ false,
    /* extent     */ 0,
    /* version    */ 0,
    /* extensions */ nullptr,
};

const vector_tile_Tile_Feature VectorTilePBFParser::_defaultFeature = {
    /* has_id   */ false,
    /* id       */ 0LL,
    /* tags     */ { &VectorTilePBFParser::intVecDecode, nullptr },
    /* has_type */ false,
    /* type     */ vector_tile_Tile_GeomType_UNKNOWN,
    /* geometry */ { &VectorTilePBFParser::intVecDecode, nullptr },
};

const vector_tile_Tile_Value VectorTilePBFParser::_defaultValue = {
    /* pb_callback_t string_value */ { &VectorTilePBFParser::stringDecode, nullptr },
    /* bool has_float_value       */ false,
    /* float float_value          */ 0.0f,
    /* bool has_double_value      */ false,
    /* double double_value        */ 0.0,
    /* bool has_int_value         */ false,
    /* int64_t int_value          */ 0LL,
    /* bool has_uint_value        */ false,
    /* uint64_t uint_value        */ 0ULL,
    /* bool has_sint_value        */ false,
    /* int64_t sint_value         */ 0LL,
    /* has_bool_value             */ false,
    /* bool_value                 */ false,
    /* extensions                 */ nullptr,
};


VectorTilePBFParser::VectorTilePBFParser(
        VectorTileData *tileData,
        VectorStyleDelegateImpl* styleData,
        PlatformThreadInfo *styleInst,
        const std::string &uuidName,
        const std::set<std::string> &uuidValues,
        std::map<SimpleIdentity, std::vector<VectorObjectRef>*>& vecObjByStyle,
        bool localCoords,
        bool parseAll,
        std::vector<VectorObjectRef>* keepVectors,
        CancelFunction isCanceled)
    : _tileData      (tileData)
    , _styleDelegate (styleData)
    , _styleInst     (styleInst)
    , _uuidName      (uuidName)
    , _uuidValues    (uuidValues)
    , _vecObjByStyle (vecObjByStyle)
    , _localCoords   (localCoords)
    , _parseAll      (parseAll)
    , _keepVectors   (keepVectors)
    , _checkCancelled(std::move(isCanceled))
    , _bbox          (tileData->bbox)
    , _bboxWidth     (_bbox.ur().x() - _bbox.ll().x())
    , _bboxHeight    (_bbox.ur().y() - _bbox.ll().y())
    , _sx            ((_bboxWidth > 0) ? (TileSize / _bboxWidth) : 0)
    , _sy            ((_bboxHeight > 0) ? (TileSize / _bboxHeight) : 0)
    , _tileOriginX   (tileData->bbox.ll().x())
    , _tileOriginY   (tileData->bbox.ur().y())
{
}

bool VectorTilePBFParser::parse(const uint8_t* data, size_t length)
{
    _vector_tile_Tile tile = {
        /* layer     */ { layerDecode, this },
        /*extensions */ nullptr,
    };

    auto stream = pb_istream_from_buffer(data, length);
    if (!pb_decode(&stream, vector_tile_Tile_fields, &tile))
    {
        _parseError = stream.errmsg ? stream.errmsg : std::string();
        return false;
    }

    return true;
}

// Tile contains a collection of Layers
bool VectorTilePBFParser::layerDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    return (*(This**)arg)->layerDecode(stream, field);
}

bool VectorTilePBFParser::layerDecode(pb_istream_t *stream, const pb_field_iter_t *field)
{
    if (_checkCancelled(_styleInst))
    {
        _wasCancelled = true;
        return false;
    }

    vector_tile_Tile_Layer layer = _defaultLayer;
    std::string_view layerNameView;

    layer.name.arg = &layerNameView;
    layer.features.arg = this;
    layer.keys.arg = &_layerKeys;
    layer.values.arg = &_layerValues;

    const auto layerBytes = (uint32_t)stream->bytes_left;
    _layerKeys.clear();
    _layerKeys.reserve(layerKeyHeuristic(layerBytes));
    _layerValues.clear();
    _layerValues.reserve(layerValueHeuristic(layerBytes));
    _featureTags.clear();
    _featureTags.reserve(featureTagHeuristic(layerBytes));
    _featureGeometry.clear();
    _featureGeometry.reserve(featureGeometryHeuristic(layerBytes));
    _features.clear();
    _features.reserve(featureHeuristic(layerBytes));

    if (!pb_decode(stream, vector_tile_Tile_Layer_fields, &layer))
    {
        return false;
    }

    auto layerName = std::string(layerNameView);

    // When `has_extent` is false, nanopb sets the default in `extent`
    _layerScale = (double)layer.extent / TileSize;

    // Prevent a divide-by-zero, or negative scales
    if (_layerScale <= 0)
    {
        wkLogLevel(Warn, "VectorTilePBFParser: Invalid layer extent (%s / %d / %d)",
                   layerName.c_str(), layer.extent, TileSize);
        _skippedLayerCount += 1;
        return true;
    }

    // if we don't have any styles for a layer, don't bother parsing the features
    bool found = false;
    if (_styleDelegate->layerShouldDisplay(_styleInst, layerName, _tileData->ident))
        found = true;
    else {
        // Try a lowercase version
        // TODO: This doesn't handle non-ASCII well
        std::string lowerLayerName = layerName;
        std::transform(lowerLayerName.begin(), lowerLayerName.end(), lowerLayerName.begin(),
                                   [](unsigned char c){ return std::tolower(c); });
        
        if (lowerLayerName != layerName)
            if (_styleDelegate->layerShouldDisplay(_styleInst, lowerLayerName, _tileData->ident)) {
                found = true;
                layerName = lowerLayerName;
            }
    }
    
    if (!found) {
        _skippedLayerCount += 1;
        return true;
    }

    size_t prevTagIndex = 0;
    size_t prevGeomIndex = 0;
    for (auto const &feature : _features)
    {
        if (_checkCancelled(_styleInst))
        {
            _wasCancelled = true;
            return false;
        }

        auto attributes = std::make_shared<MutableDictionaryC>();
        attributes->setString(layerNameKey, layerName);
        attributes->setInt(geometryTypeKey, (int)feature.geomType);
        attributes->setInt(layerOrderKey, (int)_layerCount);

        const bool tagsOk = processTags(attributes, prevTagIndex, prevGeomIndex, feature);
        //const auto curTagIndex = prevTagIndex;
        const auto curGeomIndex = prevGeomIndex;
        const auto curGeomCount = feature.geomIndex - prevGeomIndex;
        prevTagIndex = feature.tagIndex;
        prevGeomIndex = feature.geomIndex;
        
        if (!tagsOk)
        {
            _skippedFeatureCount += 1;
            continue;
        }

        SimpleIDUSet styleIDs(featureStyleHeuristic());
        if (!checkStyles(styleIDs, attributes, layerName))
        {
            // Skip this feature
            _skippedFeatureCount += 1;
            continue;
        }

        _featureCount += 1;

        auto vecObj = std::make_shared<VectorObject>();

        // Parse geometry
        try
        {
            switch (feature.geomType)
            {
                case GeomTypeLineString:
                    parseLineString(&_featureGeometry[curGeomIndex], curGeomCount, vecObj->shapes);
                    break;
                case GeomTypePolygon:
                {
                    auto shape = VectorAreal::createAreal();
                    if (parsePolygon(&_featureGeometry[curGeomIndex], curGeomCount, *shape))
                    {
                        vecObj->shapes.insert(shape);
                    }
                    break;
                }
                case GeomTypePoint:
                {
                    auto shape = VectorPoints::createPoints();
                    if (parsePoints(&_featureGeometry[curGeomIndex], curGeomCount, *shape))
                    {
                        vecObj->shapes.insert(shape);
                    }
                    break;
                }
                default:
                case GeomTypeUnknown:
#if DEBUG
                    wkLogLevel(Warn, "VectorTilePBFParser: Unknown geometry type %d", feature.geomType);
#endif
                    _unknownGeomTypes += 1;
                    break;
            }
        }
        catch (const std::exception &ex)
        {
            wkLogLevel(Error, "VectorTilePBFParser: Vector Parsing Error: %s", ex.what());
            _parseErrors += 1;
            vecObj.reset();
        }
        catch (...)
        {
            wkLogLevel(Error, "VectorTilePBFParser: Vector Parsing Error: ?");   // Bad, don't throw non-exceptions!
            _parseErrors += 1;
            vecObj.reset();
        }

        for (const auto &shape: vecObj->shapes)
        {
            shape->setAttrDict(attributes);
        }

        addFeature(vecObj, styleIDs);
    }
    _layerCount += 1;

    return true;
}

/// https://github.com/mapbox/vector-tile-spec/tree/master/2.1/#432-parameter-integers
/// A ParameterInteger is zigzag encoded so that small negative and positive values are both encoded as small integers.
int32_t VectorTilePBFParser::decodeParamInt(int32_t p) {
    return ((p >> 1U) ^ (-(p & 1U)));       //NOLINT these are right out of the spec
}

/// https://github.com/mapbox/vector-tile-spec/tree/master/2.1/#431-command-integers
/// A command ID is encoded as an unsigned integer in the least significant 3 bits of the CommandInteger, and is in the range 0 through 7, inclusive.
/// A command count is encoded as an unsigned integer in the remaining 29 bits of a CommandInteger, and is in the range 0 through pow(2, 29) - 1, inclusive.
std::pair<uint8_t, int32_t> VectorTilePBFParser::decodeCommand(int32_t c) {
    return std::make_pair(c & ((1 << CmdBits) - 1), c >> CmdBits);  //NOLINT these are right out of the spec
}

// Layer contains a collection of Features
bool VectorTilePBFParser::featureDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    return (*(This**)arg)->featureDecode(stream, field);
}

bool VectorTilePBFParser::featureDecode(pb_istream_t *stream, const pb_field_iter_t *field)
{
    auto feature = _defaultFeature;
    feature.tags.arg = &_featureTags;
    feature.geometry.arg = &_featureGeometry;

    if (!pb_decode(stream, vector_tile_Tile_Feature_fields, &feature))
    {
        return false;
    }

    const auto geomType = static_cast<MapnikGeometryType>(feature.type);
    _features.emplace_back(_featureTags.size(),_featureGeometry.size(),geomType);

    return true;
}

bool VectorTilePBFParser::processTags(const MutableDictionaryCRef &attributes, size_t tagIdx, size_t geomIdx, const Feature &feature)
{
    const auto tagCount = feature.tagIndex - tagIdx;
    if (tagCount % 2 != 0)
    {
        wkLogLevel(Warn, "VectorTilePBFParser: Odd feature tags!");
    }

    for (size_t m = tagIdx; m + 1 < feature.tagIndex; m += 2)
    {
        const auto keyIndex = _featureTags[m];
        const auto valueIndex = _featureTags[m + 1];

        if (keyIndex >= _layerKeys.size() || valueIndex >= _layerValues.size()) {
            wkLogLevel(Warn, "VectorTilePBFParser: Invalid feature tag %d/%d (%d/%d)", keyIndex, valueIndex, (int)_layerKeys.size(), (int)_layerValues.size());
            _badAttributes += 1;
            continue;
        }

        const auto &key = _layerKeys[keyIndex];
        if (key.empty()) {
            continue;
        }

        // TODO: We don't really need transient string allocations here
        const auto skey = std::string(key);

        const auto &value = _layerValues[valueIndex];
        switch (value.type) {
            case SmallValue::SmallValString: attributes->setString(skey, std::string(value.stringValue)); break;
            case SmallValue::SmallValFloat:  attributes->setDouble(skey, value.floatValue); break;
            case SmallValue::SmallValDouble: attributes->setDouble(skey, value.doubleValue); break;
            case SmallValue::SmallValInt:    attributes->setInt(skey, value.intValue); break;
            case SmallValue::SmallValUInt:   attributes->setInt(skey, (int)value.uintValue); break;
            case SmallValue::SmallValSInt:   attributes->setInt(skey, (int)value.sintValue); break;
            case SmallValue::SmallValBool:   attributes->setInt(skey, (int)value.boolValue); break;
            default:
            case SmallValue::SmallValNone:
                _unknownValueTypes += 1;
                wkLogLevel(Warn, "VectorTilePBFParser: Invalid Value Type %d", value.type);
                break;
        }
    }
    
    return true;
}

bool VectorTilePBFParser::checkStyles(SimpleIDUSet& styleIDs, const MutableDictionaryCRef &attributes, const std::string &layerName)
{
    // Ask for the styles that correspond to this feature
    // If there are none, we can skip this.
    
    // Do a quick inclusion check
    if (!_uuidName.empty())
    {
        std::string uuidVal = attributes->getString(_uuidName); // TODO: extra string copy
        if (_uuidValues.find(uuidVal) == _uuidValues.end())
        {
            // Skip this feature
            return false;
        }
    }
    
    // TODO: populate a reused vector?
    const auto styles = _styleDelegate->stylesForFeature(_styleInst, *attributes, _tileData->ident, layerName);
    for (const auto &style : styles)
    {
        styleIDs.insert(style->getUuid(_styleInst));
    }
    
    return (!styleIDs.empty() || _parseAll);
}

void VectorTilePBFParser::parseLineString(const uint32_t *geometry, size_t geomCount, ShapeSet& shapes) const
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    Point2f firstCoord;
    VectorLinearRef lin;
    
    for (int k = 0; k < geomCount; )
    {
        // length is the number of coordinates before the CMD changes
        if (!length)
        {
            std::tie(cmd, length) = decodeCommand(geometry[k++]);
        }
        if (length > 0)
        {
            length -= 1;
            if (cmd == SEG_MOVETO || cmd == SEG_LINETO)
            {
                const auto dx = decodeParamInt(geometry[k++]);
                const auto dy = decodeParamInt(geometry[k++]);
                
                x += (static_cast<double>(dx) / _layerScale);
                y += (static_cast<double>(dy) / _layerScale);
                
                // At this point x/y is a coord encoded in tile coord space, from 0 to TILE_SIZE
                // Convert to epsg:3785, then to degrees, then to radians
                point = Point2f((_tileOriginX + x / _sx),(_tileOriginY - y / _sy));
                
                if (!_localCoords) {
                    point.x() = DegToRad(point.x() / MAX_EXTENT * 180.0);
                    point.y() = 2 * atan(exp(DegToRad(point.y() / MAX_EXTENT * 180.0))) - M_PI_2;
                }

                if (cmd == SEG_MOVETO)  // move-to means we are starting a new segment
                {
                    if (lin && !lin->pts.empty())  // We've already got a line, finish it
                    {
                        lin->initGeoMbr();
                        shapes.insert(lin);
                    }
                    lin = VectorLinear::createLinear();
                    lin->pts.reserve(length);
                    firstCoord = point;
                }

                lin->pts.emplace_back(point.x(),point.y());
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
                if (!lin->pts.empty())  //We've already got a line, finish it
                {
                    lin->pts.emplace_back(firstCoord.x(),firstCoord.y());
                    lin->initGeoMbr();
                    shapes.insert(lin);
                    lin.reset();
                }
#if DEBUG
                else
                {
                    wkLogLevel(Warn, "VectorTilePBFParser: Close line command with no points");
                }
#endif
            }
#if DEBUG
            else
            {
                wkLogLevel(Warn, "VectorTilePBFParser: Unknown command %d", cmd);
            }
#endif
        }
    }
    
    if (lin && !lin->pts.empty())
    {
        lin->initGeoMbr();
        shapes.insert(lin);
    }
}

bool VectorTilePBFParser::parsePolygon(const uint32_t *geometry, size_t geomCount, VectorAreal& shape)
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    Point2f firstCoord(0, 0);
    VectorRing ring;
    
    for (int k = 0; k < geomCount; )
    {
        if (!length)
        {
            std::tie(cmd, length) = decodeCommand(geometry[k++]);
        }
        
        if (length > 0)
        {
            length -= 1;
            if (cmd == SEG_MOVETO || cmd == SEG_LINETO)
            {
                const auto dx = decodeParamInt(geometry[k++]);
                const auto dy = decodeParamInt(geometry[k++]);
                
                x += (static_cast<double>(dx) / _layerScale);
                y += (static_cast<double>(dy) / _layerScale);
                
                // At this point x/y is a coord is encoded in tile coord space, from 0 to TILE_SIZE
                // Convert to epsg:3785, then to degrees, then to radians
                point = Point2f((_tileOriginX + x / _sx), (_tileOriginY - y / _sy));
                
                if (!_localCoords)
                {
                    point.x() = DegToRad((point.x() / MAX_EXTENT) * 180.0);
                    point.y() = 2 * atan(exp(DegToRad((point.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                }

                if (cmd == SEG_MOVETO)  //move to means we are starting a new segment
                {
                    firstCoord = point;
                    //TODO: does this ever happen when we are part way through a shape? holes?
                }
                
                ring.emplace_back(point.x(),point.y());
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
                if (!ring.empty())  //We've already got a line, finish it
                {
                    ring.emplace_back(firstCoord.x(),firstCoord.y()); //close the loop
                    shape.loops.push_back(ring); //add loop to shape
                    ring.clear(); //reuse the ring
                }
            }
            else
            {
                _unknownCommands += 1;
            }
        }
    }
    
#if DEBUG
    if (!ring.empty())
    {
        wkLogLevel(Warn, "VectorTilePBFParser: Finished polygon loop, and ring has %d points", (int)ring.size());
    }
#endif
    //TODO: Is there a possibility of still having a ring here that hasn't been added by a close command?
    
    if (!shape.loops.empty())
    {
        shape.initGeoMbr();
        return true;
    }
    return false;
}

bool VectorTilePBFParser::parsePoints(const uint32_t *geometry, size_t geomCount, VectorPoints& shape)
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    
    for (int k = 0; k < geomCount; )
    {
        if (!length)
        {
            std::tie(cmd, length) = decodeCommand(geometry[k++]);
        }
        
        if (length > 0)
        {
            length -= 1;
            if (cmd == SEG_MOVETO || cmd == SEG_LINETO)
            {
                const auto dx = decodeParamInt(geometry[k++]);
                const auto dy = decodeParamInt(geometry[k++]);
                
                x += (static_cast<double>(dx) / _layerScale);
                y += (static_cast<double>(dy) / _layerScale);
                
                // At this point x/y is a coord is encoded in tile coord space, from 0 to TILE_SIZE
                // Covert to epsg:3785, then to degrees, then to radians
                if (x > 0 && x < TileSize && y > 0 && y < TileSize)
                {
                    point = Point2f((_tileOriginX + x / _sx), (_tileOriginY - y / _sy));
                    
                    if (!_localCoords)
                    {
                        point.x() = DegToRad((point.x() / MAX_EXTENT) * 180.0);
                        point.y() = 2 * atan(exp(DegToRad((point.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                    }
                    shape.pts.emplace_back(point.x(),point.y());
                }
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
#if DEBUG
                wkLogLevel(Warn, "VectorTilePBFParser: Close point feature?");
#endif
            }
            else
            {
                _unknownCommands += 1;
            }
        }
    }
    
    if (!shape.pts.empty())
    {
        shape.initGeoMbr();
        return true;
    }
    return false;
}

void VectorTilePBFParser::addFeature(const VectorObjectRef &vecObj, const SimpleIDUSet &styleIDs)
{
    if (vecObj->shapes.empty())
    {
        return;
    }
    
    if (_keepVectors)
    {
        _keepVectors->push_back(vecObj);
    }

    // Sort this vector object into the styles that will process it
    for (auto styleID : styleIDs)
    {
        // Find or insert key
        const auto ip = _vecObjByStyle.insert(std::make_pair(styleID, nullptr));

        // If we inserted (or somehow there was a null entry...), initialize it
        auto *&vecs = ip.first->second;
        if (!vecs)
        {
            vecs = new std::vector<VectorObjectRef>();
            //TODO: ip.first->second.reserve(?)
        }
        
        vecs->push_back(vecObj);
    }
}
    
// Layer contains a collection of Values
bool VectorTilePBFParser::valueVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
    auto &vec = **(std::vector<SmallValue>**)arg;
    
    std::string_view string;
    vector_tile_Tile_Value value = vector_tile_Tile_Value_init_zero;
    value.string_value.funcs.decode = &stringDecode;
    value.string_value.arg = &string;

    if (!pb_decode(stream, vector_tile_Tile_Value_fields, &value))
    {
        return false;
    }

         if (value.has_float_value)  vec.push_back({{.floatValue  = value.float_value},  SmallValue::SmallValFloat});
    else if (value.has_double_value) vec.push_back({{.doubleValue = value.double_value}, SmallValue::SmallValDouble});  //NOLINT
    else if (value.has_int_value)    vec.push_back({{.intValue    = value.int_value},    SmallValue::SmallValInt});     //NOLINT
    else if (value.has_uint_value)   vec.push_back({{.uintValue   = value.uint_value},   SmallValue::SmallValUInt});    //NOLINT
    else if (value.has_sint_value)   vec.push_back({{.sintValue   = value.sint_value},   SmallValue::SmallValSInt});    //NOLINT
    else if (value.has_bool_value)   vec.push_back({{.boolValue   = value.bool_value},   SmallValue::SmallValBool});    //NOLINT
    else                             vec.push_back({{.stringValue = string},             SmallValue::SmallValString});  //NOLINT

    return true;
}

// Decode a string into a variable
bool VectorTilePBFParser::stringDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    *((std::string_view*)*arg) = std::string_view((char*)stream->state, stream->bytes_left);
    return true;
}

// Decode a single string into a vector
bool VectorTilePBFParser::stringVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    auto &vec = **(std::vector<std::string_view>**)arg;
    vec.emplace_back((char*)stream->state, stream->bytes_left);
    return true;
}

// Decode a repeated-integer into a vector
bool VectorTilePBFParser::intVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    auto &vec = **(std::vector<uint32_t>**)arg;
    if (vec.empty())
    {
        vec.reserve(vec.size() + stream->bytes_left);
    }
    while (stream->bytes_left)
    {
        uint64_t value;
        if (!pb_decode_varint(stream, &value))
        {
            return false;
        }
        vec.push_back((uint32_t)value);
    }
    return true;
}

}   // namespace WhirlyKit

