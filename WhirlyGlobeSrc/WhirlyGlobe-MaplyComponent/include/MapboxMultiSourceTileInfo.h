/*
 *  MapboxMultiSourcetileInfo.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/23/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyRemoteTileSource.h"
#import "MaplyMapnikVectorTiles.h"

/** @brief Used to represent a Mapbox source compiled from multiple individual sources.
    @details You can use these to merge different sources together (assuming they can be merged), returning image and/or vector tiles.
  */
@interface MapboxMultiSourceTileInfo : MaplyRemoteTileInfo <MaplyRemoteTileSourceDelegate>

/** @brief Initialze the multi source tile info object.  Fill in everything else later.
  */
- (id)initWithViewC:(MaplyBaseViewController *)viewC;

/** @brief The object needs this if you're parsing vector tiles.
  */
@property (nonatomic,weak) MaplyQuadImageTilesLayer *imageLayer;

/** @brief The cache directory where we'll store combo tiles.
    @details If set, we'll store the cached returns for image and/or vector tiles.  If not set, we won't cache.
  */
@property (nonatomic) NSString *cacheDir;

/** @brief If set, we'll use an access key to get map data.
  */
@property (nonatomic) NSString *accessToken;

/** @brief Add an image based map covering the given levels.
    @details This adds an image based map of the given type (e.g. jpg or png) at the given levels.
  */
- (bool)addImageMap:(NSString *)map minZoom:(int)minZoom maxZoom:(int)maxZoom type:(NSString *)imageType;

/** @brief Add a vector map at the given levels.
    @details This adds a vector map for the given levels
  */
- (bool)addVectorMap:(NSString *)map style:(NSData *)styleSheet styleType:(MapnikStyleType)styleType minZoom:(int)minZoom maxZoom:(int)maxZoom;

/** @brief Add a vector or image map, depending on the tile spec.
    @details This version parses a tile spec and figures out what we're loading from there.
  */
- (bool)addTileSpec:(NSDictionary *)tileSpec;

/** @brief Add a vector or image map, depending on the tile spec.
    @details This version parses a tile spec and figures out what we're loading from there.
    @details You can also override which zoom levels to use.
 */
- (bool)addTileSpec:(NSDictionary *)tileSpec minZoom:(int)minZoom maxZoom:(int)maxZoom;

@end
