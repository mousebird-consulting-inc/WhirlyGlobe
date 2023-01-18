/*
 *  QuadDisplayLayerNew.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/26/18.
 *  Copyright 2011-2022 mousebird consulting
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
#import "MaplyRenderController_private.h"

using namespace WhirlyKit;

@implementation WhirlyKitQuadDisplayLayerNew
{
    QuadDisplayControllerNewRef controller;
    __weak MaplyRenderController* renderControl;
}

- (nonnull)initWithController:(QuadDisplayControllerNewRef)inController
                renderControl:(MaplyRenderController*)inRenderControl
{
    if ((self = [super init]))
    {
        controller = inController;
        renderControl = inRenderControl;
    }
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

    // Update on any view movement, but not on re-applying the same position.
    // Note that this parameter is a double, but the value is later stored as a float.
    constexpr double minDist = std::numeric_limits<float>::min();

    // We want view updates, but only every so often
    [inLayerThread.viewWatcher addWatcherTarget:self
                                       selector:@selector(viewUpdate:)
                                        minTime:controller->getViewUpdatePeriod()
                                        minDist:minDist
                                     maxLagTime:10.0];

    controller->start();
}

- (void)teardown
{
    const auto lt = _layerThread;
    [lt.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];

    ChangeSet changes;
    controller->stop(NULL,changes);
    controller = NULL;

    [lt addChangeRequests:changes];
    [lt flushChangeRequests];
}

static const float DelayPeriod = 0.1;

// Called after some period to sweep up removes
- (void)delayCheck
{
    if (!controller || !controller->getViewState())
        return;

    __strong MaplyRenderController *rc = renderControl;
    try
    {
        [self viewUpdate:[[WhirlyKitViewStateWrapper alloc] initWithViewState:controller->getViewState()]];
    }
    catch (const std::exception &ex)
    {
        NSLog(@"Exception in QuadDisplayLayerNew.delayCheck: %s", ex.what());
        [rc report:@"QuadDisplayLayerNew-DelayCheck"
         exception:[[NSException alloc] initWithName:@"STL Exception"
                                              reason:[NSString stringWithUTF8String:ex.what()]
                                            userInfo:nil]];
    }
    catch (NSException *ex)
    {
        NSLog(@"Exception in QuadDisplayLayerNew.delayCheck: %@", ex.description);
        [rc report:@"QuadDisplayLayerNew-DelayCheck" exception:ex];
    }
    catch (...)
    {
        NSLog(@"Exception in QuadDisplayLayerNew.delayCheck");
        [rc report:@"QuadDisplayLayerNew-DelayCheck"
         exception:[[NSException alloc] initWithName:@"C++ Exception"
                                              reason:@"Unknown"
                                            userInfo:nil]];
    }
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
    if (!controller)
        return;
    controller->preSceneFlush(changes);
    
    [_layerThread addChangeRequests:changes];
}

@end
