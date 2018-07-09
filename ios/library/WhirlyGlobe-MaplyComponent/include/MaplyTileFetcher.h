/*
 *  MaplyTileFetcher_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/15/18.
 *  Copyright 2011-2018 Saildrone Inc
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
#import "MaplyCoordinateSystem.h"

/**
    Tile fetcher request.
 
    A single request for a single tile of data from a single source.
    The tile fetcher will... fetch this and call the success or failure callback.
  */
@interface MaplyTileFetchRequest : NSObject

/// The remote URL (and other info) for this request
@property (nonatomic,nonnull) MaplyRemoteTileInfo *tileInfo;

/// Tile we'd like for the remote request
@property (nonatomic) MaplyTileID tileID;
/// Frame, if doing animation, or -1 if not
@property (nonatomic) int frame;
/// How important this is to us.  Probably screen space.
@property (nonatomic) float importance;

/**
    Tile Fetcher success callback.

    Called on the Tile Fetcher's dispatch queue.  If you're going to do some work, do it
    somewhere else.
  */
@property (nonatomic,nullable) void (^success)(MaplyTileFetchRequest * __nonnull,NSData * __nonnull);

/**
    Tile Fetcher failure callback.
  */
@property (nonatomic,nullable) void (^failure)(MaplyTileFetchRequest * __nonnull,NSError * __nonnull);

@end

/**
  Tile fetcher fetches tiles.
 
  The tile fetcher interacts with loaders that want tiles, as demanded by samplers.
  It's complicated.  There's a default one of these that will get used if you pass in nil to the MaplyQuadImageLoader.
  */
@interface MaplyTileFetcher : NSObject

/// If set to non-zero we'll report out stats every few seconds
@property (nonatomic) NSTimeInterval statsPeriod;

/// Initialize with the number of connections the fetcher can have open at once
- (instancetype __nonnull)initWithConnections:(int)numConnections;

/// Ask for a tile to be fetched.  Returns an object that can be used to cancel the request.
- (id __nonnull)startTileFetch:(MaplyTileFetchRequest * __nonnull)request;

/// Cancel a request being fetched
/// Use the object returned by the startTileFetch call
- (void)cancelTileFetch:(id __nonnull)requestRet;

/// Kill all outstanding connections and clean up
- (void)shutdown;

@end
