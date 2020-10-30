/*
 *  MapboxVectorTileParser.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/25/16.
 *  Copyright 2011-2016 mousebird consulting
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
 *
 */

#import "MapboxVectorTileParser.h"
#import "MaplyVectorStyleC.h"
#import "VectorObject.h"
#import "WhirlyKitLog.h"
#import "DictionaryC.h"

#import "vector_tile.pb.h"
#import "pb_decode.h"

#import <vector>
#import <string>

static double MAX_EXTENT = 20037508.342789244;

using namespace Eigen;

namespace WhirlyKit
{
    
VectorTileData::VectorTileData()
{
}
    
VectorTileData::VectorTileData(const VectorTileData &that)
    : ident(that.ident), bbox(that.bbox), geoBBox(that.geoBBox)
{
}
    
VectorTileData::~VectorTileData()
{
    for (auto it : vecObjsByStyle)
        delete it.second;
}
    
void VectorTileData::mergeFrom(VectorTileData *that)
{
    compObjs.insert(compObjs.end(),that->compObjs.begin(),that->compObjs.end());
    images.insert(images.end(),that->images.begin(),that->images.end());
    vecObjs.insert(vecObjs.end(),that->vecObjs.begin(),that->vecObjs.end());
    for (auto it : that->vecObjsByStyle) {
        auto it2 = vecObjsByStyle.find(it.first);
        if (it2 != vecObjsByStyle.end())
            it2->second->insert(it2->second->end(),it.second->begin(),it.second->end());
        else
            vecObjsByStyle[it.first] = it.second;
    }
    that->vecObjsByStyle.clear();
    for (auto it : that->categories) {
        categories[it.first] = it.second;
    }
    
    if (!that->changes.empty())
        changes.insert(changes.end(),that->changes.begin(),that->changes.end());
    
    that->clear();
}

void VectorTileData::clear()
{
    compObjs.clear();
    images.clear();
    vecObjs.clear();

    for (auto it : vecObjsByStyle)
        delete it.second;
    vecObjsByStyle.clear();
    categories.clear();
    
    changes.clear();
}

MapboxVectorTileParser::MapboxVectorTileParser(PlatformThreadInfo *inst,VectorStyleDelegateImplRef styleDelegate)
    : localCoords(false), keepVectors(false), parseAll(false), styleDelegate(styleDelegate)
{
    // Index all the categories ahead of time.  Once.
    std::vector<VectorStyleImplRef> allStyles = styleDelegate->allStyles(inst);
    for (VectorStyleImplRef style: allStyles) {
        std::string category = style->getCategory(inst);
        if (!category.empty()) {
            long long styleID = style->getUuid(inst);
            addCategory(category, styleID);
        }
    }
}

MapboxVectorTileParser::~MapboxVectorTileParser()
{
}
    
void MapboxVectorTileParser::setUUIDs(const std::string &name,const std::set<std::string> &uuids)
{
    uuidName = name;
    uuidValues = uuids;
}

void MapboxVectorTileParser::addCategory(const std::string &category,long long styleID)
{
    styleCategories[styleID] = category;
}

class VectorTilePBFParser
{
public:
    typedef VectorTilePBFParser This;
    
public:
    VectorTilePBFParser(VectorTileData *tileData,
                        VectorStyleDelegateImpl* styleData,
                        PlatformThreadInfo *styleInst)
        : _tileData    (tileData)
        , _styleData   (styleData)
        , _styleInst   (styleInst)
        , _bbox        (tileData->bbox)
        , _bboxWidth   (_bbox.ur().x() - _bbox.ll().x())
        , _bboxHeight  (_bbox.ur().y() - _bbox.ll().y())
        , _sx          ((_bboxWidth > 0) ? (TileSize / _bboxWidth) : 0)
        , _sy          ((_bboxHeight > 0) ? (TileSize / _bboxHeight) : 0)
        , _tileOriginX (tileData->bbox.ll().x())
        , _tileOriginY (tileData->bbox.ur().y())
    {
    }

    bool Parse(const uint8_t* data, size_t length)
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
    
    std::string GetErrorString(const char* def) const
    {
        return _parseError.length() ? _parseError : def;
    }
    const std::string &GetErrorString(const std::string &def) const
    {
        return _parseError.length() ? _parseError : def;
    }

private:
    // Tile contains a collection of Layers
    static bool layerDecode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        return (*(This**)arg)->layerDecode(stream, field);
    }
    bool layerDecode(pb_istream_t *stream, const pb_field_t *field)
    {
        _currentLayer = _defaultLayer;
        _layerStarted = false;
        if (!pb_decode(stream, vector_tile_Tile_Layer_fields, &_currentLayer))
        {
            return false;
        }

        return layerFinish(_currentLayer);
    }

    // Layer contains a collection of Features
    static bool featureDecode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        return (*(This**)arg)->featureDecode(stream, field);
    }
    bool featureDecode(pb_istream_t *stream, const pb_field_t *field)
    {
        layerElement();

        // todo: see if we have enough info to make better guesses here
        _featureTags.clear();
        _featureTags.reserve(20);
        _featureGeometry.clear();
        _featureGeometry.reserve(100);
        
        auto feature = _defaultFeature;
        if (!pb_decode(stream, vector_tile_Tile_Feature_fields, &feature))
        {
            return false;
        }

        _featureCount += 1;
        
        const auto geomType = static_cast<MapnikGeometryType>(feature.type);
        wkLog("[%llx] Read feature type %d with %d tags, %d geometry",
              this, (int)geomType, (int)_featureTags.size(), (int)_featureGeometry.size());

        if (_featureTags.size() % 2 != 0) {
            wkLogLevel(Warn, "Odd feature tags!");
        }

        auto attributes = std::make_shared<MutableDictionaryC>();
        attributes->setString("layer_name", _layerName);
        attributes->setInt("geometry_type", (int)geomType);
        attributes->setInt("layer_order", _layerIndex);

        for (int m = 0; m + 1 < _featureTags.size(); m += 2)
        {
            const auto keyIndex = _featureTags[m];
            const auto valueIndex = _featureTags[m + 1];
            
            if (keyIndex >= _layerKeys.size() || valueIndex >= _layerValues.size()) {
                ++_badAttributeCount;
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
                    _unknownAttributeCount += 1;
                    wkLogLevel(Warn, "Invalid Value Type %d", value.type);
                    break;
            }
        }

        _featureTags.clear();
        _featureGeometry.clear();

        return true;
    }

    // Layer contains a collection of Values
    static bool valueVecDecode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        if (!arg || !*arg) {
            return true;
        }
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

    void layerElement()
    {
        // When we see a feature, that means we finished with (some of) the layer message
        if (!_layerStarted && !layerStart(_currentLayer))
        {
            _skipLayer = true;
        }
        _layerStarted = true;
    }
        
    // Called when we have first seen a sub-element of a layer, meaning that
    // the version, name, and extent are populated, and the layer can be evaluated.
    // Return false to skip this layer.
    bool layerStart(const vector_tile_Tile_Layer &layer)
    {
        if (!layer.has_extent)
        {
            wkLog("Layer has no extent! (%s)", _layerName.c_str());
            return false;
        }
        
        _layerName = _layerNameView;
        _layerNameView = std::string_view();
        
        _layerScale = (double)layer.extent / TileSize;
       
        // if we dont have any styles for a layer, dont bother parsing the features
        if (!_styleData->layerShouldDisplay(_styleInst, _layerName, _tileData->ident))
        {
            wkLog("Skipping layer '%s' with no style", _layerName.c_str());
            return false;
        }

        wkLog("Starting layer '%s'", _layerName.c_str());
        
        return true;
    }

    bool layerFinish(const vector_tile_Tile_Layer& layer)
    {
        wkLog("Layer '%s' has %d keys, %d values", _layerName.c_str(), (int)_layerKeys.size(), (int)_layerValues.size());
        wkLog("Done with layer '%s'", _layerName.c_str());

        _layerName.clear();
        _layerKeys.clear();
        _layerValues.clear();
        return true;
    }
    
    static bool stringDecode(pb_istream_t *stream, const pb_field_t *field, void **arg)
    {
        if (arg && *arg) {
            *((std::string_view*)*arg) = std::string_view((char*)stream->state, stream->bytes_left);
        }
        return true;
    }
    static bool stringVecDecode(pb_istream_t *stream, const pb_field_t *field, void **arg)
    {
        if (arg && *arg) {
            auto &vec = **(std::vector<std::string_view>**)arg;
            vec.push_back(std::string_view((char*)stream->state, stream->bytes_left));
        }
        return true;
    }
    static bool intVecDecode(pb_istream_t *stream, const pb_field_t *field, void **arg)
    {
        if (!arg || !*arg) {
            return true;
        }
        auto &vec = **(std::vector<uint32_t>**)arg;
        vec.reserve(vec.size() + stream->bytes_left);
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

private:
    // Default state of state structures, for easy setup
    
    const vector_tile_Tile_Layer _defaultLayer = {
        /* name       */ { &stringDecode,    &_layerNameView },
        /* features   */ { &featureDecode,   this },
        /* keys       */ { &stringVecDecode, &_layerKeys },
        /* values     */ { &valueVecDecode,  &_layerValues },
        /* has_extent */ false,
        /* extent     */ 0,
        /* version    */ 0,
        /* extensions */ nullptr,
    };
    const vector_tile_Tile_Feature _defaultFeature = {
        /* has_id   */ false,
        /* id       */ 0LL,
        /* tags     */ { &intVecDecode, &_featureTags },
        /* has_type */ false,
        /* type     */ vector_tile_Tile_GeomType_UNKNOWN,
        /* geometry */ { &intVecDecode, &_featureGeometry },
    };
    const vector_tile_Tile_Value _defaultValue = {
        /* pb_callback_t string_value */ { stringDecode, nullptr },
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

    // The current elements being parsed
    vector_tile_Tile_Layer _currentLayer;
    bool _layerStarted = false;
    bool _skipLayer = false;

    struct SmallValue
    {
        enum SmallValueType : int8_t {
            SmallValNone,
            SmallValString,
            SmallValFloat,
            SmallValDouble,
            SmallValInt,
            SmallValUInt,
            SmallValSInt,
            SmallValBool,
        };
        union {
            std::string_view stringValue;
            float floatValue;
            double doubleValue;
            int64_t intValue;
            uint64_t uintValue;
            int64_t sintValue;
            bool boolValue;
        };
        SmallValueType type;
    };
    
    int _layerIndex = 0;
    double _layerScale = 0;
    std::string _layerName;
    std::string_view _layerNameView;
    std::vector<uint32_t> _featureTags;
    std::vector<uint32_t> _featureGeometry;
    std::vector<std::string_view> _layerKeys;
    std::vector<SmallValue> _layerValues;

private:
    const VectorTileData *_tileData;
    VectorStyleDelegateImpl *_styleData;
    PlatformThreadInfo *_styleInst;
    std::string _parseError;

    const MbrD _bbox;
    const double _bboxWidth;
    const double _bboxHeight;
    
    static const int TileSize = 256;
    const double _sx;
    const double _sy;
    const double _tileOriginX;
    const double _tileOriginY;
    
    double _scale;
    double _x;
    double _y;
    int32_t _dx;
    int32_t _dy;
    int _geometrySize;
    int _cmd;
    const int CmdBits = 3;
    unsigned _length;
    int _k;
    unsigned _cmdLength;
    Point2d _point;
    Point2d _firstCoord;
    unsigned _featureCount = 0;
    int _unknownAttributeCount = 0;
    int _badAttributeCount = 0;
    int _unknownCommandTypes = 0;
    int _parseErrors = 0;
};


bool MapboxVectorTileParser::parse(PlatformThreadInfo *styleInst,RawData *rawData,VectorTileData *tileData)
{
    VectorTilePBFParser parser(tileData, &*styleDelegate, styleInst);
    if (!parser.Parse(rawData->getRawData(), rawData->getLen()))
    {
        wkLogLevel(Warn, "Failed to parse vector tile PBF: '%s'",
                   std::string(parser.GetErrorString("unknown")).c_str());
        return false;
    }
    
    //now attempt to open protobuf
    /*
        for (unsigned i=0;i<tile.layers_size();++i) {
            for (unsigned j=0;j<tileLayer.features_size();++j) {
                
                // Ask for the styles that correspond to this feature
                // If there are none, we can skip this
                SimpleIDSet styleIDs;
                // Do a quick inclusion check
                if (!uuidName.empty()) {
                    std::string uuidVal = attributes->getString(uuidName);
                    if (uuidValues.find(uuidVal) == uuidValues.end())
                        continue;
                }
                std::vector<VectorStyleImplRef> styles = styleDelegate->stylesForFeature(styleInst, attributes, tileData->ident, tileLayer.name());
                for (auto style: styles) {
                    styleIDs.insert(style->getUuid(styleInst));
                }
                if (styleIDs.empty() && !parseAll)
                    continue;
                
                //Parse geometry
                x = 0;
                y = 0;
                geometrySize = f.geometry_size();
                cmd = -1;
                length = 0;
                
                VectorObjectRef vecObj = VectorObjectRef(new VectorObject());
                
                try {
                    if(g_type == GeomTypeLineString) {
                        VectorLinearRef lin;
                        for (k = 0; k < geometrySize;) {
                            if (!length) {
                                cmd_length = f.geometry(k++);
                                cmd = cmd_length & ((1 << cmd_bits) - 1);
                                length = cmd_length >> cmd_bits;
                            }//length is the number of coordinates before the CMD changes
                            
                            if (length > 0) {
                                length--;
                                if (cmd == SEG_MOVETO || cmd == SEG_LINETO) {
                                    dx = f.geometry(k++);
                                    dy = f.geometry(k++);
                                    dx = ((dx >> 1) ^ (-(dx & 1)));
                                    dy = ((dy >> 1) ^ (-(dy & 1)));
                                    x += (static_cast<double>(dx) / scale);
                                    y += (static_cast<double>(dy) / scale);
                                    //At this point x/y is a coord encoded in tile coord space, from 0 to TILE_SIZE
                                    //Convert to epsg:3785, then to degrees, then to radians
                                    Point2d loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                    if (localCoords) {
                                        point = loc;
                                    } else {
                                        point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                        point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    }

                                    if(cmd == SEG_MOVETO) { //move to means we are starting a new segment
                                        if(lin && lin->pts.size() > 0) { //We've already got a line, finish it
                                            lin->initGeoMbr();
                                            vecObj->shapes.insert(lin);
                                        }
                                        lin = VectorLinear::createLinear();
                                        lin->pts.reserve(length);
                                        firstCoord = point;
                                    }
                                    
                                    lin->pts.push_back(Point2f(point.x(),point.y()));
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    //NSLog(@"Close line, layer:%@", layerName);
                                    if(lin->pts.size() > 0) { //We've already got a line, finish it
                                        lin->pts.push_back(Point2f(firstCoord.x(),firstCoord.y()));
                                        lin->initGeoMbr();
                                        vecObj->shapes.insert(lin);
                                        lin.reset();
                                    } else {
//                                        NSLog(@"Error: Close line with no points");
                                    }
                                } else {
//                                    NSLog(@"Unknown command type:%i", cmd);
                                }
                            }
                        }
                        
                        if(lin->pts.size() > 0) {
                            lin->initGeoMbr();
                            vecObj->shapes.insert(lin);
                        }
                    } else if(g_type == GeomTypePolygon) {
                        VectorArealRef shape = VectorAreal::createAreal();
                        VectorRing ring;
                        
                        for (k = 0; k < geometrySize;) {
                            if (!length) {
                                cmd_length = f.geometry(k++);
                                cmd = cmd_length & ((1 << cmd_bits) - 1);
                                length = cmd_length >> cmd_bits;
                            }
                            
                            if (length > 0) {
                                length--;
                                if (cmd == SEG_MOVETO || cmd == SEG_LINETO) {
                                    dx = f.geometry(k++);
                                    dy = f.geometry(k++);
                                    dx = ((dx >> 1) ^ (-(dx & 1)));
                                    dy = ((dy >> 1) ^ (-(dy & 1)));
                                    x += (static_cast<double>(dx) / scale);
                                    y += (static_cast<double>(dy) / scale);
                                    //At this point x/y is a coord is encoded in tile coord space, from 0 to TILE_SIZE
                                    //Convert to epsg:3785, then to degrees, then to radians
                                    Point2d loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                    if (localCoords) {
                                        point = loc;
                                    } else {
                                        point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                        point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    }

                                    if(cmd == SEG_MOVETO) { //move to means we are starting a new segment
                                        firstCoord = point;
                                        //TODO: does this ever happen when we are part way through a shape? holes?
                                    }
                                    
                                    ring.push_back(Point2f(point.x(),point.y()));
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    if(ring.size() > 0) { //We've already got a line, finish it
                                        ring.push_back(Point2f(firstCoord.x(),firstCoord.y())); //close the loop
                                        shape->loops.push_back(ring); //add loop to shape
                                        ring.clear(); //reuse the ring
                                    }
                                } else {
                                    unknownCommandTypes++;
                                }
                            }
                        }
                        
                        if(ring.size() > 0) {
//                            NSLog(@"Finished polygon loop, and ring has points");
                        }
                        //TODO: Is there a posibilty of still having a ring here that hasn't been added by a close command?
                        
                        shape->initGeoMbr();
                        vecObj->shapes.insert(shape);
                    } else if(g_type == GeomTypePoint) {
                        VectorPointsRef shape = VectorPoints::createPoints();
                        
                        for (k = 0; k < geometrySize;) {
                            if (!length) {
                                cmd_length = f.geometry(k++);
                                cmd = cmd_length & ((1 << cmd_bits) - 1);
                                length = cmd_length >> cmd_bits;
                            }
                            
                            if (length > 0) {
                                length--;
                                if (cmd == SEG_MOVETO || cmd == SEG_LINETO) {
                                    dx = f.geometry(k++);
                                    dy = f.geometry(k++);
                                    dx = ((dx >> 1) ^ (-(dx & 1)));
                                    dy = ((dy >> 1) ^ (-(dy & 1)));
                                    x += (static_cast<double>(dx) / scale);
                                    y += (static_cast<double>(dy) / scale);
                                    //At this point x/y is a coord is encoded in tile coord space, from 0 to TILE_SIZE
                                    //Covert to epsg:3785, then to degrees, then to radians
                                    if(x > 0 && x < 256 && y > 0 && y < 256) {
                                        Point2d loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                        if (localCoords) {
                                            point = loc;
                                        } else {
                                            point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                            point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                        }
                                        shape->pts.push_back(Point2f(point.x(),point.y()));
                                    }
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
//                                    NSLog(@"Close point feature?");
                                } else {
                                    unknownCommandTypes++;
                                }
                            }
                        }
                        
                        if(shape->pts.size() > 0) {
                            shape->initGeoMbr();
                            vecObj->shapes.insert(shape);
                        }
                    } else if(g_type == GeomTypeUnknown) {
//                        NSLog(@"Unknown geom type");
                    }
                } catch(...) {
                    parseErrors++;
                }
                
                if(vecObj->shapes.size() > 0) {
                    if (keepVectors)
                        tileData->vecObjs.push_back(vecObj);

                    // Sort this vector object into the styles that will process it
                    for (SimpleIdentity styleID : styleIDs) {
                        std::vector<VectorObjectRef> *vecs = NULL;
                        auto it = tileData->vecObjsByStyle.find(styleID);
                        if (it != tileData->vecObjsByStyle.end())
                            vecs = it->second;
                        if (!vecs) {
                            vecs = new std::vector<VectorObjectRef>();
                            tileData->vecObjsByStyle[styleID] = vecs;
                        }
                        vecs->push_back(vecObj);
                    }
                }
                
                
                for (auto shape: vecObj->shapes)
                    shape->setAttrDict(attributes);
            }
        }
    } else {
        return false;
    }
     */
    
    // Run the styles over their assembled data
    for (auto it : tileData->vecObjsByStyle) {
        std::vector<VectorObjectRef> *vecs = it.second;

        auto styleData = VectorTileDataRef(new VectorTileData(*tileData));

        // Ask the subclass to run the style and fill in the VectorTileData
        buildForStyle(styleInst,it.first,*vecs,styleData);
        
        // Sort the results into categories if needed
        auto catIt = styleCategories.find(it.first);
        if (catIt != styleCategories.end() && !styleData->compObjs.empty()) {
            std::string category = catIt->second;
            auto compObjs = styleData->compObjs;
            auto categoryIt = tileData->categories.find(category);
            if (categoryIt != tileData->categories.end()) {
                compObjs.insert(compObjs.end(), categoryIt->second.begin(), categoryIt->second.end());
            }
            tileData->categories[category] = compObjs;
        }
        
        // Merge this into the general return data
        tileData->mergeFrom(styleData.get());
    }
    
    // These are layered on top for debugging
//    if(debugLabel || debugOutline) {
//        QuadTreeNew::Node tileID = tileData->ident;
//        MbrD geoBounds = tileData->geoBBox;
//        Point2d sw = geoBounds.ll(), ne = geoBounds.ur();
//        if(debugLabel) {
//            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
//            label.text = [NSString stringWithFormat:@"%d: (%d,%d)\n%lu items", tileID.level, tileID.x,
//                          tileID.y, (unsigned long)tileData->compObjs.size()];
//            MaplyCoordinate tileCenter;
//            tileCenter.x = (ne.x() + sw.x())/2.0;
//            tileCenter.y = (ne.y() + sw.y())/2.0;
//            label.loc = tileCenter;
//
//            MaplyComponentObject *c = [viewC addScreenLabels:@[label]
//                                                         desc:@{kMaplyFont : [UIFont boldSystemFontOfSize:12],
//                                                                kMaplyTextColor : [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.25],
//                                                                kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
//                                                                kMaplyEnable: @(NO)
//                                                                }
//                                                         mode:MaplyThreadCurrent];
//            tileData->compObjs.push_back(c->contents);
//        }
//        if(debugOutline) {
//            MaplyCoordinate outline[5];
//            outline[0].x = ne.x();            outline[0].y = ne.y();
//            outline[1].x = ne.x();            outline[1].y = sw.y();
//            outline[2].x = sw.x();            outline[2].y = sw.y();
//            outline[3].x = sw.x();            outline[3].y = ne.y();
//            outline[4].x = ne.x();            outline[4].y = ne.y();
//            MaplyVectorObject *outlineObj = [[MaplyVectorObject alloc] initWithLineString:outline
//                                                                                numCoords:5
//                                                                               attributes:nil];
//            MaplyComponentObject *c = [viewC addVectors:@[outlineObj]
//                                                    desc:@{kMaplyColor: [UIColor redColor],
//                                                           kMaplyVecWidth:@(4),
//                                                           kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
//                                                           kMaplyEnable: @(NO)
//                                                           }
//                                                    mode:MaplyThreadCurrent];
//            tileData->compObjs.push_back(c->contents);
//        }
//    }
    
    return true;
}

void MapboxVectorTileParser::buildForStyle(PlatformThreadInfo *styleInst,
                                           long long styleID,
                                           std::vector<VectorObjectRef> &vecObjs,
                                           VectorTileDataRef data)
{
    VectorStyleImplRef style = styleDelegate->styleForUUID(styleInst,styleID);
    if (style)
        style->buildObjects(styleInst,vecObjs, data);
}
    
}
