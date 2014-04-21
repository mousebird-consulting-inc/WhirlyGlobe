/*
 *  MaplyMapnikVectorTiles.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2014 mousebird consulting
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


#import "MaplyMapnikVectorTiles.h"

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

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@interface MaplyMapnikVectorTiles ()
@property (nonatomic, strong, readwrite) NSArray *tileSources;

@end

@implementation MaplyMapnikVectorTiles

static double MAX_EXTENT = 20037508.342789244;

- (instancetype) initWithTileSources:(NSArray*)tileSources {
  self = [super init];
  if(self) {
    self.tileSources = tileSources;
  }
  return self;
}

- (instancetype) initWithTileSource:(MaplyRemoteTileInfo*)tileSource {
  self = [self initWithTileSources:@[tileSource]];
  return self;
}


#pragma mark - MaplyPagingDelegate
- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer {
  //NSLog(@"MVTTiles startFetchForTile: %d/%d/%d",tileID.level,tileID.x,tileID.y);
  dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    //calulate tile bounds and coordinate shift
    int tileSize = 256;
    MaplyTileID flippedTile = tileWithFlippedY(tileID);
    MaplyCoordinate ll;
    MaplyCoordinate ur;
    [layer geoBoundsforTile:tileID ll:&ll ur:&ur];
    ll = [self toMerc:ll];
    ur = [self toMerc:ur];
    double sx = tileSize / (ur.x - ll.x);
    double sy = tileSize / (ur.y - ll.y);
    //Tile origin is upper left corner, in epsg:3785
    double tileOriginX = ll.x;
    double tileOriginY = ur.y;

    //declare all variables needed in parsing loop here
    NSData *tileData;
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
    CFAbsoluteTime start = CFAbsoluteTimeGetCurrent();

    unsigned featureCount = 0;
    
    NSMutableDictionary *featureStyles = [NSMutableDictionary new];
    
    for(MaplyRemoteTileInfo *tileSource in self.tileSources) {
      if([tileSource tileIsLocal:flippedTile]) {
        tileData = [NSData dataWithContentsOfFile:[tileSource fileNameForTile:flippedTile]];
      } else {
        NSURLRequest *request = [tileSource requestForTile:flippedTile];
        if(request) {
          NSURLResponse *response;
          NSError *error;
          tileData = [NSURLConnection sendSynchronousRequest:request
                                           returningResponse:&response error:&error];
          if(tileData) {
            [tileData writeToFile:[tileSource fileNameForTile:flippedTile] atomically:NO];
          }
        }
      }
      
      if(tileData.length) {
        if([tileData isCompressed]) {
          tileData = [tileData uncompressGZip];
          if(!tileData.length) {
            NSLog(@"Error: tile data was nil after decompression");
            continue;
          }
        }
        
        //now attempt to open protobuf
        mapnik::vector::tile tile;
        if(tile.ParseFromArray(tileData.bytes, (int)tileData.length)) {
          tileData = nil;
          //Itterate layers
          for (unsigned i=0;i<tile.layers_size();++i) {
            mapnik::vector::tile_layer const& tileLayer = tile.layers(i);
            scale = tileLayer.extent() / 256.0;
            NSString *layerName = [NSString stringWithUTF8String:tileLayer.name().c_str()];
            if(![self.styleDelegate layerShouldDisplay:layerName]) {
              // if we dont have any styles for a layer, dont bother parsing the features
              continue;
            }
            
            //itterate features
            for (unsigned j=0;j<tileLayer.features_size();++j) {
              featureCount++;
              mapnik::vector::tile_feature const & f = tileLayer.features(j);
              g_type = static_cast<MapnikGeometryType>(f.type());
              
              //Parse attributes
              NSMutableDictionary *attributes = [NSMutableDictionary new];
              attributes[@"geometry_type"] = @(g_type); //this seems wastefull, but is needed for the rule matcher
              
              for (int m = 0; m < f.tags_size(); m += 2) {
                UInt32 key_name = f.tags(m);
                UInt32 key_value = f.tags(m + 1);
                if (key_name < static_cast<std::size_t>(tileLayer.keys_size())
                    && key_value < static_cast<std::size_t>(tileLayer.values_size())) {
                  NSString *key = [NSString stringWithUTF8String:tileLayer.keys(key_name).c_str()];
                  if(!key.length) {
                    continue;
                  }
                  
                  mapnik::vector::tile_value const& value = tileLayer.values(key_value);
                  if (value.has_string_value()) {
                    attributes[key] = [NSString stringWithUTF8String:value.string_value().c_str()];
                  } else if (value.has_int_value()) {
                    attributes[key] = @(value.int_value());
                  } else if (value.has_double_value()) {
                    attributes[key] = @(value.double_value());
                  } else if (value.has_float_value()) {
                    attributes[key] = @(value.double_value());
                  } else if (value.has_bool_value()) {
                    attributes[key] = @(value.bool_value());
                  } else if (value.has_sint_value()) {
                    attributes[key] = @(value.sint_value());
                  } else if (value.has_uint_value()) {
                    attributes[key] = @(value.uint_value());
                  }
                }
              }
              
              NSArray *styles = [self.styleDelegate stylesForFeatureWithAttributes:attributes
                                                                            onTile:tileID
                                                                           inLayer:layerName];
              if(!styles.count) {
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
                        point.x() = DegToRad(((tileOriginX + x / sx) / MAX_EXTENT) * 180.0);
                        point.y() = 2 * atan(exp(DegToRad(((tileOriginY - y / sy) / MAX_EXTENT) * 180.0))) - M_PI_2;
                        
                        if(cmd == SEG_MOVETO) { //move to means we are starting a new segment
                          if(lin && lin->pts.size() > 0) { //We've already got a line, finish it
                            lin->initGeoMbr();
                            [vecObj addShape:lin];
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
                        point.x() = DegToRad(((tileOriginX + x / sx) / MAX_EXTENT) * 180.0);
                        point.y() = 2 * atan(exp(DegToRad(((tileOriginY - y / sy) / MAX_EXTENT) * 180.0))) - M_PI_2;
                        
                        shape->pts.push_back(point);
                      } else if (cmd == (SEG_CLOSE & ((1 << cmd_bits) - 1))) {
                        NSLog(@"Close point feature?");
                      } else {
                        NSLog(@"Unknown command type:%i", cmd);
                      }
                    }
                  }
                  
                  shape->initGeoMbr();
                  [vecObj addShape:shape];
                } else if(g_type == GeomTypeUnknown) {
                  NSLog(@"Unknown geom type");
                }
              } catch(...) {
                NSLog(@"Error parsing feature");
              }
              
              for(MaplyVectorTileStyle *style in styles) {
                NSMutableArray *featuresForStyle = featureStyles[style.uuid];
                if(!featuresForStyle) {
                  featuresForStyle = [NSMutableArray new];
                  featureStyles[style.uuid] = featuresForStyle;
                }
                [featuresForStyle addObject:vecObj];
              }
              vecObj.attributes = attributes;
            } //end of iterating features
          }//end of itterating layers
        } else {
          NSLog(@"Failed to parse pbf %d/%d/%d", flippedTile.level, flippedTile.x, flippedTile.y);
        }
        tileData = nil;
      } else {
        NSLog(@"No data for tile %d/%d/%d", flippedTile.level, flippedTile.x, flippedTile.y);
      }
    }//end of iterating tile sources
    
    NSArray *symbolizerKeys = [featureStyles.allKeys sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self"
                                                                                                                 ascending:YES]]];
    for(id key in symbolizerKeys) {
      MaplyVectorTileStyle *symbolizer = [self.styleDelegate styleForUUID:key];
      NSArray *features = featureStyles[key];
      [components addObjectsFromArray:[symbolizer buildObjects:features viewC:layer.viewC]];
    }
    
    if(self.debugLabel || self.debugOutline) {
      MaplyCoordinate ne;
      MaplyCoordinate sw;
      [layer geoBoundsforTile:tileID ll:&sw ur:&ne];
      if(self.debugLabel) {
        MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
        label.text = [NSString stringWithFormat:@"%d/%d/%d %lu items", flippedTile.level, flippedTile.x,
                      flippedTile.y, (unsigned long)components.count];
        MaplyCoordinate tileCenter;
        tileCenter.x = (ne.x + sw.x)/2.0;
        tileCenter.y = (ne.y + sw.y)/2.0;
        label.loc = tileCenter;
        
        MaplyComponentObject *c = [layer.viewC addScreenLabels:@[label]
                                                          desc:@{kMaplyFont : [UIFont boldSystemFontOfSize:12],
                                                                 kMaplyTextColor : [UIColor blackColor]}];
        [components addObject:c];
      }
      if(self.debugOutline) {
        MaplyCoordinate outline[5];
        outline[0] = ne;
        outline[1].x = ne.x;
        outline[1].y = sw.y;
        outline[2] = sw;
        outline[3].x = sw.x;
        outline[3].y = ne.y;
        outline[4] = ne;
        MaplyVectorObject *outlineObj = [[MaplyVectorObject alloc] initWithLineString:outline
                                                                            numCoords:5
                                                                           attributes:nil];
        MaplyComponentObject *c = [layer.viewC addVectors:@[outlineObj]
                                                     desc:@{kMaplyColor: [UIColor redColor],
                                                            kMaplyVecWidth:@(1)
                                                            }];
        [components addObject:c];
      }
    }
    
    [layer addData:components forTile:tileID style:MaplyDataStyleReplace];
    [layer tileDidLoad:tileID];

    CFTimeInterval duration = CFAbsoluteTimeGetCurrent() - start;
    NSLog(@"Added %lu components for %d features for tile %d/%d/%d in %f seconds",
          (unsigned long)components.count, featureCount,
          flippedTile.level, flippedTile.x, flippedTile.y,
          duration);
  });
}


- (int)minZoom {
  if(self.tileSources.count) {
    return [(MaplyRemoteTileInfo*)self.tileSources[0] minZoom];
  } else {
    return 3;
  }
}


- (int)maxZoom {
  if(self.tileSources.count) {
    return [(MaplyRemoteTileInfo*)self.tileSources[0] maxZoom];
  } else {
    return 14;
  }
}


/**
 Convert a coordinate from lat/lon radians to epsg:3785
 Verified output with "cs2cs +init=epsg:4326 +to +init=epsg:3785", correct within .5 meters, 
 but frequently off by .4
 */
- (MaplyCoordinate)toMerc:(MaplyCoordinate)coord {
//  MaplyCoordinate orig = coord;
  coord.x = RadToDeg(coord.x) * MAX_EXTENT / 180;
  coord.y = 3189068.5 * log((1.0 + sin(coord.y)) / (1.0 - sin(coord.y)));
//  NSLog(@"%f %f -> %.2f %.2f", RadToDeg(orig.x), RadToDeg(orig.y), coord.x, coord.y);
  return coord;
}


MaplyTileID tileWithFlippedY(MaplyTileID tileId) {
  MaplyTileID flippedYTile;
  flippedYTile.level = tileId.level;
  flippedYTile.x = tileId.x;
  flippedYTile.y = ((int)(1<<tileId.level)-tileId.y)-1;
  return flippedYTile;
}

@end
