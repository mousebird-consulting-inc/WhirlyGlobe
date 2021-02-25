/*
 *  MaplySimpleTileFetcher.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/31/19.
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

#import "MaplyTileSourceNew.h"

/** Simple Tile Fetcher is meant for sub classing.
 
    Some data sources aren't all that complex.  You read from a local source,
    you return the data.  Something else turns it into visible objects.  Simple.
 
    To implement one of those, subclass the Simple Tile Fetcher and let it do the
    tricky bits.
  */
@interface MaplySimpleTileFetcher : NSObject<MaplyTileFetcher>

/// Your Subclass must call this init method
- (nullable instancetype)initWithName:(NSString * __nonnull)name minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// The quad paging loader variants need a TileInfo object, even if it's very simple
- (nullable NSObject<MaplyTileInfoNew> *)tileInfo;

/// Dispatch queue the data fetcher is doing its work on
@property (nonnull) dispatch_queue_t queue;

/// Set by default.  We won't every return an error on failing to load.  Useful for sparse data sets
@property bool neverFail;

/// Name used for debugging
@property NSString * __nonnull name;

/// Min zoom level
- (int)minZoom;

/// Max zoom level
- (int)maxZoom;

/** Override dataForTile:tileID: to return your own data for a given tile.
    The fetchInfo can be a custom object (if you set it up that way) or
    you can just use the tileID argument.
 
    You'll be called on the dispatch queue.
 
    You can return either an NSData or a MaplyLoaderReturn
  */
- (id __nullable)dataForTile:(id __nonnull)fetchInfo tileID:(MaplyTileID)tileID;

/** Override the shutdown method.
 
    Call the superclass shutdown method *first* and then run your own shutdown.
  */
- (void)shutdown;

@end
