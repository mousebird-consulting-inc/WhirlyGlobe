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
#import "VectorObject.h"
#import "vector_tile.pb.h"

static double MAX_EXTENT = 20037508.342789244;

using namespace Eigen;

namespace WhirlyKit
{

MapboxVectorTileParser::MapboxVectorTileParser()
{
}

MapboxVectorTileParser::~MapboxVectorTileParser()
{
}
    
    
bool MapboxVectorTileParser::parseVectorTile(RawData *rawData,std::vector<VectorObject *> &vecObjs,const Mbr &mbr)
{
    //calulate tile bounds and coordinate shift
    int tileSize = 256;
    double sx = tileSize / (mbr.ur().x() - mbr.ll().x());
    double sy = tileSize / (mbr.ur().y() - mbr.ll().y());
    //Tile origin is upper left corner, in epsg:3785
    double tileOriginX = mbr.ll().x();
    double tileOriginY = mbr.ur().y();
    
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
    
    //now attempt to open protobuf
    vector_tile::Tile tile;
    if(tile.ParseFromArray(rawData->getRawData(), (int)rawData->getLen())) {
        //Itterate layers
        for (unsigned i=0;i<tile.layers_size();++i) {
            vector_tile::Tile_Layer const& tileLayer = tile.layers(i);
            scale = tileLayer.extent() / 256.0;
            
            // iterate over features
            for (unsigned j=0;j<tileLayer.features_size();++j) {
                featureCount++;
                vector_tile::Tile_Feature const & f = tileLayer.features(j);
                g_type = static_cast<MapnikGeometryType>(f.type());
                
                //Parse attributes
                Dictionary attributes;
                attributes.setInt("geometry_type", (int)g_type);
                attributes.setString("layer_name", tileLayer.name());
                attributes.setInt("layer_order",i);
                
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
                            attributes.setString(key, value.string_value());
                        } else if (value.has_int_value()) {
                            attributes.setInt(key, value.int_value());
                        } else if (value.has_double_value()) {
                            attributes.setDouble(key, value.double_value());
                        } else if (value.has_float_value()) {
                            attributes.setDouble(key, value.float_value());
                        } else if (value.has_bool_value()) {
                            attributes.setInt(key, (int)value.bool_value());
                        } else if (value.has_sint_value()) {
                            attributes.setInt(key, (int)value.sint_value());
                        } else if (value.has_uint_value()) {
                            attributes.setInt(key, (int)value.uint_value());
                        }
                    }
                }
                
                //Parse geometry
                x = 0;
                y = 0;
                geometrySize = f.geometry_size();
                cmd = -1;
                length = 0;
                
                VectorObject *vecObj = new VectorObject();
                vecObjs.push_back(vecObj);
                
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
                                    //Covert to epsg:3785, then to degrees, then to radians
                                    point.x() = DegToRad(((tileOriginX + x / sx) / MAX_EXTENT) * 180.0);
                                    point.y() = 2 * atan(exp(DegToRad(((tileOriginY - y / sy) / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    
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
                                    //Covert to epsg:3785, then to degrees, then to radians
                                    point.x() = DegToRad(((tileOriginX + x / sx) / MAX_EXTENT) * 180.0);
                                    point.y() = 2 * atan(exp(DegToRad(((tileOriginY - y / sy) / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    
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
//                                    NSLog(@"Unknown command type:%i", cmd);
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
                                    point.x() = DegToRad(((tileOriginX + x / sx) / MAX_EXTENT) * 180.0);
                                    point.y() = 2 * atan(exp(DegToRad(((tileOriginY - y / sy) / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    
                                    shape->pts.push_back(point);
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
//                                    NSLog(@"Close point feature?");
                                } else {
//                                    NSLog(@"Unknown command type:%i", cmd);
                                }
                            }
                        }
                        
                        shape->initGeoMbr();
                        vecObj->shapes.insert(shape);
                    } else if(g_type == GeomTypeUnknown) {
//                        NSLog(@"Unknown geom type");
                    }
                } catch(...) {
//                    NSLog(@"Error parsing feature");
                }
                
                for (auto shape: vecObj->shapes)
                    shape->setAttrDict(attributes);
            } //end of iterating features
        }//end of iterating layers
    } else {
        return false;
    }
    
    return true;
}
    
}
