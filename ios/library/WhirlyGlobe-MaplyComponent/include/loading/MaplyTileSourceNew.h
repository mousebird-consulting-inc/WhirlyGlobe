/*
 *  MaplyTileSourceNew.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2011-2019 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "loading/MaplyImageTile.h"
#import "math/MaplyCoordinate.h"
#import "math/MaplyCoordinateSystem.h"

/**
 This represents the indentifier for a unique tile in the pyramid.
 
 Each tile in an image (or vector tile) pyramid can be uniquely
 identified by an x, y, and level.  The levels start at zero and run to
 a maximum.  x and y also start at zero and run to 2^level-1.
 
 
 How these are interpreted is up to the tile source itself.  For example, some tile sources start at the lower left for (0,0) and run to the upper left.  Others do it the opposite way.  There's a flipY option in the MaplyQuadImageTileLayer to deal with this, but the system doesn't care all that much as long as you are consistent.
 @see MaplyTileSource
 @see MaplyQuadPagingLayer
 @see MaplyQuadImageTilesLayer
 */
typedef struct
{
    int x, y, level;
} MaplyTileID;

/// Convert a MaplyTileID to an NSString
NSString *__nonnull MaplyTileIDString(MaplyTileID tileID);

/**
    Tile Info Protocol.
 
    This describes a single source of data tiles.  A uses these to
    figure out what to load when and where.  The loader passes the result of
    fetchInfoForTile to a MaplyTileFetcher to get the data it wants.
  */
@protocol MaplyTileInfoNew<NSObject>

/**
 The minimum zoom level available.
 
 This is the lowest level we'll try to fetch.  Any levels below that will be filled in with placeholders.  Those are empty, but they allow us to load tiles beneath.
 */
- (int)minZoom;

/**
 The maximum zoom level available.
 
 This is the highest level (e.g. largest) that we'll
 fetch for a given pyramid tile source.  The source can sparse,
 so you are not required to have these tiles available, but this
 is as high as the loader will fetch.
 */
- (int)maxZoom;

/**
  FetchInfo object for a given tile.
 
  The FetchInfo object is specific to the type of TileFetcher you're using and
  tells the fetcher how to get the data you wawnt.
  RemoteTileFetchers want a RemoteTileInfoNew object.
 */
- (id __nullable)fetchInfoForTile:(MaplyTileID)tileID flipY:(bool)flipY;

@end

/**
  A tile source that just returns nil.  You can use this like a tile source where you need one that doesn't do anything.
 */
@interface MaplyTileInfoNone : NSObject<MaplyTileInfoNew>
@end



/**
 Generic Tile fetcher request.
 
 A single request for a single tile of data from a single source.
 The tile fetcher will... fetch this and call the success or failure callback.
 */
@interface MaplyTileFetchRequest : NSObject

/// Priority before importance.  Less is more important.
@property (nonatomic) int priority;
/// How important this is to us.  Probably screen space.
@property (nonatomic) float importance;
/// If all other values are equal, sort by this.
/// It keeps requests we're waiting for grouped together
@property (nonatomic) int group;

/// An object representing the tile source.  Used for sorting.  Not accessed by the fetcher.
@property (nonatomic,nonnull,strong) id tileSource;

/** This is requested from a TileInfo object by a loader and then passed
 along to the TileFetcher.  TileFetchers expect certain objects.
 The RemoteTileFetcher wants a RemoteFetchInfo object and will check.
 Other fetchers will want other things.
 */
@property (nonatomic,nonnull,strong) id fetchInfo;

/**
 Tile Fetcher success callback.
 
 Called on a new dispatch queue and won't be marked as loaded until it returns.
 This is a good way to limit how many things are loading/parsing at the same time.
 */
@property (nonatomic,nullable) void (^success)(MaplyTileFetchRequest * __nonnull,id __nonnull);

/**
 Tile Fetcher failure callback.
 */
@property (nonatomic,nullable) void (^failure)(MaplyTileFetchRequest * __nonnull,NSError * __nonnull);

@end

/**
    Tile Fetcher protocol.
 
    The tile fetcher interacts with loaders that want tiles, as demanded by samplers.
    A given data source (e.g. remote URL, MBTiles) needs one of these to fetch and return data.
  */
@protocol MaplyTileFetcher<NSObject>

/// Add a whole group of requests at once.
/// This is useful if we want to avoid low priority tiles grabbing the slots first
- (void)startTileFetches:(NSArray<MaplyTileFetchRequest *> *__nonnull)requests;

/// Update an active request with a new priority and importance
- (id __nonnull)updateTileFetch:(id __nonnull)fetchID priority:(int)priority importance:(double)importance;

/// Name of this tile fetcher.  Used for coordinating tile sources.
- (NSString * __nonnull)name;

/// Cancel a group of requests at once
/// Use the object returned by the startTileFetch call (which is just a Request object)
- (void)cancelTileFetches:(NSArray * __nonnull)requestRets;

/// Kill all outstanding connections and clean up
- (void)shutdown;

@end
