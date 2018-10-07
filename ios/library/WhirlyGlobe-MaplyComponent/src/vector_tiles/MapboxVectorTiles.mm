/*
 *  MaplyMapnikVectorTiles.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2017 mousebird consulting
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


#import "MapboxVectorTiles.h"
#import "MaplyTileSource.h"
#import "MapboxVectorStyleSet.h"
#import "MapboxVectorStyleBackground.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>

#import "CoordSystem.h"
#import "MaplyRemoteTileSource.h"
#import "MaplyVectorStyle.h"
#import "MaplyVectorObject_private.h"
#import "MaplyScreenLabel.h"
#import "NSData+Zlib.h"
#import "vector_tile.pb.h"
#import "VectorData.h"
#import "MaplyMBTileSource.h"
#import "MapnikStyleSet.h"
#import "MaplyRenderController_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

static double MAX_EXTENT = 20037508.342789244;

@implementation MaplyVectorTileData
@end

@implementation MapboxVectorTileParser

- (instancetype)initWithStyle:(NSObject<MaplyVectorStyleDelegate> *)styleDelegate viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super init];
    if (!self)
        return nil;
    
    _styleDelegate = styleDelegate;
    _viewC = viewC;
    
    return self;
}

- (void)dealloc
{
    _styleDelegate = nil;
    _viewC = nil;
}

- (MaplyVectorTileData *)buildObjects:(NSData *)tileData tile:(MaplyTileID)tileID bounds:(MaplyBoundingBox)bbox geoBounds:(MaplyBoundingBox)geoBbox
{
    //calulate tile bounds and coordinate shift
    int tileSize = 256;
    double sx = tileSize / (bbox.ur.x - bbox.ll.x);
    double sy = tileSize / (bbox.ur.y - bbox.ll.y);
    //Tile origin is upper left corner, in epsg:3785
    double tileOriginX = bbox.ll.x;
    double tileOriginY = bbox.ur.y;
    
    MaplyVectorTileInfo *tileInfo = [[MaplyVectorTileInfo alloc] init];
    tileInfo.tileID = tileID;
    tileInfo.geoBBox = {MaplyCoordinateDMake(geoBbox.ll.x, geoBbox.ll.y),MaplyCoordinateDMake(geoBbox.ur.x, geoBbox.ur.y)};

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
    
    NSMutableArray *components = [NSMutableArray array];
    NSMutableArray *vecObjs = nil;
    if (_keepVectors)
        vecObjs = [NSMutableArray array];
    //    CFAbsoluteTime start = CFAbsoluteTimeGetCurrent();
    
    unsigned featureCount = 0;
    
    NSMutableDictionary *featureStyles = [NSMutableDictionary new];
    NSMutableDictionary *categories = [NSMutableDictionary dictionary];
    
    //now attempt to open protobuf
    vector_tile::Tile tile;
    if(tile.ParseFromArray(tileData.bytes, (int)tileData.length)) {
        tileData = nil;
        //Itterate layers
        for (unsigned i=0;i<tile.layers_size();++i) {
            vector_tile::Tile_Layer const& tileLayer = tile.layers(i);
            scale = tileLayer.extent() / 256.0;
            NSString *layerName = [NSString stringWithUTF8String:tileLayer.name().c_str()];
            if(![_styleDelegate layerShouldDisplay:layerName tile:tileID] && !_parseAll) {
                // if we dont have any styles for a layer, dont bother parsing the features
                continue;
            }
            
            //itterate features
            for (unsigned j=0;j<tileLayer.features_size();++j) {
                featureCount++;
                vector_tile::Tile_Feature const & f = tileLayer.features(j);
                g_type = static_cast<MapnikGeometryType>(f.type());
                
                //Parse attributes
                NSMutableDictionary *attributes = [NSMutableDictionary new];
                attributes[@"geometry_type"] = @(g_type); //this seems wastefull, but is needed for the rule matcher
                attributes[@"layer_name"] = layerName;
                attributes[@"layer_order"] = @(i);
                
                for (int m = 0; m < f.tags_size(); m += 2) {
                    UInt32 key_name = f.tags(m);
                    UInt32 key_value = f.tags(m + 1);
                    if (key_name < static_cast<std::size_t>(tileLayer.keys_size())
                        && key_value < static_cast<std::size_t>(tileLayer.values_size())) {
                        NSString *key = [NSString stringWithUTF8String:tileLayer.keys(key_name).c_str()];
                        if(!key.length) {
                            continue;
                        }
                        
                        vector_tile::Tile_Value const& value = tileLayer.values(key_value);
                        if (value.has_string_value()) {
                            attributes[key] = [NSString stringWithUTF8String:value.string_value().c_str()];
                        } else if (value.has_int_value()) {
                            attributes[key] = @(value.int_value());
                        } else if (value.has_double_value()) {
                            attributes[key] = @(value.double_value());
                        } else if (value.has_float_value()) {
                            attributes[key] = @(value.float_value());
                        } else if (value.has_bool_value()) {
                            attributes[key] = @(value.bool_value());
                        } else if (value.has_sint_value()) {
                            attributes[key] = @(value.sint_value());
                        } else if (value.has_uint_value()) {
                            attributes[key] = @(value.uint_value());
                        } else {
                            NSLog(@"Unknown attribute type");
                        }
                    } else {
                        NSLog(@"Got a bad one");
                    }
                }
                
                NSArray *styles = [self.styleDelegate stylesForFeatureWithAttributes:attributes
                                                                              onTile:tileID
                                                                             inLayer:layerName
                                                                               viewC:_viewC];
                
                if(!styles.count && !_parseAll) {
//                    NSLog(@"kind = %@",attributes[@"kind"]);
                    continue; //no point parsing the geometry if we arent going to render
                }
                
                //Parse geometry
                x = 0;
                y = 0;
                geometrySize = f.geometry_size();
                cmd = -1;
                length = 0;
                
                MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] init];
                
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
                                    Point2f loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                    if (_localCoords) {
                                        point = loc;
                                    } else {
                                        point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                        point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                    }
                                    
                                    if(cmd == SEG_MOVETO) { //move to means we are starting a new segment
                                        if(lin && lin->pts.size() > 0) { //We've already got a line, finish it
                                            lin->initGeoMbr();
                                            [vecObj addShape:lin];
                                        }
                                        lin = VectorLinear::createLinear();
                                        firstCoord = point;
                                    }
                                    
                                    lin->pts.push_back(point);
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    //NSLog(@"Close line, layer:%@", layerName);
                                    if(lin->pts.size() > 0) { //We've already got a line, finish it
                                        lin->pts.push_back(firstCoord);
                                        lin->initGeoMbr();
                                        [vecObj addShape:lin];
                                        lin.reset();
                                    } else {
                                        NSLog(@"Error: Close line with no points");
                                    }
                                } else {
                                    NSLog(@"Unknown command type:%i", cmd);
                                }
                            }
                        }
                        
                        if(lin->pts.size() > 0) {
                            lin->initGeoMbr();
                            [vecObj addShape:lin];
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
                                    Point2f loc((tileOriginX + x / sx),(tileOriginY - y / sy));
                                    if (_localCoords) {
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
                                    NSLog(@"Unknown command type:%i", cmd);
                                }
                            }
                        }
                        
                        if(ring.size() > 0) {
                            NSLog(@"Finished polygon loop, and ring has points");
                        }
                        //TODO: Is there a posibilty of still having a ring here that hasn't been added by a close command?
                        
                        shape->initGeoMbr();
                        [vecObj addShape:shape];
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
                                        if (_localCoords) {
                                            point = loc;
                                        } else {
                                            point.x() = DegToRad((loc.x() / MAX_EXTENT) * 180.0);
                                            point.y() = 2 * atan(exp(DegToRad((loc.y() / MAX_EXTENT) * 180.0))) - M_PI_2;
                                        }
                                        shape->pts.push_back(point);
                                    }
                                } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                                    NSLog(@"Close point feature?");
                                } else {
                                    NSLog(@"Unknown command type:%i", cmd);
                                }
                            }
                        }
                        if(shape->pts.size() > 0) {
                            shape->initGeoMbr();
                            [vecObj addShape:shape];
                        }
                    } else if(g_type == GeomTypeUnknown) {
                        NSLog(@"Unknown geom type");
                    }
                } catch(...) {
                    NSLog(@"Error parsing feature");
                }
              
                if(vecObj.shapes.size() > 0) {
                    if (vecObjs)
                        [vecObjs addObject:vecObj];
                    for(NSObject<MaplyVectorStyle> *style in styles) {
                        NSMutableArray *featuresForStyle = featureStyles[style.uuid];
                        if(!featuresForStyle) {
                            featuresForStyle = [NSMutableArray new];
                            featureStyles[style.uuid] = featuresForStyle;
                        }
                        [featuresForStyle addObject:vecObj];
                    }
                }
                vecObj.attributes = attributes;
            } //end of iterating features
        }//end of itterating layers
    } else {
        return nil;
    }
    
    NSArray *symbolizerKeys = [featureStyles.allKeys sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self" ascending:YES]]];
    for(id key in symbolizerKeys) {
        NSObject<MaplyVectorStyle> *symbolizer = [self.styleDelegate styleForUUID:key viewC:_viewC];
        NSArray *features = featureStyles[key];
        NSArray *theseCompObjs = [symbolizer buildObjects:features forTile:tileInfo viewC:_viewC];

        // Categories used for sorting component objects (for enable/disable)
        NSString *category = [symbolizer getCategory];
        if (category) {
            NSMutableArray *catArray = [categories objectForKey:category];
            if (!catArray)
                catArray = [NSMutableArray array];
            [catArray addObjectsFromArray:theseCompObjs];
            categories[category] = catArray;
        }
        [components addObjectsFromArray:theseCompObjs];
    }
    
    if(self.debugLabel || self.debugOutline) {
        MaplyCoordinate ne = geoBbox.ur;
        MaplyCoordinate sw = geoBbox.ll;
        if(self.debugLabel) {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            label.text = [NSString stringWithFormat:@"%d: (%d,%d)\n%lu items", tileID.level, tileID.x,
                          tileID.y, (unsigned long)components.count];
            MaplyCoordinate tileCenter;
            tileCenter.x = (ne.x + sw.x)/2.0;
            tileCenter.y = (ne.y + sw.y)/2.0;
            label.loc = tileCenter;
            
            MaplyComponentObject *c = [_viewC addScreenLabels:@[label]
                                                         desc:@{kMaplyFont : [UIFont boldSystemFontOfSize:12],
                                                                kMaplyTextColor : [UIColor colorWithRed:0.25 green:0.25 blue:0.25 alpha:0.25],
                                                                kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
                                                                kMaplyEnable: @(NO)
                                                                }
                                                         mode:MaplyThreadCurrent];
            [components addObject:c];
        }
        if(self.debugOutline) {
            MaplyCoordinate outline[5];
            outline[0].x = ne.x;            outline[0].y = ne.y;
            outline[1].x = ne.x;            outline[1].y = sw.y;
            outline[2].x = sw.x;            outline[2].y = sw.y;
            outline[3].x = sw.x;            outline[3].y = ne.y;
            outline[4].x = ne.x;            outline[4].y = ne.y;
            MaplyVectorObject *outlineObj = [[MaplyVectorObject alloc] initWithLineString:outline
                                                                                numCoords:5
                                                                               attributes:nil];
            MaplyComponentObject *c = [_viewC addVectors:@[outlineObj]
                                                    desc:@{kMaplyColor: [UIColor redColor],
                                                           kMaplyVecWidth:@(4),
                                                           kMaplyDrawPriority : @(kMaplyMaxDrawPriorityDefault+100000000),
                                                           kMaplyEnable: @(NO)
                                                           }
                                                    mode:MaplyThreadCurrent];
            [components addObject:c];
        }
    }
    
    MaplyVectorTileData *tileRet = [[MaplyVectorTileData alloc] init];
    tileRet.compObjs = components;
    tileRet.vecObjs = vecObjs;
    if ([categories count] > 0)
        tileRet.categories = categories;
    
    return tileRet;
}

@end
