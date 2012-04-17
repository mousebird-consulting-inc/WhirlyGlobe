/*
 *  GlobeLayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011 mousebird consulting
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

#import "GlobeLayerViewWatcher.h"
#import "LayerThread.h"

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    id target;
    SEL selector;
    NSTimeInterval minTime;
    NSTimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@implementation WhirlyGlobeViewState

- (id)initWithView:(WhirlyGlobeView *)globeView
{
    self = [super init];
    if (self)
    {
        heightAboveGlobe = globeView.heightAboveGlobe;
        rotQuat = globeView.rotQuat;
        modelMatrix = [globeView calcModelMatrix];
    }
    
    return self;
}

@end


@implementation WhirlyGlobeLayerViewWatcher

- (id)initWithView:(WhirlyGlobeView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        inView.watchDelegate = self;
    }
    
    return self;
}

- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime
{
    LocalWatcher *watch = [[LocalWatcher alloc] init];
    watch->target = target;
    watch->selector = selector;
    watch->minTime = minTime;
    [watchers addObject:watch];
    
    if (!lastViewState)
    {
        WhirlyGlobeViewState *viewState = [[WhirlyGlobeViewState alloc] initWithView:(WhirlyGlobeView *)view];
        lastViewState = viewState;        
    }
    
    // Make sure it gets a starting update
    [self performSelector:@selector(updateSingleWatcher:) onThread:layerThread withObject:watch waitUntilDone:NO];
}

- (void)removeWatcherTarget:(id)target selector:(SEL)selector
{
    LocalWatcher *found = nil;
    
    for (LocalWatcher *watch in watchers)
    {
        if (watch->target == target && watch->selector == selector)
        {
            found = watch;
            break;
        }
    }
    
    if (found)
    {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateSingleWatcher:) object:found];
        [watchers removeObject:found];
    }
}

// This is called in the main thread
- (void)viewUpdated:(WhirlyGlobeView *)inGlobeView
{
    WhirlyGlobeViewState *viewState = [[WhirlyGlobeViewState alloc] initWithView:inGlobeView];
    lastViewState = viewState;
    [self performSelector:@selector(viewUpdateLayerThread:) onThread:layerThread withObject:viewState waitUntilDone:NO];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(sweepLaggards:) object:nil];
    lastUpdate = [[NSDate date] timeIntervalSinceReferenceDate];
}

// Let the watcher know about an update
// Called in the layer thread
- (void)updateSingleWatcher:(LocalWatcher *)watch
{
    [watch->target performSelector:watch->selector onThread:layerThread withObject:lastViewState waitUntilDone:NO];
    watch->lastUpdated = lastUpdate;
}

// This version is called in the layer thread
// We can dispatch things from here
- (void)viewUpdateLayerThread:(WhirlyGlobeViewState *)viewState
{
    NSTimeInterval curTime = [[NSDate date] timeIntervalSinceReferenceDate];
    
    // Look for anything that hasn't been updated in a while
    float minNextUpdate = 100;
    for (LocalWatcher *watch in watchers)
    {
        NSTimeInterval minTest = curTime - watch->lastUpdated;
        if (minTest > watch->minTime)
        {
            [self updateSingleWatcher:watch];
        } else {
            minNextUpdate = MIN(minNextUpdate,minTest);
        }
    }
    
    // Sweep up the laggard watchers
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(sweepLaggards:) object:nil];
    if (minNextUpdate < 100.0)
    {
        [self performSelector:@selector(sweepLaggards:) withObject:nil afterDelay:minNextUpdate];
    }
}

// Minimum update times are there to keep the layers from getting inundated
// That does mean they might get old information when the view stops moving
// So we call this at the end of a given update pass to sweep up the remains
- (void)sweepLaggards:(id)sender
{
    [self viewUpdateLayerThread:(WhirlyGlobeViewState *)lastViewState];
}

@end
