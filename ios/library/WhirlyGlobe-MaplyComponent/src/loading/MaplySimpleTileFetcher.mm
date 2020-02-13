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

#import "MaplySimpleTileFetcher.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Encapsulates a single tile load request
@interface MaplySimpleTileFetchInfo : NSObject

@property (nonatomic,assign) int x;
@property (nonatomic,assign) int y;
@property (nonatomic,assign) int level;

@end

@implementation MaplySimpleTileFetchInfo
@end

// Internal object used by the QuadImageLoader to generate tile load info
@interface MaplySimpleTileInfo : NSObject<MaplyTileInfoNew>
@end

@implementation MaplySimpleTileInfo
{
    int minZoom,maxZoom;
}

- (instancetype)initWithMinZoom:(int)inMinZoom maxZoom:(int)inMaxZoom
{
    self = [super init];
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
    
    return self;
}

- (id _Nullable)fetchInfoForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    MaplySimpleTileFetchInfo *fetchInfo = [[MaplySimpleTileFetchInfo alloc] init];
    fetchInfo.x = tileID.x;
    fetchInfo.y = tileID.y;
    fetchInfo.level = tileID.level;
    
    return fetchInfo;
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

@end

// A single tile that we're aware of
class TileInfo
{
public:
    TileInfo() : priority(0), importance(0.0), request(nil), fetchInfo(nil) { }
    
    /// Comparison based on importance, tile source, then x,y,level
    bool operator < (const TileInfo &that) const
    {
        if (this->priority == that.priority) {
            if (this->importance == that.importance) {
                return this->request < that.request;
            }
            return this->importance < that.importance;
        }
        return this->priority > that.priority;
    }
    
    // Priority before importance
    int priority;
    
    // Importance of this tile request as passed in by the fetch request
    double importance;
    
    // The request as it came from outside the tile fetcher
    MaplyTileFetchRequest *request;
    
    // Specific fetchInfo from the fetch request.
    MaplySimpleTileFetchInfo *fetchInfo;
};

typedef std::shared_ptr<TileInfo> TileInfoRef;
typedef struct {
    bool operator () (const TileInfoRef a,const TileInfoRef b) const {
        return *a < *b;
    }
} TileInfoSorter;
typedef std::set<TileInfoRef,TileInfoSorter> TileInfoSet;
typedef std::map<MaplyTileFetchRequest *,TileInfoRef> TileFetchMap;

@implementation MaplySimpleTileFetcher
{
    bool active;
    bool loadScheduled;
    int minZoom,maxZoom;
    MaplySimpleTileInfo *tileInfo;
    
    TileInfoSet toLoad;  // Tiles sorted by importance
    TileFetchMap tilesByFetchRequest;  // Tiles sorted by fetch request
}

- (nullable instancetype)initWithName:(NSString *)name minZoom:(int)inMinZoom maxZoom:(int)inMaxZoom;
{
    self = [super init];
    if (!self)
        return nil;
    _name = name;
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
    _neverFail = true;
    
    tileInfo = [[MaplySimpleTileInfo alloc] initWithMinZoom:minZoom maxZoom:maxZoom];
    _queue = dispatch_queue_create([_name cStringUsingEncoding:NSASCIIStringEncoding], DISPATCH_QUEUE_SERIAL);
    
    active = true;
    return self;
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

- (NSObject<MaplyTileInfoNew> *)tileInfo
{
    return tileInfo;
}

- (id)dataForTile:(id)fetchInfo tileID:(MaplyTileID)tileID
{
    NSLog(@"MaplySimpleTileFetcher: You forgot to fill in the dataForTile:tileID: method in your subclass.");
    
    return nil;
}

- (void)updateLoading
{
    loadScheduled = false;
    
    if (!active)
        return;
    
    if (toLoad.empty())
        return;
    
    // Take the first one off the stack
    auto it = toLoad.rbegin();
    TileInfoRef tile = *it;
    
    // The actual data fetch.  Woo.
    MaplyTileID tileID;
    tileID.level = tile->fetchInfo.level;
    tileID.x = tile->fetchInfo.x;    tileID.y = tile->fetchInfo.y;
    id tileData = [self dataForTile:tile->fetchInfo tileID:tileID];
    NSError *error = nil;
    if (!tileData) {
        error = [[NSError alloc] initWithDomain:@"MaplySimpleTileFetcher" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to fetch tile from sqlite file"}];
    }
    
    MaplySimpleTileFetcher * __weak weakSelf = self;
    // Do the callback on a background queue
    // Because the parsing might take a while
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       // We assume the parsing is going to take some time
                       if (tileData || self.neverFail) {
                           tile->request.success(tile->request,tileData);
                       } else {
                           tile->request.failure(tile->request, error);
                       }
                       
                       dispatch_queue_t theQueue = weakSelf.queue;
                       if (theQueue)
                           dispatch_async(theQueue,
                                          ^{
                                              [weakSelf updateLoading];
                                          });
                   });
    
    [weakSelf finishTile:tile];
}

- (void)finishTile:(TileInfoRef)tile
{
    // Done with the tile, so take it out of here
    toLoad.erase(tile);
    auto rit = tilesByFetchRequest.find(tile->request);
    if (rit != tilesByFetchRequest.end())
        tilesByFetchRequest.erase(rit);
}

- (void)scheduleLoading
{
    if (!active)
        return;
    
    if (!loadScheduled) {
        loadScheduled = true;
        dispatch_async(self.queue, ^{
            [self updateLoading];
        });
    }
}

- (void)startTileFetches:(NSArray<MaplyTileFetchRequest *> * _Nonnull)requests
{
    if (!active)
        return;
    
    // Check each of the fetchInfo objects
    for (MaplyTileFetchRequest *request in requests)
        if (![request.fetchInfo isKindOfClass:[MaplySimpleTileFetchInfo class]]) {
            NSLog(@"MaplyMBTileFetcher is expecting MaplyMBTileFetchInfo objects.  Rejecting requests.");
            return;
        }
    
    dispatch_async(self.queue, ^{
        for (MaplyTileFetchRequest *request in requests) {
            // Set up new request
            TileInfoRef tile(new TileInfo());
            tile->importance = request.importance;
            tile->priority = request.priority;
            tile->request = request;
            tile->fetchInfo = request.fetchInfo;
            self->tilesByFetchRequest[request] = tile;
            self->toLoad.insert(tile);
        }
        
        [self scheduleLoading];
    });
}

- (void)cancelTileFetches:(NSArray * _Nonnull)requests
{
    if (!active)
        return;
    
    dispatch_async(self.queue, ^{
        for (MaplyTileFetchRequest *request in requests) {
            auto it = self->tilesByFetchRequest.find(request);
            if (it == self->tilesByFetchRequest.end()) {
                // Wasn't there.  Ignore.
                return;
            }
            TileInfoRef tile = it->second;
            self->toLoad.erase(tile);
            self->tilesByFetchRequest.erase(it);
        }
    });
}

- (id _Nonnull)updateTileFetch:(id _Nonnull)request priority:(int)priority importance:(double)importance
{
    if (!active)
        return nil;
    
    dispatch_async(self.queue, ^{
        auto it = self->tilesByFetchRequest.find(request);
        if (it == self->tilesByFetchRequest.end())
            return;
        
        // Reinsert the tile with the new values
        TileInfoRef tile = it->second;
        self->toLoad.erase(tile);
        tile->priority = priority;
        tile->importance = importance;
        self->toLoad.insert(tile);
    });
    
    return request;
}

- (void)shutdown
{
    active = false;
    
    // Execute an empty task and wait for it to return
    // This drains the queue
    dispatch_sync(self.queue, ^{});
    
    _queue = nil;
}

@end
