/*
 *  NetworkTileQuadSource.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/11/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "QuadDisplayLayer.h"
#import "SphericalMercator.h"
#import "TileQuadLoader.h"

/** Network Tile Quad Source.
    This implements a tile source for the standard http level/x/y
    image hiearachy.
 */
@interface WhirlyKitNetworkTileQuadSource : NSObject<WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource>
{
    /// Where we're fetching from
    NSString *baseURL;
    /// Image extension
    NSString *ext;
    /// Spherical Mercator coordinate system, for the tiles
    WhirlyKit::SphericalMercatorCoordSystem *coordSys;
    /// Bounds in Spherical Mercator
    WhirlyKit::Mbr mbr;
    /// Available levels, as read from the database.
    /// You can modify these yourself as well, to limit what's loaded
    int minZoom,maxZoom;
    /// Number of simultaneous fetches.  Defaults to 4.
    int numSimultaneous;
    /// Size of a tile in pixels square.  256 is the usual.
    int pixelsPerTile;   
    /// Location of cache, if set
    NSString *cacheDir;
    // Flip Y coordinate
    BOOL flipY;
}

@property (nonatomic,assign) int numSimultaneous;
@property (nonatomic,retain) NSString *cacheDir;
@property (nonatomic,assign) BOOL flipY;

/// Initialize with the base URL and image extension (e.g. png, jpg)
- (id)initWithBaseURL:(NSString *)base ext:(NSString *)imageExt;

- (void)setMinZoom:(int)zoom;
- (void)setMaxZoom:(int)zoom;

@end

