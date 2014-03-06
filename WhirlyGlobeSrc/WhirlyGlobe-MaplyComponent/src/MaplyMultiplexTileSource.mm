/*
 *  MaplyMultiplexTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/5/13.
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

#import <vector>
#import <set>
#import "MaplyMultiplexTileSource.h"
#import "AFHTTPRequestOperation.h"
#import "MaplyQuadImageTilesLayer.h"

namespace Maply
{
// Used to store active tile fetching operations
class TileFetch
{
public:
    TileFetch(int which,AFHTTPRequestOperation *op) : which(which), op(op) { }
    TileFetch() : which(-1), op(nil) { }
    
    bool operator < (const TileFetch &that) const
    {
        return which < that.which;
    }
    
    int which;
    AFHTTPRequestOperation *op;
};
typedef std::set<TileFetch> TileFetchSet;
    
// Used to store lists of tile data by tile ID
class SortedTile
{
public:
    SortedTile(MaplyTileID tileID)
    : tileID(tileID)
    {
    }
    SortedTile(MaplyTileID tileID,int depth)
    : tileID(tileID)
    {
        tileData.resize(depth,nil);
    }
    
    bool operator < (const SortedTile &that) const
    {
        if (tileID.level == that.tileID.level)
        {
            if (tileID.x == that.tileID.x)
                return tileID.y < that.tileID.y;
            else
                return tileID.x < that.tileID.x;
        } else
            return tileID.level < that.tileID.level;
    }
    
    // Kill any outstanding fetches
    void cancel()
    {
        for (TileFetchSet::iterator it = fetches.begin();
             it != fetches.end(); ++it)
        {
            [it->op cancel];
        }
        fetches.clear();
        tileData.clear();
    }
    
    MaplyTileID tileID;
    // What we're currently fetching
    TileFetchSet fetches;
    // The data we've already fetched
    std::vector<id> tileData;
};

typedef std::set<SortedTile> SortedTileSet;

}

@implementation MaplyMultiplexTileSource
{
    NSArray *_tileSources;
    int _minZoom,_maxZoom;
    bool canDoValidTiles;
    Maply::SortedTileSet sortedTiles;
}

- (id)initWithSources:(NSArray *)tileSources
{
    self = [super init];
    
    if (!self || !tileSources || [tileSources count] == 0)
        return nil;
    
    _tileSources = tileSources;

    // Check the various info
    _minZoom = [tileSources[0] minZoom];
    _maxZoom = [tileSources[0] maxZoom];
    _coordSys = [tileSources[0] coordSys];
    canDoValidTiles = [tileSources[0] respondsToSelector:@selector(validTile:bbox:)];
    
    for (unsigned int ii=1;ii<[tileSources count];ii++)
    {
        NSObject<MaplyTileSource> *tileSource = tileSources[ii];
        if ([tileSource minZoom] != _minZoom || [tileSource maxZoom] != _maxZoom)
            return nil;
        // Note: Check the coordinate system
    }
    
    return self;
}

- (void)dealloc
{
    @synchronized(self)
    {
        for (Maply::SortedTileSet::iterator it = sortedTiles.begin();
             it != sortedTiles.end(); ++it)
        {
            Maply::SortedTile tile = *it;
            tile.cancel();
        }
        sortedTiles.clear();
    }
}

- (int)minZoom
{
    return _minZoom;
}

- (int)maxZoom
{
    return _maxZoom;
}

- (int)tileSize
{
    return [((NSObject<MaplyTileSource> *)_tileSources[0]) tileSize];
}

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    if (!canDoValidTiles)
        return true;
    
    // Just ask the first one
    if ([_tileSources count] > 0)
    {
        NSObject<MaplyTileSource> *tileSource = _tileSources[0];
        return [tileSource validTile:tileID bbox:bbox];
    }
    
    return true;
}

// It's local if all the tile sources say so
- (bool)tileIsLocal:(MaplyTileID)tileID
{
    bool tileLocal = true;
    for (NSObject<MaplyTileSource> *tileSource in _tileSources)
    {
        if ([tileSource respondsToSelector:@selector(tileIsLocal:)])
            tileLocal &= [tileSource tileIsLocal:tileID];
        else
            tileLocal = false;
        if (!tileLocal)
            break;
    }
    
    return tileLocal;
}

// Kill any outstanding fetches for a given tile
- (void)clearFetchesFor:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        Maply::SortedTileSet::iterator it = sortedTiles.find(Maply::SortedTile(tileID));
        if (it != sortedTiles.end())
        {
            Maply::SortedTile tile = *it;
            tile.cancel();
            sortedTiles.erase(it);
        }
    }
}

// If we're accepting errors for tiles, we do a last minute check of the data right here
- (NSError *)marshalDataArray:(NSMutableArray *)dataArray
{
    // We're looking for an NSNull or an NSError
    for (id obj in dataArray)
    {
        if ([obj isKindOfClass:[NSNull class]])
            return [[NSError alloc] initWithDomain:@"MaplyMultiplexTileSource" code:0 userInfo:nil];
        else if ([obj isKindOfClass:[NSError class]])
            return obj;
    }
    
    return nil;
}

// Got tile data back, figure out what to do with it
- (void)gotTile:(MaplyTileID)tileID which:(int)which data:(id)tileData layer:(MaplyQuadImageTilesLayer *)layer
{
//    NSLog(@"Got tile: %d: (%d,%d), %d",tileID.level,tileID.x,tileID.y,which);
    
    // Look for it in the bit list
    bool done = true;
    Maply::SortedTile theTile(tileID);
    @synchronized(self)
    {
        Maply::SortedTileSet::iterator it;
        it = sortedTiles.find(Maply::SortedTile(tileID));
        if (it == sortedTiles.end())
            // That's weird.  Just punt
            return;
        theTile = *it;
        sortedTiles.erase(it);

        // Add the tile data in and see if we're done
        theTile.tileData[which] = (tileData ? tileData : [NSNull null]);
        for (unsigned int ii=0;ii<theTile.tileData.size();ii++)
            if (theTile.tileData[ii] == nil)
            {
                done = false;
                break;
            }

        // If we're not, put the tile back in the set
        if (!done)
            sortedTiles.insert(theTile);
    }
    
    // Let's write it back out for the cache
    MaplyRemoteTileInfo *tileSource = _tileSources[which];
    NSString *fileName = [tileSource fileNameForTile:tileID];
    if (fileName && [tileData isKindOfClass:[NSData class]])
    {
        NSData *imgData = tileData;
        [imgData writeToFile:fileName atomically:YES];
    }

    // We're done, so let everyone know
    if (done)
    {
//        NSLog(@"Finished load for tile %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
        
        // Let the paging layer know about it
        NSMutableArray *allData = [NSMutableArray array];
        for (unsigned int ii=0;ii<theTile.tileData.size();ii++)
            [allData addObject:theTile.tileData[ii]];
 
        NSError *marshalError = [self marshalDataArray:allData];
        if (!marshalError)
        {
            // Let the delegate know we loaded successfully
            if (_delegate && [_delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
                [_delegate remoteTileSource:self tileDidLoad:tileID];
        
            [layer loadedImages:allData forTile:tileID];
        } else {
            // Last minute failure!
            [layer loadError:marshalError forTile:tileID];
            if (_delegate && [_delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                [_delegate remoteTileSource:self tileDidNotLoad:tileID error:marshalError];
        }
    }
}

// Got an error while trying to fetch tile
- (void)failedToGetTile:(MaplyTileID)tileID error:(NSError *)error layer:(MaplyQuadImageTilesLayer *)layer
{
    NSLog(@"Failed load for tile %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
    
    [self clearFetchesFor:tileID];
    
    // Unsucessful load
    [layer loadError:error forTile:tileID];
    if (_delegate && [_delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
        [_delegate remoteTileSource:self tileDidNotLoad:tileID error:error];
}

- (void)startFetchLayer:(id)layer tile:(MaplyTileID)tileID
{
//    NSLog(@"Starting fetch for tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
    
    // Clear out any existing state and add clean state
    [self clearFetchesFor:tileID];
    Maply::SortedTile newTile(tileID,(int)[_tileSources count]);
    // Don't think about this one too hard.
    std::vector<void (^)()> workBlocks;
    
    // Work through the sources, kicking off a request as we go
    int which = 0;
    for (MaplyRemoteTileInfo *tileSource in _tileSources)
    {
        // If it's local, just go fetch it
        if ([tileSource tileIsLocal:tileID])
        {
            // We'll save the block for later and run at the end
            void (^workBlock)() =
            ^{
                NSString *fileName = [tileSource fileNameForTile:tileID];
                NSData *imgData = [NSData dataWithContentsOfFile:fileName];
                
                if (!imgData)
                    [self failedToGetTile:tileID error:nil layer:layer];
                else
                    [self gotTile:tileID which:which data:imgData layer:layer];
            };
            workBlocks.push_back(workBlock);
        } else {
            // It's not local, so we need to kick off a request
            NSURLRequest *urlReq = [tileSource requestForTile:tileID];
            
            // Kick of an async request for the data
            MaplyMultiplexTileSource __weak *weakSelf = self;
            AFHTTPRequestOperation *op = [[AFHTTPRequestOperation alloc] initWithRequest:urlReq];
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            op.successCallbackQueue = queue;
            op.failureCallbackQueue = queue;
            [op setCompletionBlockWithSuccess:
             ^(AFHTTPRequestOperation *operation, id responseObject)
             {
                 if (weakSelf)
                     [self gotTile:tileID which:which data:responseObject layer:layer];
             }
                                      failure:
             ^(AFHTTPRequestOperation *operation, NSError *error)
             {
                 if (weakSelf)
                 {
                     if (weakSelf.acceptFailures)
                         [self gotTile:tileID which:which data:error layer:layer];
                     else
                         [self failedToGetTile:tileID error:error layer:layer];
                 }
             }];
            
            newTile.fetches.insert(Maply::TileFetch(which,op));
        }
        which++;
    }

    // Kick off the operations after we have the tile data in our set.
    // Otherwise these can come back before we've defined all of this
    @synchronized(self)
    {
        sortedTiles.insert(newTile);
        // Kick off the fetch operations
        for (Maply::TileFetchSet::iterator it = newTile.fetches.begin();
             it != newTile.fetches.end(); ++it)
            [it->op start];
    }
    // Run the work blocks that fetch local data
    for (unsigned int ii=0;ii<workBlocks.size();ii++)
        workBlocks[ii]();
}

@end
