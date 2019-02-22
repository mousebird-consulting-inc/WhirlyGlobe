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

#import "MaplyQuadImageFrameLoader.h"
#import "MaplyBaseViewController_private.h"
#import "MaplyShader_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyScreenLabel.h"
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
    startTime = TimeGetCurrent();
    _period = 10.0;
    _pauseLength = 0.0;
    numFrames = [loader getNumFrames];

    [viewC addActiveObject:self];

    return self;
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

    TimeInterval now = TimeGetCurrent();
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

@implementation MaplyQuadImageFrameLoader

- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)inParams tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)frameInfos viewC:(MaplyBaseViewController * __nonnull)inViewC
{
    if (!inParams.singleLevel) {
        NSLog(@"MaplyQuadImageFrameLoader only supports samplers with singleLevel set to true");
        return nil;
    }
    self = [super initWithViewC:inViewC];

    params = inParams;

    // Loader does all the work.  The Obj-C version is just a wrapper
    self->loader = QuadImageFrameLoader_iosRef(new QuadImageFrameLoader_ios(params->params,
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
    
    return self;
}

- (bool)delayedInit
{
    if (!loadInterp) {
        loadInterp = [[MaplyImageLoaderInterpreter alloc] init];
    }
    loader->layer = self;

    // Hook into the active updater to organize geometry for rendering
    self.viewC->renderControl->scene->addActiveModel(loader);

    if (![super delayedInit])
        return false;

    return true;
}

- (void)setCurrentImage:(double)where
{
    double curFrame = std::min(std::max(where,0.0),(double)([loader->frameInfos count]-1));
    loader->setCurFrame(curFrame);
}

- (int)getNumFrames
{
    return [loader->frameInfos count];
}

- (void)shutdown
{
    self->samplingLayer.layerThread.scene->removeActiveModel(loader);

    [super shutdown];
}

@end
