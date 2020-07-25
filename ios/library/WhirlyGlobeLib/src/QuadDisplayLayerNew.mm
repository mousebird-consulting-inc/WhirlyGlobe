/*
 *  QuadDisplayLayerNew.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
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

#import "QuadDisplayLayerNew.h"
#import "LayerThread.h"

using namespace WhirlyKit;

@implementation WhirlyKitQuadDisplayLayerNew
{
    QuadDisplayControllerNewRef controller;
}

- (nonnull)initWithController:(QuadDisplayControllerNewRef)inController
{
    self = [super init];
    controller = inController;

    return self;
}

- (WhirlyKit::QuadDisplayControllerNewRef)getController
{
    return controller;
}

- (void)dealloc
{
    controller = NULL;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    _layerThread = inLayerThread;
    
    // We want view updates, but only every so often
    if (_layerThread.viewWatcher)
        [_layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:controller->getViewUpdatePeriod() minDist:0.0 maxLagTime:10.0];
    
    controller->start();
}

- (void)teardown
{    
    ChangeSet changes;
    
    if (_layerThread.viewWatcher)
        [_layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];

    controller->stop(NULL,changes);
    controller = NULL;
    
    [_layerThread addChangeRequests:changes];
}

static const float DelayPeriod = 0.1;

// Called after some period to sweep up removes
- (void)delayCheck
{
    if (!controller || !controller->getViewState())
        return;

    [self viewUpdate:[[WhirlyKitViewStateWrapper alloc] initWithViewState:controller->getViewState()]];
}

// Called periodically when the user moves, but not too often
- (void)viewUpdate:(WhirlyKitViewStateWrapper *)inViewState
{
    if (!controller)
        return;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(delayCheck) object:nil];

    ChangeSet changes;
    // Controller is sitting on some unloads, so call it back in a bit
    // Note: We don't need to go through the view update logic here.  We should just process the unloads.
    if (controller->viewUpdate(NULL,inViewState.viewState,changes)) {
        [self performSelector:@selector(delayCheck) withObject:nil afterDelay:DelayPeriod];
    }
    
    [_layerThread addChangeRequests:changes];
}

- (void)preSceneFlush:(WhirlyKitLayerThread *)layerThread
{
    ChangeSet changes;
    controller->preSceneFlush(changes);
    
    [_layerThread addChangeRequests:changes];
}

@end
