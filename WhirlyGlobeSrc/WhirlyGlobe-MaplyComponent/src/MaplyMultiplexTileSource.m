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

#import "MaplyMultiplexTileSource.h"

@implementation MaplyMultiplexTileSource
{
    NSArray *_tileSources;
    int _minZoom,_maxZoom;
    bool canDoValidTiles;
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

- (MaplyImageTile *)imageForTile:(MaplyTileID)tileID
{
    // Hit up each source
    NSMutableArray * __block tileDataArray = [NSMutableArray array];
    for (unsigned int ii=0;ii<[_tileSources count];ii++)
        [tileDataArray addObject:[NSNull null]];
    
    int which = 0;
    int __block numRemaining = [_tileSources count];
    NSError __block *fetchError = nil;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    // Dispatch each one in parallel
    for (NSObject<MaplyTileSource> *tileSource in _tileSources)
    {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
        ^{
            id tile = [tileSource imageForTile:tileID];
            if ([tile isKindOfClass:[NSError class]])
            {
                fetchError = tile;
                tile = nil;
            }
            if (tile)
            {
                @synchronized(tileDataArray)
                {
                    tileDataArray[which] = tile;
                }
            }
            
            numRemaining--;
            dispatch_semaphore_signal(semaphore);
        });
        which++;
    }

    while (numRemaining > 0)
        dispatch_semaphore_wait(semaphore,DISPATCH_TIME_FOREVER);
    
    if (fetchError)
    {
        if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
            [_delegate remoteTileSource:self tileDidNotLoad:tileID error:fetchError];
        
        return nil;
    } else {
        if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
            [_delegate remoteTileSource:self tileDidLoad:tileID];
    }

    // Make sure we got them all
    for (id tile in tileDataArray)
        if ([tile isKindOfClass:[NSNull class]])
            return nil;

//    NSLog(@"Multiplex source: Loaded tile %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
    
    return [[MaplyImageTile alloc] initWithRandomData:tileDataArray];
}

@end
