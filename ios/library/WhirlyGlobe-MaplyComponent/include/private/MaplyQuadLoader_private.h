/*
 *  MaplyQuadImageLoader_private.h
 *
 *  Created by Steve Gifford on 2/12/19.
 *  Copyright 2012-2019 mousebird consulting inc
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

#import "loading/MaplyQuadLoader.h"
#import "QuadTileBuilder.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyQuadLoader_private.h"
#import "QuadDisplayLayerNew.h"
#import "QuadLoaderReturn.h"
#import "QuadImageFrameLoader_iOS.h"

@interface MaplyQuadLoaderBase()<QuadImageFrameLoaderLayer>
{
@public
    bool valid;

    NSObject<MaplyTileFetcher> *tileFetcher;
    NSObject<MaplyLoaderInterpreter> *loadInterp;
    
    int minLevel,maxLevel;
    
    WhirlyKit::SamplingParams params;
    MaplyQuadSamplingLayer *samplingLayer;
    
    WhirlyKit::QuadImageFrameLoader_iosRef loader;
    
    WhirlyKit::QuadImageFrameLoader::FrameStats frameStats;
}

- (instancetype)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)inViewC;

// The subclasses return their own
- (MaplyLoaderReturn *)makeLoaderReturn;

// We delay setup by a tick so the user can mess with settings
- (bool)delayedInit;

// Change the tile source to a new one (if they match)
- (void)changeTileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *)tileInfos;

// Force a reload of all visible tiles
- (void)reload;

@end

@interface MaplyLoaderReturn()
{
@public
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    
    // We're just wrapping the object that does the work
    WhirlyKit::QuadLoaderReturnRef loadReturn;

    // Unparsed data coming back
    std::vector<id> tileData;
}

@end
