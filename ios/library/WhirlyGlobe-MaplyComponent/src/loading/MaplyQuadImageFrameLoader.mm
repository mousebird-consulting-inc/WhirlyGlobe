/*
 *  MaplyQuadImageFrameLoader.mm
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2012-2018 mousebird consulting inc
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

#import "loading/MaplyQuadImageFrameLoader.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyShader_private.h"
#import "MaplyRenderTarget_private.h"
#import "visual_objects/MaplyScreenLabel.h"
#import "MaplyQuadImageLoader_private.h"
#import "MaplyQuadLoader_private.h"
#import "MaplyImageTile_private.h"

using namespace WhirlyKit;

NSString * const MaplyQuadImageLoaderFetcherName = @"QuadImageLoader";

@implementation MaplyQuadImageFrameAnimator
{
    MaplyBaseViewController * __weak viewC;
    MaplyQuadImageFrameLoader * __weak loader;
    TimeInterval startTime;
    int numFrames;
}

- (instancetype)initWithFrameLoader:(MaplyQuadImageFrameLoader *)inLoader viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    self = [super init];
    loader = inLoader;
    viewC = inViewC;
    _pauseLength = 0.0;
    numFrames = [loader getNumFrames];
    self.period = 10.0;

    [viewC addActiveObject:self];

    return self;
}

- (void)setPeriod:(NSTimeInterval)period
{
    if (period == _period)
        return;
    
    _period = period;
    if (!scene)
        return;
    // Adjust the start time to make the quad loader appear not to move initially
    if (numFrames > 1) {
        startTime = scene->getCurrentTime()-[loader getCurrentImage]/(numFrames-1) * _period;
    } else if (numFrames > 0) {
        startTime = scene->getCurrentTime()-[loader getCurrentImage] * _period;
    }
}

- (void)shutdown
{
    [viewC removeActiveObject:self];
    loader = nil;
    viewC = nil;
}

// MARK: ActiveObject methods

// Have to do the position update in the setCurrentImage so we're not messing with the rendering loop
- (bool)hasUpdate
{
    if (!viewC || !loader)
        return false;

    TimeInterval now = scene->getCurrentTime();
    TimeInterval totalPeriod = _period + _pauseLength;
    double when = fmod(now-startTime,totalPeriod);
    if (when >= _period)
        // Snap it to the end for a while
        [loader setCurrentImage:numFrames-1];
    else {
        double where = when/_period * (numFrames-1);
        [loader setCurrentImage:where];
    }

    return false;
}

- (void)updateForFrame:(void *)frameInfo
{
}

- (void)teardown
{
    loader = nil;
}

@end

@implementation MaplyQuadImageFrameStats
@end

@implementation MaplyQuadImageFrameLoaderStats

@end

@implementation MaplyQuadImageFrameLoader
{
    bool started;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)inParams tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)frameInfos viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    if (!inParams.singleLevel) {
        NSLog(@"MaplyQuadImageFrameLoader only supports samplers with singleLevel set to true");
        return nil;
    }
    self = [super initWithViewC:inViewC];

    params = inParams->params;
    params.generateGeom = true;

    // Loader does all the work.  The Obj-C version is just a wrapper
    self->loader = QuadImageFrameLoader_iosRef(new QuadImageFrameLoader_ios(params,
                                                                            frameInfos,
                                                                            QuadImageFrameLoader::MultiFrame));
    
    self.baseDrawPriority = kMaplyImageLayerDrawPriorityDefault;
    self.drawPriorityPerLevel = 100;
    
    self.flipY = true;
    self.debugMode = false;
    self->minLevel = 10000;
    self->maxLevel = -1;
    for (MaplyRemoteTileInfoNew *frameInfo in frameInfos) {
        self->minLevel = std::min(self->minLevel,frameInfo.minZoom);
        self->maxLevel = std::max(self->maxLevel,frameInfo.maxZoom);
    }
    self->valid = true;
    started = false;
    
    return self;
}

- (void)addFocus
{
    if (started) {
        NSLog(@"MaplyQuadImageFrameLoader: addFocus called too late.");
        return;
    }
    
    loader->addFocus();
}

- (void)setLoadFrameMode:(MaplyLoadFrameMode)loadFrameMode
{
    if (!loader)
        return;
    
    _loadFrameMode = loadFrameMode;
    loader->setLoadMode(_loadFrameMode == MaplyLoadFrameBroad ? QuadImageFrameLoader::LoadMode::Broad : QuadImageFrameLoader::LoadMode::Narrow);
}

- (bool)delayedInit
{
    started = true;
    if (!loadInterp) {
        loadInterp = [[MaplyImageLoaderInterpreter alloc] init];
    }
    loader->layer = self;

    // Hook into the active updater to organize geometry for rendering
    [self.viewC getRenderControl]->scene->addActiveModel(loader);

    if (![super delayedInit])
        return false;

    return true;
}

- (int)getNumFocus
{
    if (!loader)
        return 0;
    
    return loader->getNumFocus();
}

- (MaplyLoaderReturn *)makeLoaderReturn
{
    return [[MaplyImageLoaderReturn alloc] initWithLoader:self];
}

- (void)setCurrentImage:(double)where
{
    [self setFocus:0 currentImage:where];
}

- (void)setFocus:(int)focusID currentImage:(double)where
{
    double curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
    loader->setCurFrame(focusID,curFrame);
}

- (double)getCurrentImage
{
    return [self getCurrentImageForFocus:0];
}

- (double)getCurrentImageForFocus:(int)focusID
{
    if (!loader)
        return 0.0;
    
    return loader->getCurFrame(focusID);
}

- (void)setRequireTopTiles:(bool)newVal
{
    if (!loader)
        return;
    
    loader->setRequireTopTilesLoaded(newVal);
}

- (int)getNumFrames
{
    return [loader->frameInfos count];
}

- (MaplyQuadImageFrameLoaderStats * __nonnull)getFrameStats
{
    QuadImageFrameLoader::Stats stats = loader->getStats();
    
    MaplyQuadImageFrameLoaderStats *retStats = [[MaplyQuadImageFrameLoaderStats alloc] init];
    retStats.numTiles = stats.numTiles;
    NSMutableArray *frameStats = [[NSMutableArray alloc] init];
    for (auto frameStat: stats.frameStats) {
        MaplyQuadImageFrameStats *retFrameStat = [[MaplyQuadImageFrameStats alloc] init];
        retFrameStat.totalTiles = frameStat.totalTiles;
        retFrameStat.tilesToLoad = frameStat.tilesToLoad;
        [frameStats addObject:retFrameStat];
    }
    retStats.frames = frameStats;
    
    return retStats;
}

- (void)setFocus:(int)focusID renderTarget:(MaplyRenderTarget *)renderTarget
{
    if (!loader)
        return;
    
    loader->setRenderTarget(focusID, renderTarget.renderTargetID);
}

- (void)setFocus:(int)focusID shader:(MaplyShader * __nullable)shader
{
    if (!loader)
        return;
    
    loader->setShaderID(focusID,[shader getShaderID]);
}

- (void)shutdown
{
    self->samplingLayer.layerThread.scene->removeActiveModel(loader);

    [super shutdown];
}

@end
