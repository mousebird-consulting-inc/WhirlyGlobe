/*
 *  MaplyMapnikVectorTiles.h
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


#import <Foundation/Foundation.h>
#import "MaplyQuadPagingLayer.h"

typedef enum  {
  GeomTypeUnknown = 0,
  GeomTypePoint = 1,
  GeomTypeLineString = 2,
  GeomTypePolygon = 3
} MapnikGeometryType;

typedef enum {
  SEG_END    = 0,
  SEG_MOVETO = 1,
  SEG_LINETO = 2,
  SEG_CLOSE = (0x40 | 0x0f)
} MapnikCommandType;

@class MaplyVectorTileStyle;
@class MaplyRemoteTileInfo;

@protocol VectorStyleDelegate <NSObject>

- (NSArray*)stylesForFeature:(MaplyVectorObject*)feature
                  attributes:(NSDictionary*)attributes
                      onTile:(MaplyTileID)tileID
                     inLayer:(NSString*)layer;
- (BOOL)layerShouldDisplay:(NSString*)layer;

- (MaplyVectorTileStyle*)styleForUUID:(NSString*)uiid;

@end


@interface MaplyMapnikVectorTiles : NSObject <MaplyPagingDelegate>

@property (nonatomic, readonly) NSArray *tileSources;
@property (nonatomic, strong) NSObject<VectorStyleDelegate> *styleDelegate;
@property (nonatomic, assign) BOOL debugLabel;
@property (nonatomic, assign) BOOL debugOutline;

- (instancetype) initWithTileSource:(MaplyRemoteTileInfo*)tileSource;
- (instancetype) initWithTileSources:(NSArray*)tileSources;
- (void)setMaxConcurrentOperationCount:(NSUInteger)count;

@end
