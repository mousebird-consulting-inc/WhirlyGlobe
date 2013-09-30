/*
 *  MaplyRemoteTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

@class MaplyRemoteTileSource;

/** The remote tile source delegate provides feedback on which
    tiles loaded and which didn't.  You'll be called in all sorts of
    random threads here, so act accordingly.
  */
@protocol MaplyRemoteTileSourceDelegate <NSObject>

@optional

/** The tile successfully loaded.
    @param tileSource the remote tile source that loaded the tile.
    @param tileID The ID of the tile we loaded.
  */
- (void) remoteTileSource:(MaplyRemoteTileSource *)tileSource tileDidLoad:(MaplyTileID)tileID;

/** The tile failed to load.
    @param tileSource The remote tile source that tried to load the tile.
    @param tileId The tile ID of the tile that failed to load.
    @param error The NSError message, probably from the network routine.
  */
- (void) remoteTileSource:(MaplyRemoteTileSource *)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *)error;

@end

/** MapBox Tiles archive tile source.  This object knows how to read MBTiles
 files and return the appropriate tile when asked.
 */
@interface MaplyRemoteTileSource : NSObject<MaplyTileSource>

/// Create with the base URL (where to fetch the tiles), the image extension (e.g. png) and min and max
///  zoom levels.
- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Create with a JSON tile spec.  This contains all the usual info.
- (id)initWithTilespec:(NSDictionary *)jsonSpec;

/// Base URL we're fetching tiles from
@property (nonatomic,readonly) NSString *baseURL;

/// We can limit the max zoom if we like
@property (nonatomic) int maxZoom;

/// Image extension (e.g. png, jpeg)
@property (nonatomic,readonly) NSString *ext;

/// Number of pixels along the side of a tile (256 by default)
@property (nonatomic,readonly) int pixelsPerSide;

/// Coordinate system for the remote tiles.  Spherical Mercator by default.
/// Set this to something else if you have something more unique.
@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

/// If set, we'll store (and look for) cache images here.
@property (nonatomic) NSString *cacheDir;

/// If set, you'll get callbacks when the various tiles load (or don't).
/// You get called in all sorts of threads.  Act accordingly.
@property (nonatomic,weak) NSObject<MaplyRemoteTileSourceDelegate> *delegate;

@end
