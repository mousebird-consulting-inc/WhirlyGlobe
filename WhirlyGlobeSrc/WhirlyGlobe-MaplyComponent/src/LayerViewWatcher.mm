/*
 *  LayerViewWatcher.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/23/14.
 *  Copyright 2011-2015 mousebird consulting
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

#import "LayerViewWatcher_private.h"
#import "LayerThread_private.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
// Bridge the gap between the C++ view watcher callback and the Obj-C WhirlyKitLayerViewWatcher
class ViewWatcherAdapter: public ViewWatcher
{
public:
    ViewWatcherAdapter() { }
    ViewWatcherAdapter(View *view,WhirlyKitLayerViewWatcher *layerViewWatcher)
    : view(view), layerViewWatcher(layerViewWatcher)
    {
    }
    virtual void viewUpdated(View *view)
    {
        [layerViewWatcher viewUpdated:view];
    }
    
    View *view;
    WhirlyKitLayerViewWatcher __weak *layerViewWatcher;
};
}

// Keep track of what our watchers are up to
@interface LocalWatcher : NSObject
{
@public
    NSObject<WhirlyKitLayer> *target;
    TimeInterval minTime,maxLagTime;
    Point3d lastEyePos;
    float minDist;
    TimeInterval lastUpdated;
}
@end

@implementation LocalWatcher
@end

@implementation WhirlyKitLayerViewWatcher
{
    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    /// The view we're following for upates
    WhirlyKit::View *view;
    /// Watchers we'll call back for updates
    NSMutableArray *watchers;

    /// When the last update was run
    TimeInterval lastUpdate;

    /// You should know the type here.  A globe or a map view state.
    WhirlyKit::ViewState *lastViewState;

    WhirlyKit::ViewState *newViewState;
    bool kickoffScheduled;
    bool sweepLaggardsScheduled;
}

- (id)initWithView:(WhirlyKit::View *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super init];
    if (self)
    {
        layerThread = inLayerThread;
        view = inView;
        watchers = [NSMutableArray array];
    }

    return self;
}

- (void)dealloc
{
    [self setLastViewState:NULL];
    [self setNewViewState:NULL];
}

- (void)setLastViewState:(ViewState *)theViewState
{
    if (lastViewState)
        delete lastViewState;
    lastViewState = theViewState;
}

- (void)setNewViewState:(ViewState *)theViewState
{
    if (newViewState)
        delete newViewState;
    newViewState = theViewState;
}

- (void)addWatcherTarget:(NSObject<WhirlyKitLayer> *)target minTime:(TimeInterval)minTime minDist:(float)minDist maxLagTime:(TimeInterval)maxLagTime
{
    LocalWatcher *watch = [[LocalWatcher alloc] init];
    watch->target = target;
    watch->minTime = minTime;
    watch->minDist = minDist;
    watch->maxLagTime = maxLagTime;
    [watchers addObject:watch];

    // Note: This is running in the layer thread, yet we're accessing the view.  Might be a problem.
    if (!lastViewState && layerThread.renderer->getFramebufferSize().x() != 0)
    {
        ViewState *viewState = _viewStateFactory->makeViewState(view,layerThread.renderer);
        [self setLastViewState:viewState];
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
    if ([NSThread currentThread] == layerThread)
        [self removeWatcherTargetLayer:toRemove];
    else
        [self performSelector:@selector(removeWatcherTargetLayer:) onThread:layerThread withObject:toRemove waitUntilDone:NO];
}

- (void)removeWatcherTargetLayer:(LocalWatcher *)toRemove
{
    LocalWatcher *found = nil;

    for (LocalWatcher *watch in watchers)
    {
        if (watch->target == toRemove->target)
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

// This is called in the main thread
- (void)viewUpdated:(WhirlyKit::View *)inView
{
    WhirlyKit::ViewState *viewState = self.viewStateFactory->makeViewState(inView,layerThread.renderer);

    // The view has to be valid first
    if (layerThread.renderer->getFramebufferSize().x() <= 0.0)
    {
        // Let's check back every so often
        // Note: Porting
//        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(viewUpdated:) object:inView];
//        [self performSelector:@selector(viewUpdated:) withObject:inView afterDelay:0.1];
        return;
    }

//    lastViewState = viewState;
    @synchronized(self)
    {
        [self setNewViewState:viewState];
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
        [self setLastViewState:newViewState];
        newViewState = NULL;
        kickoffScheduled = false;
    }
    [self viewUpdateLayerThread:lastViewState];
    lastUpdate = CFAbsoluteTimeGetCurrent();
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
    // Make sure the thing we're watching is still valid.
    // This can happen with dangling selectors
    if (![watchers containsObject:watch])
    {
//        NSLog(@"Whoa! Tried to call a watcher that's no longer there.");
        return;
    }

    if (lastViewState)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [watch->target viewUpdate:lastViewState];
#pragma clang diagnostic pop
        watch->lastUpdated = CFAbsoluteTimeGetCurrent();
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
- (void)viewUpdateLayerThread:(WhirlyKit::ViewState *)viewState
{
    TimeInterval curTime = CFAbsoluteTimeGetCurrent();

    // Look for anything that hasn't been updated in a while
    std::set<LayerPriorityOrder> orderedLayers;
    TimeInterval minNextUpdate = 100;
    TimeInterval maxLayerDelay = 0.0;
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

//    static int count = 0;
//    if (count++ % 20 == 0)
//        NSLog(@"Max layer delay = %f, %f, layerThread = %x",maxLayerDelay,minNextUpdate,(unsigned int)layerThread);

    // Update the layers by priority
    // Note: What happens if this takes a really long time?
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

    [self viewUpdateLayerThread:(WhirlyKit::ViewState *)lastViewState];
}

@end

@implementation WhirlyGlobeLayerViewWatcher
{
    WhirlyGlobe::GlobeViewStateFactory globeViewFactory;
    ViewWatcherAdapter adapter;
}

- (id)initWithView:(WhirlyGlobe::GlobeView *)inView thread:(WhirlyKitLayerThread *)inLayerThread
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        adapter.view = inView;
        adapter.layerViewWatcher = self;
        inView->addWatcher(&adapter);
        super.viewStateFactory = &globeViewFactory;
    }

    return self;
}

@end

@implementation MaplyLayerViewWatcher
{
    Maply::MapViewStateFactory mapViewStateFactory;
    ViewWatcherAdapter adapter;
}

- (id)initWithView:(Maply::MapView *)inView thread:(WhirlyKitLayerThread *)inLayerThread;
{
    self = [super initWithView:inView thread:inLayerThread];
    if (self)
    {
        adapter.view = inView;
        adapter.layerViewWatcher = self;
        inView->addWatcher(&adapter);
        super.viewStateFactory = &mapViewStateFactory;
    }

    return self;
}

@end
