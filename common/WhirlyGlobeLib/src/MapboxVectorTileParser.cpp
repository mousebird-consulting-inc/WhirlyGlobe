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
#import "vector_tile.pb.h"
#import <vector>

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

MapboxVectorTileParser::MapboxVectorTileParser(VectorStyleDelegateImplRef styleDelegate)
    : localCoords(false), keepVectors(false), parseAll(false), styleDelegate(styleDelegate)
{
    // Index all the categories ahead of time.  Once.
    std::vector<VectorStyleImplRef> allStyles = styleDelegate->allStyles();
    for (VectorStyleImplRef style: allStyles) {
        std::string category = style->getCategory();
        if (!category.empty()) {
            long long styleID = style->getUuid();
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
    
bool MapboxVectorTileParser::parse(PlatformThreadInfo *styleInst,RawData *rawData,VectorTileData *tileData)
{
    //calulate tile bounds and coordinate shift
    int tileSize = 256;
    double sx = tileSize / (tileData->bbox.ur().x() - tileData->bbox.ll().x());
    double sy = tileSize / (tileData->bbox.ur().y() - tileData->bbox.ll().y());
    //Tile origin is upper left corner, in epsg:3785
    double tileOriginX = tileData->bbox.ll().x();
    double tileOriginY = tileData->bbox.ur().y();
    
    double scale;
    double x;
    double y;
    int32_t dx;
    int32_t dy;
    int geometrySize;
    MapnikGeometryType g_type;
    int cmd;
    const int cmd_bits = 3;
    unsigned length;
    int k;
    unsigned cmd_length;
    Point2f point;
    Point2f firstCoord;
    
    unsigned featureCount = 0;
    
    int unknownAttributeCount = 0;
    int badAttributeCount = 0;
    int unknownCommandTypes = 0;
    int parseErrors = 0;
    
    //now attempt to open protobuf
    vector_tile::Tile tile;
    if(tile.ParseFromArray(rawData->getRawData(), (int)rawData->getLen())) {
        // Run through layers
        for (unsigned i=0;i<tile.layers_size();++i) {
            vector_tile::Tile_Layer const& tileLayer = tile.layers(i);
            scale = tileLayer.extent() / 256.0;

            std::string layerName = tileLayer.name();
            
            // if we dont have any styles for a layer, dont bother parsing the features
            if (!styleDelegate->layerShouldDisplay(layerName, tileData->ident))
                continue;
            
            // Work through features
            for (unsigned j=0;j<tileLayer.features_size();++j) {
                featureCount++;
                vector_tile::Tile_Feature const & f = tileLayer.features(j);
                g_type = static_cast<MapnikGeometryType>(f.type());
                
                //Parse attributes
                MutableDictionaryRef attributes = MutableDictionaryMake();
                attributes->setInt("geometry_type", (int)g_type);
                attributes->setString("layer_name", layerName);
                attributes->setInt("layer_order",i);
                
                for (int m = 0; m < f.tags_size(); m += 2) {
                    int32_t key_name = f.tags(m);
                    int32_t key_value = f.tags(m + 1);
                    if (key_name < static_cast<std::size_t>(tileLayer.keys_size())
                        && key_value < static_cast<std::size_t>(tileLayer.values_size())) {
                        const std::string &key = tileLayer.keys(key_name);
                        if(key.empty()) {
                            continue;
                        }
                        
                        vector_tile::Tile_Value const& value = tileLayer.values(key_value);
                        if (value.has_string_value()) {
                            attributes->setString(key, value.string_value());
                        } else if (value.has_int_value()) {
                            attributes->setInt(key, value.int_value());
                        } else if (value.has_double_value()) {
                            attributes->setDouble(key, value.double_value());
                        } else if (value.has_float_value()) {
                            attributes->setDouble(key, value.float_value());
                        } else if (value.has_bool_value()) {
                            attributes->setInt(key, (int)value.bool_value());
                        } else if (value.has_sint_value()) {
                            attributes->setInt(key, (int)value.sint_value());
                        } else if (value.has_uint_value()) {
                            attributes->setInt(key, (int)value.uint_value());
                        } else {
                            unknownAttributeCount++;
                        }
                    } else {
                        badAttributeCount++;
                    }
                }
                
                // Ask for the styles that correspond to this feature
                // If there are none, we can skip this
                SimpleIDSet styleIDs;
                // Do a quick inclusion check
                if (!uuidName.empty()) {
                    std::string uuidVal = attributes->getString(uuidName);
                    if (uuidValues.find(uuidVal) == uuidValues.end())
                        continue;
                }
                std::vector<VectorStyleImplRef> styles = styleDelegate->stylesForFeature(attributes, tileData->ident, tileLayer.name());
                for (auto style: styles) {
                    styleIDs.insert(style->getUuid());
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
                                    Point2f loc((tileOriginX + x / sx),(tileOriginY - y / sy));
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
                                    
                                    lin->pts.push_back(point);
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    //NSLog(@"Close line, layer:%@", layerName);
                                    if(lin->pts.size() > 0) { //We've already got a line, finish it
                                        lin->pts.push_back(firstCoord);
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
                                    Point2f loc((tileOriginX + x / sx),(tileOriginY - y / sy));
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
                                    
                                    ring.push_back(point);
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    if(ring.size() > 0) { //We've already got a line, finish it
                                        ring.push_back(firstCoord); //close the loop
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
                                        Point2f loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                        if (localCoords) {
                                            point = loc;
                                        } else {
                                            point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                            point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                        }
                                        shape->pts.push_back(point);
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
    VectorStyleImplRef style = styleDelegate->styleForUUID(styleID);
    if (style)
        style->buildObjects(styleInst,vecObjs, data);
}
    
}
