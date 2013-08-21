/*
 *  MaplyQuadEarthWithRemoteTiles.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/24/12.
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

#import "MaplyViewControllerLayer.h"

/** The Quad Earth With Remote Tiles object represents a pageable map layer which pulles
    tiles form a remote source
  */
@interface MaplyQuadEarthWithRemoteTiles : MaplyViewControllerLayer

/// Set up a spherical earth layer with a remote set of tiles.
/// Returns nil on failure.
- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Set up a spherical earth layer with a remote set of tiles defined by the tilespec
///  in JSON (that's been parsed into an NSDictionary).
- (id)initWithTilespec:(NSDictionary *)jsonDict;

/// Off by default, if set this will create skirts around the tiles
@property (nonatomic,assign) bool handleEdges;

/// The number of simultaneous fetches allowed at once
@property (nonatomic,assign) int numSimultaneousFetches;

/// If set, the directory to cache tiles in.  There's no cleanup, so beware
@property (nonatomic,strong) NSString *cacheDir;

@end
