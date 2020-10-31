//
//  VectorTilePBFParser.cpp
//  WhirlyGlobeMaplyComponent
//
//  Created by Tim Sylvester on 10/30/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#include "VectorTilePBFParser.h"

#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "VectorObject.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"

#import "vector_tile.pb.h"
#import "pb_decode.h"

#import <vector>
#import <string>

namespace WhirlyKit
{

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
        std::vector<VectorObjectRef>* keepVectors)
    : _tileData     (tileData)
    , _styleDelegate(styleData)
    , _styleInst    (styleInst)
    , _uuidName     (uuidName)
    , _uuidValues   (uuidValues)
    , _vecObjByStyle(vecObjByStyle)
    , _localCoords  (localCoords)
    , _parseAll     (parseAll)
    , _keepVectors  (keepVectors)
    , _bbox         (tileData->bbox)
    , _bboxWidth    (_bbox.ur().x() - _bbox.ll().x())
    , _bboxHeight   (_bbox.ur().y() - _bbox.ll().y())
    , _sx           ((_bboxWidth > 0) ? (TileSize / _bboxWidth) : 0)
    , _sy           ((_bboxHeight > 0) ? (TileSize / _bboxHeight) : 0)
    , _tileOriginX  (tileData->bbox.ll().x())
    , _tileOriginY  (tileData->bbox.ur().y())
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
    vector_tile_Tile_Layer layer = _defaultLayer;
    _currentLayer = &layer;
    
    layer.name.arg = &_layerNameView;
    layer.features.arg = this;
    layer.keys.arg = &_layerKeys;
    layer.values.arg = &_layerValues;
    
    _layerStarted = false;
    if (!pb_decode(stream, vector_tile_Tile_Layer_fields, &layer))
    {
        return false;
    }

    return layerFinish();
}

/// https://github.com/mapbox/vector-tile-spec/tree/master/2.1/#432-parameter-integers
/// A ParameterInteger is zigzag encoded so that small negative and positive values are both encoded as small integers.
int32_t VectorTilePBFParser::decodeParamInt(int32_t p) {
    return ((p >> 1) ^ (-(p & 1)));
}

/// https://github.com/mapbox/vector-tile-spec/tree/master/2.1/#431-command-integers
/// A command ID is encoded as an unsigned integer in the least significant 3 bits of the CommandInteger, and is in the range 0 through 7, inclusive.
/// A command count is encoded as an unsigned integer in the remaining 29 bits of a CommandInteger, and is in the range 0 through pow(2, 29) - 1, inclusive.
std::pair<uint8_t, int32_t> VectorTilePBFParser::decodeCommand(int32_t c) {
    return std::make_pair(c & ((1 << CmdBits) - 1), c >> CmdBits);
}

// Layer contains a collection of Features
bool VectorTilePBFParser::featureDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg)
{
    return (*(This**)arg)->featureDecode(stream, field);
}

bool VectorTilePBFParser::featureDecode(pb_istream_t *stream, const pb_field_iter_t *field)
{
    layerElement();

    if (_skipLayer)
    {
        // We're skipping this layer, so don't process any of the features.
        // We still have to read past them, of course.
        _skippedFeatureCount += 1;
        return true;
    }
    
    // todo: see if we have enough info to make better guesses here
    _featureTags.clear();
    _featureTags.reserve(20);
    _featureGeometry.clear();
    _featureGeometry.reserve(100);
    
    auto feature = _defaultFeature;
    feature.tags.arg = &_featureTags;
    feature.geometry.arg = &_featureGeometry;

    if (!pb_decode(stream, vector_tile_Tile_Feature_fields, &feature))
    {
        return false;
    }
    
    const auto geomType = static_cast<MapnikGeometryType>(feature.type);

    auto attributes = std::make_shared<MutableDictionaryC>();
    attributes->setString("layer_name", _layerName);
    attributes->setInt("geometry_type", (int)geomType);
    attributes->setInt("layer_order", _layerCount);

    _layerCount += 1;
    
    if (!processTags(attributes))
    {
        _skippedFeatureCount += 1;
        return true;
    }
    
    SimpleIDSet styleIDs;
    if (!checkStyles(styleIDs, attributes))
    {
        // Skip this feature
        _skippedFeatureCount += 1;
        return true;
    }

    _featureCount += 1;

    auto vecObj = std::make_shared<VectorObject>();
    
    // Parse geometry
    try
    {
        switch (geomType)
        {
            case GeomTypeLineString:
                parseLineString(_featureGeometry, vecObj->shapes);
                break;
            case GeomTypePolygon:
            {
                auto shape = VectorAreal::createAreal();
                if (parsePolygon(_featureGeometry, *shape))
                {
                    vecObj->shapes.insert(shape);
                }
                break;
            }
            case GeomTypePoint:
            {
                auto shape = VectorPoints::createPoints();
                if (parsePoints(_featureGeometry, *shape))
                {
                    vecObj->shapes.insert(shape);
                }
                break;
            }
            default:
            case GeomTypeUnknown:
#if DEBUG
                wkLogLevel(Warn, "Unknown geometry type %d", geomType);
#endif
                _unknownGeomTypes += 1;
                break;
        }
    }
    catch (const std::exception &ex)
    {
        wkLogLevel(Error, "Vector Parsing Error: %s", ex.what());
        _parseErrors += 1;
        vecObj.reset();
    }
    catch (...)
    {
        wkLogLevel(Error, "Vector Parsing Error: ?");   // Bad, don't throw non-exceptions!
        _parseErrors += 1;
        vecObj.reset();
    }
    
    for (const auto &shape: vecObj->shapes)
    {
        shape->setAttrDict(attributes);
    }

    addFeature(vecObj, styleIDs);

    _featureTags.clear();
    _featureGeometry.clear();

    return true;
}

bool VectorTilePBFParser::processTags(const MutableDictionaryCRef &attributes)
{
    if (_featureTags.size() % 2 != 0)
    {
        wkLogLevel(Warn, "Odd feature tags!");
    }

    for (int m = 0; m + 1 < _featureTags.size(); m += 2)
    {
        const auto keyIndex = _featureTags[m];
        const auto valueIndex = _featureTags[m + 1];
        
        if (keyIndex >= _layerKeys.size() || valueIndex >= _layerValues.size()) {
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
                wkLogLevel(Warn, "Invalid Value Type %d", value.type);
                break;
        }
    }
    
    return true;
}

bool VectorTilePBFParser::checkStyles(SimpleIDSet& styleIDs, const MutableDictionaryCRef &attributes)
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
    const auto styles = _styleDelegate->stylesForFeature(_styleInst, attributes, _tileData->ident, _layerName);
    for (const auto &style : styles)
    {
        styleIDs.insert(style->getUuid(_styleInst));
    }
    
    return (!styleIDs.empty() || _parseAll);
}

void VectorTilePBFParser::parseLineString(const std::vector<uint32_t> &geometry, ShapeSet& shapes)
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    Point2f firstCoord;
    VectorLinearRef lin;
    
    for (int k = 0; k < geometry.size(); )
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
                    if (lin && lin->pts.size() > 0)  // We've already got a line, finish it
                    {
                        lin->initGeoMbr();
                        shapes.insert(lin);
                    }
                    lin = VectorLinear::createLinear();
                    lin->pts.reserve(length);
                    firstCoord = point;
                }
                
                lin->pts.push_back(Point2f(point.x(),point.y()));
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
                if (lin->pts.size() > 0)  //We've already got a line, finish it
                {
                    lin->pts.push_back(Point2f(firstCoord.x(),firstCoord.y()));
                    lin->initGeoMbr();
                    shapes.insert(lin);
                    lin.reset();
                }
#if DEBUG
                else
                {
                    wkLogLevel(Warn, "Close line command with no points");
                }
#endif
            }
#if DEBUG
            else
            {
                wkLogLevel(Warn, "Unknown command %d", cmd);
            }
#endif
        }
    }
    
    if (lin->pts.size() > 0)
    {
        lin->initGeoMbr();
        shapes.insert(lin);
    }
}

bool VectorTilePBFParser::parsePolygon(const std::vector<uint32_t> &geometry, VectorAreal& shape)
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    Point2f firstCoord(0, 0);
    VectorRing ring;
    
    for (int k = 0; k < geometry.size(); )
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
                
                ring.push_back(Point2f(point.x(),point.y()));
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
                if (ring.size() > 0)  //We've already got a line, finish it
                {
                    ring.push_back(Point2f(firstCoord.x(),firstCoord.y())); //close the loop
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
    if (ring.size() > 0)
    {
        wkLogLevel(Warn, "Finished polygon loop, and ring has %d points", (int)ring.size());
    }
#endif
    //TODO: Is there a posibilty of still having a ring here that hasn't been added by a close command?
    
    if (!shape.loops.empty())
    {
        shape.initGeoMbr();
        return true;
    }
    return false;
}

bool VectorTilePBFParser::parsePoints(const std::vector<uint32_t> &geometry, VectorPoints& shape)
{
    double x = 0;
    double y = 0;
    int cmd = -1;
    int length = 0;
    Point2f point;
    
    for (int k = 0; k < geometry.size(); )
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
                    shape.pts.push_back(Point2f(point.x(),point.y()));
                }
            }
            else if (cmd == SEG_CLOSE_MASKED)
            {
#if DEBUG
                wkLogLevel(Warn, "Close point feature?");
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

void VectorTilePBFParser::addFeature(const VectorObjectRef &vecObj, const SimpleIDSet &styleIDs)
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

    if      (!string.empty())        vec.push_back({{.stringValue = string},             SmallValue::SmallValString});
    else if (value.has_float_value)  vec.push_back({{.floatValue  = value.float_value},  SmallValue::SmallValFloat});
    else if (value.has_double_value) vec.push_back({{.doubleValue = value.double_value}, SmallValue::SmallValDouble});
    else if (value.has_int_value)    vec.push_back({{.intValue    = value.int_value},    SmallValue::SmallValInt});
    else if (value.has_uint_value)   vec.push_back({{.uintValue   = value.uint_value},   SmallValue::SmallValUInt});
    else if (value.has_sint_value)   vec.push_back({{.sintValue   = value.sint_value},   SmallValue::SmallValSInt});
    else if (value.has_bool_value)   vec.push_back({{.boolValue   = value.bool_value},   SmallValue::SmallValBool});
    
    return true;
}

void VectorTilePBFParser::layerElement()
{
    // When we see a feature, that means we finished with (some of) the layer message
    if (!_layerStarted && !layerStart())
    {
        _skipLayer = true;
        _skippedLayerCount += 1;
    }
    _layerStarted = true;
}
    
// Called when we have first seen a sub-element of a layer, meaning that
// the version, name, and extent are populated, and the layer can be evaluated.
// Return false to skip this layer.
bool VectorTilePBFParser::layerStart()
{
    if (!_currentLayer)
    {
        wkLogLevel(Error, "Layer not set!");
        return false;
    }
    if (!_currentLayer || !_currentLayer->has_extent)
    {
        wkLogLevel(Warn, "Layer has no extent! (%s)", _layerName.c_str());
        return false;
    }
    
    _layerName = _layerNameView;
    _layerNameView = std::string_view();
    
    _layerScale = (double)_currentLayer->extent / TileSize;
   
    // if we dont have any styles for a layer, dont bother parsing the features
    if (!_styleDelegate->layerShouldDisplay(_styleInst, _layerName, _tileData->ident))
    {
        return false;
    }
    
    return true;
}

bool VectorTilePBFParser::layerFinish()
{
    _layerName.clear();
    _layerKeys.clear();
    _layerValues.clear();
    
    _currentLayer = nullptr;
    _skipLayer = false;
    
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
    vec.push_back(std::string_view((char*)stream->state, stream->bytes_left));
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

