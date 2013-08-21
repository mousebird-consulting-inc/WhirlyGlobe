/*
 *  LayoutLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/4/12.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import <map>
#import "LayoutLayer.h"
#import "GlobeLayerViewWatcher.h"
#import "ScreenSpaceGenerator.h"
#import "GlobeMath.h"

using namespace Eigen;
using namespace WhirlyKit;


namespace WhirlyKit
{
}

@implementation WhirlyKitLayoutLayer
{
    // Layer thread we're on
    WhirlyKitLayerThread * __weak layerThread;
    // Scene we're updating
    Scene *scene;
    // Set if we haven't moved for a while
    bool stopped;
    // Last view state we've seen
    WhirlyKitViewState *viewState;
    // Used for sizing info
    WhirlyKitSceneRendererES *renderer;
}

- (id)initWithRenderer:(WhirlyKitSceneRendererES *)inRenderer
{
    self = [super init];
    if (!self)
        return nil;
    renderer = inRenderer;
    _maxDisplayObjects = 0;
    
    return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    
    // Get us view updates, but we'll filter them
    if (layerThread.viewWatcher)
        [layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:0.0 minDist:0.0 maxLagTime:0.0];
    
    [self checkUpdate];
}

- (void)shutdown
{
    scene = NULL;
    if (layerThread.viewWatcher)
        [layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(checkUpdate) object:nil];
}

// How long we'll wait to see if the user has stopped twitching
static const float DelayPeriod = 0.1;

// We're getting called for absolutely every update here
- (void)viewUpdate:(WhirlyKitViewState *)inViewState
{
    if (!scene)
        return;
    
    if (viewState && [viewState isKindOfClass:[WhirlyKitViewState class]] && [inViewState isSameAs:viewState])
        return;
    viewState = inViewState;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];
    
    if (stopped)
    {
//        NSLog(@"Started moving");
//        [self disableObjects];
        stopped = false;
    }
    
    // Set a timer to see if we've stopped in a bit
    [self performSelector:@selector(delayCheck) withObject:nil afterDelay:DelayPeriod];
}

// We also need to check on updates outside of the layer thread
- (void)checkUpdate
{
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (viewState && layoutManager && layoutManager->hasChanges())
    {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];

        // Set a timer to see if we've stopped in a bit
        [self performSelector:@selector(delayCheck) withObject:nil afterDelay:DelayPeriod];
    }
    
    [self performSelector:@selector(checkUpdate) withObject:nil afterDelay:2*DelayPeriod];
}

// Called after some period to check if we've stopped moving
- (void)delayCheck
{
    if (!viewState)
        return;
    
    stopped = true;
//    NSLog(@"Stopped moving");
    
    [self updateLayout];
}

- (void)setMaxDisplayObjects:(int)maxDisplayObjects
{
    _maxDisplayObjects = maxDisplayObjects;
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setMaxDisplayObjects(_maxDisplayObjects);
    }
    
// Layout all the objects we're tracking
- (void)updateLayout
{
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
            {
        ChangeSet changes;
        layoutManager->updateLayout(viewState,changes);
        [layerThread addChangeRequests:changes];
        }
    }
    
- (void)addLayoutObjects:(const std::vector<WhirlyKit::LayoutObject> &)newObjects
{
    if ([NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitLayoutLayer: Called addLayoutObjects from outside the layer thread.  Ignoring data.");
        return;
    }
    
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->addLayoutObjects(newObjects);

    // Note: This is too often.  Need a better way of notifying need for an update.
    [self updateLayout];
}

- (void)removeLayoutObjects:(const SimpleIDSet &)objectIDs
{
    if ([NSThread currentThread] != layerThread)
    {
        NSLog(@"WhirlyKitLayoutLayer: Called removeLayoutObjects from outside the layer thread.  Ignoring data.");
        return;
    }
    
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->removeLayoutObjects(objectIDs);
    
    // Note: This is too often.  Need a better way of notifying need for an update.
    [self updateLayout];
}

@end
