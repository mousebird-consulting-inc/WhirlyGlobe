/*
 *  MaplyTileFetcher.mm
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

#import "MaplyTileFetcher_private.h"
#import "MaplyRenderController_private.h"

namespace WhirlyKit
{
    
// Standard Node with level, x, y and a frame for sorting
class TileIdent : public QuadTreeNew::Node
{
public:
    TileIdent(const TileIdent &that) : Node(that), tileInfo(that.tileInfo), frame (that.frame) { }
    /// Construct with the cell coordinates, level, and frame
    TileIdent(const QuadTreeNew::Node &node,MaplyRemoteTileInfo *tileInfo,int frame) : QuadTreeNew::Node(node), tileInfo(tileInfo), frame(frame) { }
    TileIdent(int x,int y,int level,MaplyRemoteTileInfo *tileInfo,int frame) : frame(frame), tileInfo(tileInfo), QuadTreeNew::Node(x,y,level) { }
    TileIdent(const MaplyTileID &tileID,MaplyRemoteTileInfo *tileInfo,int frame) : tileInfo(tileInfo), frame(frame) { level = tileID.level;  x = tileID.x;  y = tileID.y; }
    
    /// Comparison based on x,y,level.  Used for sorting
    bool operator < (const TileIdent &that) const
    {
        if ((Node &)*this == (Node &)that) {
            if (tileInfo == that.tileInfo) {
                return frame < that.frame;
            }
            return tileInfo < that.tileInfo;
        } else
            return (Node &)*this < (Node &)that;
    }
    
    /// Equality operator
    bool operator == (const TileIdent &that) const
    {
        if ((Node &)*this == (Node &)that) {
            return frame == that.frame && tileInfo == that.tileInfo;
        } else
            return false;
    }
    
    /// Not equal operator
    bool operator != (const TileIdent &that) const
    {
        if ((Node &)*this != (Node &)that)
            return frame != that.frame && tileInfo != that.tileInfo;
        else
            return false;
    }
    
    // Remote source for the tile
    MaplyRemoteTileInfo *tileInfo;
    
    // Frame for this tile load
    int frame;
};

// A single tile that we're aware of
class TileInfo
{
public:
    TileInfo() : state(), importance(0.0), ident(0,0,0,nil,-1), request(nil), task(nil) { }

    /// Comparison based on importance, tile source, then x,y,level
    bool operator < (const TileInfo &that) const
    {
        if (this->importance == that.importance) {
            if (ident == that.ident) {
                return ident < that.ident;
            }
            return ident < that.ident;
        }
        return this->importance < that.importance;
    }

    // We're either loading it or going to load it eventually
    typedef enum {ToLoad,Loading} State;
    State state;
    
    // Identifier for this request (TileID + frame)
    TileIdent ident;
    
    // Importance of this tile request as passed in by the fetch request
    double importance;
    
    // The request as it came from outside the tile fetcher
    MaplyTileFetchRequest *request;
    
    // If we're loading it, this is the data task associated with it
    NSURLSessionDataTask *task;
};


typedef std::shared_ptr<TileInfo> TileInfoRef;
typedef struct {
    bool operator () (const TileInfoRef a,const TileInfoRef b) const {
        return *a < *b;
    }
} TileInfoSorter;
typedef std::map<TileIdent,TileInfoRef> TileInfoMap;
typedef std::set<TileInfoRef,TileInfoSorter> TileInfoSet;

}

using namespace WhirlyKit;

@implementation MaplyTileFetchRequest

-(instancetype)init
{
    self = [super init];
    _tileID.level = 0;  _tileID.x = 0;  _tileID.y = 0;
    _frame = -1;
    _importance = 0.0;
    
    _success = nil;
    _failure = nil;
    
    return self;
}

@end

@implementation MaplyTileFetcher
{
    bool active;
    NSURLSession *session;
    int numConnectionsMax;
    dispatch_queue_t queue;
    
    TileInfoMap tiles;  // All the tiles we're supposed to be loading
    TileInfoSet loading;  // Tiles that are currently loading
    TileInfoSet toLoad;  // Tiles sorted by importance
    
    int totalRequests;
    int totalCancels;
    int totalFails;
    int remoteData;
    int localData;
}

- (instancetype)initWithConnections:(int)numConnections
{
    self = [super init];
    active = true;
    numConnectionsMax = numConnections;
    // All the internal work is done on a single queue.  Nothing significant, really.
    queue = dispatch_queue_create("MaplyTileFetcher", nil);
    session = [NSURLSession sharedSession];
    totalRequests = 0;
    totalCancels = 0;
    totalFails = 0;
    remoteData = 0;
    localData = 0;
    
    return self;
}

- (void)setStatsPeriod:(NSTimeInterval)statsPeriod
{
    _statsPeriod = statsPeriod;
    
    [self statsDump];
}

- (void)statsDump
{
    if (!active)
        return;
    
    NSLog(@"---MaplyTileFetcher Stats---");
    NSLog(@"   Total Requests = %d",totalRequests);
    NSLog(@"   Canceled Requests = %d",totalCancels);
    NSLog(@"   Failed Requests = %d",totalFails);
    NSLog(@"   Data Transferred = %.2fMB",remoteData / (1024.0*1024.0));
    NSLog(@"   Cached Data = %.2fMB",localData / (1024.0*1024.0));

    if (_statsPeriod > 0.0) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(_statsPeriod * NSEC_PER_SEC)), queue, ^{
            [self statsDump];
        });
    }
}

- (id)startTileFetch:(MaplyTileFetchRequest *)request
{
    if (!active)
        return nil;
    
    dispatch_async(queue,
    ^{
        [self startTileFetchLocal:request];
    });
    
    return request;
}

- (void)cancelTileFetch:(MaplyTileFetchRequest *)request
{
    if (!active)
        return;

    dispatch_async(queue,
    ^{
        [self cancelTileFetchLocal:request];
    });
}

// Run on the dispatch queue
- (void)startTileFetchLocal:(MaplyTileFetchRequest *)request
{
    totalRequests++;
    
    TileIdent ident(request.tileID,request.tileInfo,request.frame);
    auto it = tiles.find(ident);
    if (it != tiles.end()) {
        // It's already loading, so punt
        NSLog(@"MaplyTileFetcher: Client requested the same tile fetched twice. Ignoring.");
        return;
    }
    
    // Set up new request
    TileInfoRef tile(new TileInfo());
    tile->ident = ident;
    tile->importance = request.importance;
    tile->state = TileInfo::ToLoad;
    tile->request = request;
    tiles[ident] = tile;
    toLoad.insert(tile);
    
    [self updateLoading];
}

// Run on the dispatch queue
- (void)cancelTileFetchLocal:(MaplyTileFetchRequest *)request
{
    totalCancels++;
    
    TileIdent ident(request.tileID,request.tileInfo,request.frame);
    auto it = tiles.find(ident);
    if (it == tiles.end()) {
        // Wasn't there.  Ignore.
        return;
    }
    TileInfoRef tile = it->second;
    switch (tile->state) {
        case TileInfo::Loading:
            [tile->task cancel];
            loading.erase(tile);
            break;
        case TileInfo::ToLoad:
            toLoad.erase(tile);
            break;
    }
    tile->task = nil;
    tiles.erase(it);
    
    [self updateLoading];
}

// Run on the dispatch queue
- (void)updateLoading
{
    // Ask for a few more to load
    while (loading.size() < numConnectionsMax) {
        auto nextLoad = toLoad.rbegin();
        if (nextLoad == toLoad.rend())
            break;
        
        // Move it into loading
        TileInfoRef tile = *nextLoad;
        toLoad.erase(std::next(nextLoad).base());
        tile->state = TileInfo::Loading;
        loading.insert(tile);
        
        MaplyTileID tileID;
        tileID.level = tile->ident.level;
        tileID.x = tile->ident.x;
        tileID.y = tile->ident.y;
        int frame = tile->ident.frame;
        auto tileInfo = tile->ident.tileInfo;
        NSURLRequest *urlReq = [tileInfo requestForTile:tileID];
        
        // Set up the fetch task so we can use it in a couple places
        tile->task = [session dataTaskWithRequest:urlReq completionHandler:
                      ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
                          dispatch_async(self->queue,
                                         ^{
                              if (error) {
                                  // Cancels don't count as errors
                                  if (error.code != NSURLErrorCancelled) {
                                      self->totalFails++;
                                      [self finishedLoading:tile data:nil error:error];
                                  }
                              } else {
                                  self->remoteData += [data length];
                                  [self finishedLoading:tile data:data error:error];
                                  [tileInfo writeToCache:tileID tileData:data];
                              }
                        });
                      }];
        
        // Look for it cached
        if ([tile->ident.tileInfo tileIsLocal:tileID frame:frame]) {
            // Do the reading somewhere else
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                NSData *data = [tileInfo readFromCache:tileID];
                if (!data) {
                    // It failed (which happens) so we need to fetch it after all
                    [tile->task resume];
                } else {
                    // It worked, but run the finished loading back on our queue
                    dispatch_async(self->queue,^{
                        self->localData += [data length];
                        [self finishedLoading:tile data:data error:nil];
                    });
                }
            });
        } else {
            [tile->task resume];
        }
    }
}

// Called on our queue
- (void)finishedLoading:(TileInfoRef)tile data:(NSData *)data error:(NSError *)error
{
    auto it = tiles.find(tile->ident);
    if (it == tiles.end())
        // No idea what it is.  Toss it.
        return;
    
    if (data) {
        tile->request.success(tile->request,data);
    } else {
        tile->request.failure(tile->request, error);
    }
    
    loading.erase(tile);
    toLoad.erase(tile);
    tiles.erase(it);

    [self updateLoading];
}

- (void)shutdown
{
    active = false;
    
    // Execute an empty task and wait for it to return
    // This drains the queue
    dispatch_sync(queue, ^{});
    
    toLoad.clear();
    loading.clear();
    tiles.clear();
}

@end
