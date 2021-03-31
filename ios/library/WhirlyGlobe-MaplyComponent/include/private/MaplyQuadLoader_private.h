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
    // Used to throttle the number of tiles being parsed at once
    dispatch_queue_t serialQueue;
    dispatch_semaphore_t serialSemaphore;
    
    int minLevel,maxLevel;
    
    WhirlyKit::SamplingParams params;
    MaplyQuadSamplingLayer *samplingLayer;
    
    WhirlyKit::QuadImageFrameLoader_iosRef loader;
    
    WhirlyKit::QuadImageFrameLoader::FrameStats frameStats;
}

- (instancetype __nonnull)initWithViewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)inViewC;

// The subclasses return their own
- (MaplyLoaderReturn * __nullable)makeLoaderReturn;

// We delay setup by a tick so the user can mess with settings
- (bool)delayedInit;

// Change the tile source to a new one (if they match)
- (void)changeTileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> * __nullable)tileInfos;

// Force a reload of all visible tiles
- (void)reload;

// Force a reload of all tiles in a bounding box
- (void)reloadArea:(MaplyBoundingBox)bounds;

// Force a reload of all tiles in a bounding box
- (void)reloadAreas:(NSArray<NSValue*>* __nullable)bounds;

// Called on the layer thread.  Recalculates loading priorites and sends those to the fetcher.
- (void)updatePriorities;

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
