/*
 *  NetworkTileQuadSource.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/11/12.
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

#import "QuadDisplayLayer.h"
#import "SphericalMercator.h"
#import "TileQuadLoader.h"

/** Base class shared between tile and tilespec versions.
    Use those directly, not this.
  */
@interface WhirlyKitNetworkTileQuadSourceBase : NSObject<WhirlyKitQuadDataStructure>

/// Number of simultaneous fetches.  Defaults to 4.
@property (nonatomic,assign) int numSimultaneous;
/// Location of cache, if set
@property (nonatomic,retain) NSString *cacheDir;
/// If set, we call this delegate to get some elevation
@property (nonatomic,weak) NSObject<WhirlyKitElevationHelper> *elevDelegate;

@end

/** Network Tile Quad Source.
    This implements a tile source for the standard http level/x/y
    image hiearachy.
 */
@interface WhirlyKitNetworkTileQuadSource : WhirlyKitNetworkTileQuadSourceBase<WhirlyKitQuadTileImageDataSource>

/// Where we're fetching from
@property (nonatomic) NSString *baseURL;
/// Image extension
@property (nonatomic) NSString *ext;

/// Initialize with the base URL and image extension (e.g. png, jpg)
- (id)initWithBaseURL:(NSString *)base ext:(NSString *)imageExt;

- (void)setMinZoom:(int)zoom;
- (void)setMaxZoom:(int)zoom;

@end

/** Network TileSpec based quad source.
    This version reads from an NSDictionary that's been parsed out of JSON that corresponds to the
    TileSpec https://github.com/mapbox/tilejson-spec.  The main difference is that it can deal with
    multiple URLs to pull from.
 */
@interface WhirlyKitNetworkTileSpecQuadSource : WhirlyKitNetworkTileQuadSourceBase<WhirlyKitQuadTileImageDataSource>

@property (nonatomic) NSArray *tileURLs;

/// Initialize with an NSDictionary that's been parsed from TileSpec JSON
- (id)initWithTileSpec:(NSDictionary *)jsonDict;

@end

