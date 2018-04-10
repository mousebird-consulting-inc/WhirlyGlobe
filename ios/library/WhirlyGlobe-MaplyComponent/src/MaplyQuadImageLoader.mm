/*
 *  MaplyQuadImageLoader.mm
 *
 *  Created by Steve Gifford on 4/10/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "MaplyQuadImageLoader_private.h"
#import "QuadTileBuilder.h"

@implementation MaplyQuadImageLoader
{
    NSObject<MaplyTileSource> *tileSource;
    WhirlyKitQuadTileBuilder *builder;
}

- (instancetype)initWithTileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    tileSource = inTileSource;
    if (![tileSource respondsToSelector:@selector(startFetchLayer:tile:frame:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement startFetchLayer:tile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(cancelTile:frame:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement cancelTile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(clear)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement cancelTile:frame:");
        return nil;
    }
    if (![tileSource respondsToSelector:@selector(validTile:bbox:)]) {
        NSLog(@"MaplyQuadImageLoader requires tile source implement validTile:bbox:");
        return nil;
    }
    
    _numSimultaneousFetches = 16;

    self = [super init];
    return self;
}

// MARK: Quad Build Delegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)inBuilder
{
    builder = inBuilder;
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles
{
    
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles
{
    
}

// Called by the tile source

- (void)loadedReturn:(MaplyQuadImageLoaderReturn *)loadReturn error:(NSError *)error
{
    
}

@end
