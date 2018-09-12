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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

/**
    Tile fetcher request.
 
    A single request for a single tile of data from a single source.
    The tile fetcher will... fetch this and call the success or failure callback.
  */
@interface MaplyTileFetchRequest : NSObject

/// Priority before importance.  Less is more important.
@property (nonatomic) int priority;
/// How important this is to us.  Probably screen space.
@property (nonatomic) float importance;

/// An object representing the tile source.  Used for sorting.
@property (nonatomic,nonnull,strong) id tileSource;

/// URL to fetch from
@property (nonatomic,nonnull,strong) NSURLRequest *urlReq;
/// File name for cached file (if present).  Save it here when fetched if set.
@property (nonatomic,nonnull,strong) NSString *cacheFile;

/**
    Tile Fetcher success callback.

    Called on a new dispatch queue and won't be marked as loaded until it returns.
    This is a good way to limit how many things are loading/parsing at the same time.
  */
@property (nonatomic,nullable) void (^success)(MaplyTileFetchRequest * __nonnull,NSData * __nonnull);

/**
    Tile Fetcher failure callback.
  */
@property (nonatomic,nullable) void (^failure)(MaplyTileFetchRequest * __nonnull,NSError * __nonnull);

@end

@class MaplyTileFetcher;

/// Stats collected by the fetcher
@interface MaplyTileFetcherStats : NSObject

@property (nonatomic,readonly,weak,nullable) MaplyTileFetcher *fetcher;

// Start of stats collection
@property (nonatomic,nonnull,strong) NSDate *startDate;

// Total requests, remote and cached
@property (nonatomic) int totalRequests;

// Requests that resulted in a remote HTTP call
@property (nonatomic) int remoteRequests;

// Total requests cancelled
@property (nonatomic) int totalCancels;

// Requests failed
@property (nonatomic) int totalFails;

// Bytes of remote data loaded
@property (nonatomic) int remoteData;

// Bytes of cached data loaded
@property (nonatomic) int localData;

// Total time spent waiting for successful remote data requests
@property (nonatomic) NSTimeInterval totalLatency;

// Add the given stats to ours
- (void)addStats:(MaplyTileFetcherStats * __nonnull)stats;

// Print out the stats
- (void)dump;

@end

/**
  Tile fetcher fetches tiles.
 
  The tile fetcher interacts with loaders that want tiles, as demanded by samplers.
  It's complicated.  There's a default one of these that will get used if you pass in nil to the MaplyQuadImageLoader.
  */
@interface MaplyTileFetcher : NSObject

/// Initialize with the number of connections the fetcher can have open at once
- (instancetype __nonnull)initWithName:(NSString * __nonnull)name connections:(int)numConnections;

/// Ask for a tile to be fetched.  Returns an object that can be used to cancel the request.
/// This is just returning the request as the id
- (id __nonnull)startTileFetch:(MaplyTileFetchRequest * __nonnull)request;

/// Add a whole group of requests at once.
/// This is useful if we want to avoid low priority tiles grabbing the slots first
- (void)startTileFetches:(NSArray<MaplyTileFetchRequest *> *__nonnull)requests;

/// Update an active request with a new priority and importance
- (id __nonnull)updateTileFetch:(id __nonnull)fetchID priority:(int)priority importance:(double)importance;

/// Name of this tile fetcher.  Used for coordinating tile sources.
@property (nonatomic,readonly,nonnull) NSString *name;

/// Number of outstanding connections in parallel
@property (nonatomic) int numConnections;

/// Cancel a request being fetched
/// Use the object returned by the startTileFetch call
- (void)cancelTileFetch:(id __nonnull)requestRet;

/// Cancel a group of requests at once
/// Use the object returned by the startTileFetch call (which is just a Request object)
- (void)cancelTileFetches:(NSArray * __nonnull)requestRets;

/// Return the fetching stats since the beginning or since the last reset
- (MaplyTileFetcherStats * __nullable)getStats:(bool)allTime;

/// Reset the counters for one variant of stat
- (void)resetStats;

/// Kill all outstanding connections and clean up
- (void)shutdown;

@end
