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
    NSMutableArray *tileDataArray = [NSMutableArray array];
    for (NSObject<MaplyTileSource> *tileSource in _tileSources)
    {
        id tile = [tileSource imageForTile:tileID];
        if (!tile)
            return nil;
        [tileDataArray addObject:tile];
    }
    
    return [[MaplyImageTile alloc] initWithRandomData:tileDataArray];
}

@end
