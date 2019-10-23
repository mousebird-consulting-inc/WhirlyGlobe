/*
 *  LayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
 *  Copyright 2011-2019 mousebird consulting
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

#import "LayerViewWatcher.h"
#import "LayerThread.h"
#import "SceneRenderer.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitViewStateWrapper

- (id)initWithViewState:(ViewStateRef)viewState
{
    self = [super init];
    _viewState = viewState;
    
    return self;
}

@end

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    id __weak target;
    SEL selector;
    TimeInterval minTime,maxLagTime;
    Point3d lastEyePos;
    float minDist;
    TimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@interface WhirlyKitLayerViewWatcher()
- (void)viewUpdated:(View *)inView;
@end

namespace WhirlyKit {

// Interface with C++ View
class ViewWatcherWrapper : public ViewWatcher
{
public:
    WhirlyKitLayerViewWatcher * __weak viewWatcher;
    
    // View has been updated so we'll just hand that over to the watcher
    virtual void viewUpdated(View *view)
    {
        [viewWatcher viewUpdated:view];
    }
};

}

@implementation WhirlyKitLayerViewWatcher
{
    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    View *view;
    /// Watchers we'll call back for updates
    NSMutableArray *watchers;
    
    /// When the last update was run
    TimeInterval lastUpdate;
    
    /// You should know the type here.  A globe or a map view state.
    ViewStateRef lastViewState;
    
    ViewStateRef newViewState;
    bool kickoffScheduled;
    bool sweepLaggardsScheduled;
    
    ViewWatcherWrapper viewWatchWrapper;
}

- (id)initWithView:(View *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super init];
    if (self)
    {
        layerThread = inLayerThread;
        view = inView;
        watchers = [NSMutableArray array];
        lastViewState = inView->makeViewState(layerThread.renderer);
        viewWatchWrapper.viewWatcher = self;
        inView->addWatcher(&viewWatchWrapper);
    }
    
    return self;
}

- (void)stop
{
    view->removeWatcher(&viewWatchWrapper);
}

- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(TimeInterval)minTime minDist:(double)minDist maxLagTime:(TimeInterval)maxLagTime
{
    LocalWatcher *watch = [[LocalWatcher alloc] init];
    watch->target = target;
    watch->selector = selector;
    watch->minTime = minTime;
    watch->minDist = minDist;
    watch->maxLagTime = maxLagTime;
    @synchronized(self)
    {
        [watchers addObject:watch];
    }
    
    if (!lastViewState && layerThread.renderer->framebufferWidth != 0)
    {
        lastViewState = view->makeViewState(layerThread.renderer);
    }
    
    // Make sure it gets a starting update
    // The trick here is we need to let the main thread finish setting up first
    if (lastViewState)
        [self performSelectorOnMainThread:@selector(updateSingleWatcherDelay:) withObject:watch waitUntilDone:NO];
}

- (void)removeWatcherTarget:(id)target selector:(SEL)selector
{
    // Call into the layer thread, just to be safe
    LocalWatcher *toRemove = [[LocalWatcher alloc] init];
    toRemove->target = target;
    toRemove->selector = selector;
    if ([NSThread currentThread] == layerThread)
        [self removeWatcherTargetLayer:toRemove];
    else
        [self performSelector:@selector(removeWatcherTargetLayer:) onThread:layerThread withObject:toRemove waitUntilDone:NO];
}

- (void)removeWatcherTargetLayer:(LocalWatcher *)toRemove
{
    LocalWatcher *found = nil;
    
    @synchronized(self)
    {
        for (LocalWatcher *watch in watchers)
        {
            if (watch->target == toRemove->target && watch->selector == toRemove->selector)
            {
                found = watch;
                break;
            }
        }
        
        if (found)
        {
            [watchers removeObject:found];
        }
    }
}

// This is called in the main thread
- (void)viewUpdated:(View *)inView
{
    if (layerThread.renderer == nil)
        return;
    
    // The view has to be valid first
    if (layerThread.renderer->framebufferWidth <= 0.0)
    {
        // Let's check back every so often
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(viewUpdated:) object:nil];
        [self performSelector:@selector(viewUpdated:) withObject:nil afterDelay:0.1];
        return;
    }

    ViewStateRef viewState = view->makeViewState(layerThread.renderer);
    
    //    lastViewState = viewState;
    @synchronized(self)
    {
        newViewState = viewState;
        if (!kickoffScheduled)
        {
            kickoffScheduled = true;
            [self performSelector:@selector(kickoffViewUpdated) onThread:layerThread withObject:nil waitUntilDone:NO];
        }
    }
}

// This is called in the layer thread
// We kick off the update here
- (void)kickoffViewUpdated
{
    @synchronized(self)
    {
        lastViewState = newViewState;
        kickoffScheduled = false;
    }
    [self viewUpdateLayerThread:lastViewState];
    lastUpdate = layerThread.scene->getCurrentTime();
}

// We're in the main thread here
// Now we can kick off the watcher delay on the layer thread
- (void)updateSingleWatcherDelay:(LocalWatcher *)watch
{
    [self performSelector:@selector(updateSingleWatcher:) onThread:layerThread withObject:watch waitUntilDone:NO];
}

// Let the watcher know about an update
// Called in the layer thread
- (void)updateSingleWatcher:(LocalWatcher *)watch
{
    @synchronized(self)
    {
        // Make sure the thing we're watching is still valid.
        // This can happen with dangling selectors
        if (![watchers containsObject:watch])
        {
            //        NSLog(@"Whoa! Tried to call a watcher that's no longer there.");
            return;
        }
    }
    
    if (lastViewState)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [watch->target performSelector:watch->selector withObject:[[WhirlyKitViewStateWrapper alloc] initWithViewState:lastViewState]];
#pragma clang diagnostic pop
        watch->lastUpdated = layerThread.scene->getCurrentTime();
        watch->lastEyePos = lastViewState->eyePos;
    } else
        NSLog(@"Missing last view state");
}

// Used to order updates
class LayerPriorityOrder
{
public:
    bool operator < (const LayerPriorityOrder &that) const { return sinceLastUpdate > that.sinceLastUpdate; }
    LayerPriorityOrder(TimeInterval sinceLastUpdate,LocalWatcher *watch) : sinceLastUpdate(sinceLastUpdate), watch(watch) { }
    LayerPriorityOrder(const LayerPriorityOrder &that) : sinceLastUpdate(that.sinceLastUpdate), watch(that.watch) { }
    TimeInterval sinceLastUpdate;
    LocalWatcher *watch;
};

// This version is called in the layer thread
// We can dispatch things from here
- (void)viewUpdateLayerThread:(ViewStateRef)viewState
{
    TimeInterval curTime = layerThread.scene->getCurrentTime();
    
    // Look for anything that hasn't been updated in a while
    std::set<LayerPriorityOrder> orderedLayers;
    TimeInterval minNextUpdate = 100;
    TimeInterval maxLayerDelay = 0.0;
    @synchronized(self)
    {
        for (LocalWatcher *watch in watchers)
        {
            TimeInterval minTest = curTime - watch->lastUpdated;
            if (minTest > watch->minTime)
            {
                bool runUpdate = false;
                
                // Check the distance, if that's set
                if (watch->minDist > 0.0)
                {
                    // If we haven't moved past the trigger, don't update this time
                    double thisDist2 = (viewState->eyePos - watch->lastEyePos).squaredNorm();
                    if (thisDist2 > watch->minDist*watch->minDist)
                        runUpdate = true;
                    else {
                        if (minTest > watch->maxLagTime)
                        {
                            runUpdate = true;
                            minNextUpdate = MIN(minNextUpdate,minTest);
                        }
                    }
                } else
                    runUpdate = true;
                
                if (runUpdate)
                    orderedLayers.insert(LayerPriorityOrder(minTest,watch));
            } else {
                minNextUpdate = MIN(minNextUpdate,minTest);
            }
            maxLayerDelay = MAX(maxLayerDelay,minTest);
        }
    }
    
    //    static int count = 0;
    //    if (count++ % 20 == 0)
    //        NSLog(@"Max layer delay = %f, %f, layerThread = %x",maxLayerDelay,minNextUpdate,(unsigned int)layerThread);
    
    // Update the layers by priority
    for (std::set<LayerPriorityOrder>::iterator it = orderedLayers.begin();
         it != orderedLayers.end(); ++it)
        [self updateSingleWatcher:it->watch];
    
    @synchronized(self)
    {
        if (!sweepLaggardsScheduled)
        {
            if (minNextUpdate < 100.0)
            {
                sweepLaggardsScheduled = true;
                [self performSelector:@selector(sweepLaggards:) withObject:nil afterDelay:minNextUpdate];
            }
        }
    }
}

// Minimum update times are there to keep the layers from getting inundated
// That does mean they might get old information when the view stops moving
// So we call this at the end of a given update pass to sweep up the remains
- (void)sweepLaggards:(id)sender
{
    @synchronized(self)
    {
        sweepLaggardsScheduled = false;
    }
    
    [self viewUpdateLayerThread:lastViewState];
}

@end
