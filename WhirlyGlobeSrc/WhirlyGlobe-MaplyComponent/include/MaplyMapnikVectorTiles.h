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
#import "MaplyTileSource.h"

/** @brief Geometry type for data found within PBF files.
    @details These are the geometry types supported within Mapnik PBF files.
  */
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
@class MaplyMBTileSource;

/** @brief Protocol for styling the vectors.
    @details You pass in an object which adheres to this protocol and will style
    the vectors read by a MaplyMapnikVectorTiles object.  In general, this will be
    a parsed Mapnik vector file, but you can substitute your own logic as well.
  */
@protocol VectorStyleDelegate <NSObject>

- (NSArray*)stylesForFeatureWithAttributes:(NSDictionary*)attributes
                      onTile:(MaplyTileID)tileID
                     inLayer:(NSString*)layer;
- (BOOL)layerShouldDisplay:(NSString*)layer;

- (MaplyVectorTileStyle*)styleForUUID:(NSString*)uiid;

@end


/** @brief Provides an demand creation for Mapnik style vector tiles.
    @details Create one of these to read Mapnik PBF style tiles from a remote
    or local source.  This handles the geometry creation, calls a delegate
    for the styling and can read from remote or local data files.
  */
@interface MaplyMapnikVectorTiles : NSObject <MaplyPagingDelegate>

@property (nonatomic, readonly) NSArray *tileSources;
@property (nonatomic, strong) NSObject<VectorStyleDelegate> *styleDelegate;
@property (nonatomic, assign) BOOL debugLabel;
@property (nonatomic, assign) BOOL debugOutline;

/** @brief Init with a single remote tile source.
  */

- (instancetype) initWithTileSource:(NSObject<MaplyTileSource>*)tileSource;

/** @brief Init with a list of tile sources.
    @details These are MaplyRemoteTileInfo objects and will be combined by the
    MaplyMapnikVectorTiles object for display.
*/
- (instancetype) initWithTileSources:(NSArray*)tileSources;

/** @brief Init with the filename of an MBTiles archive containing PBF tiles.
    @details This will read individual tiles from an MBTiles archive containging PBF.
    @details The file should be local.
  */
- (instancetype) initWithMBTiles:(MaplyMBTileSource *)tileSource;

@end
