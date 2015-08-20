/*
 *  MaplyQuadDisplayLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/22/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyQuadDisplayLayer_private.h"
#import "LayerThread_private.h"

using namespace WhirlyKit;

namespace WhirlyKit
{

// Adapter between this and the display controller
// The loader calls the QuadDisplayController directly for a few things.
class QuadLayerAdapter : public QuadDisplayControllerAdapter
{
public:
    // Called right after a tile loaded
    virtual void adapterTileDidLoad(const Quadtree::Identifier &tileIdent)
    {
        // Make sure we actually evaluate them
        [NSObject cancelPreviousPerformRequestsWithTarget:quadLayer selector:@selector(evalStep:) object:nil];
        [quadLayer performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    }
    // Called right after a tile unloaded
    virtual void adapterTileDidNotLoad(const Quadtree::Identifier &tileIdent)
    {
        // Might get stuck here
        [NSObject cancelPreviousPerformRequestsWithTarget:quadLayer selector:@selector(evalStep:) object:nil];
        [quadLayer performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    }
    // Wake the thread up
    virtual void adapterWakeUp()
    {
        // Might get stuck here
        [NSObject cancelPreviousPerformRequestsWithTarget:quadLayer selector:@selector(evalStep:) object:nil];
        [quadLayer performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
    }
    
    WhirlyKitQuadDisplayLayer *quadLayer;
};
    
}

@implementation WhirlyKitQuadDisplayLayer
{
    SceneRendererES *renderer;
    QuadLayerAdapter adapter;
    TimeInterval frameStart,frameInterval;
}

- (id)initWithDataSource:(WhirlyKit::QuadDataStructure *)dataSource loader:(WhirlyKit::QuadLoader *)loader renderer:(WhirlyKit::SceneRendererES *)inRenderer
{
    self = [super init];
    if (!self)
        return nil;

    renderer = inRenderer;
    _displayControl = new QuadDisplayController(dataSource,loader,&adapter);
    // Note: Porting
    _displayControl->setMeteredMode(false);
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (_displayControl)
        delete _displayControl;
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)scene
{
    _layerThread = inLayerThread;
    
    _displayControl->init(scene,renderer);
    
    // We want view updates, but only 1s in frequency
    if (_layerThread.viewWatcher)
        [_layerThread.viewWatcher addWatcherTarget:self minTime:_displayControl->getViewUpdatePeriod() minDist:_displayControl->getMinUpdateDist() maxLagTime:10.0];
    
    if (_displayControl->getMeteredMode())
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameStart:) name:kWKFrameMessage object:nil];
}

// Information about what's currently loaded in
// Less detail than dumpInfo (which was for debugging)
- (void)log
{
//    if ([_loader respondsToSelector:@selector(log)])
//        [_loader log];
}

- (void)shutdown
{
    ChangeSet changes;
    _displayControl->shutdown(changes);
    [_layerThread addChangeRequests:changes];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (_layerThread.viewWatcher) {
        [(WhirlyGlobeLayerViewWatcher *)_layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
    }
}

// How much of the frame time we're willing to spend
static const TimeInterval AvailableFrame = 4.0/5.0;

// Called by the renderer (in that thread, so be careful)
- (void)frameStart:(NSNotification *)note
{
    WhirlyKitFrameMessage *msg = note.object;
    
    // If it's not coming from our renderer, we can ignore it
    if ((SceneRendererES *)msg.renderer != renderer)
        return;
    
    frameStart = msg.frameStart;
    frameInterval = msg.frameInterval;
    
    if (_displayControl->getMeteredMode())
        [self performSelector:@selector(frameStartThread) onThread:_layerThread withObject:nil waitUntilDone:NO];
}

- (void)frameStartThread
{
    TimeInterval howLong = CFAbsoluteTimeGetCurrent()-frameStart+AvailableFrame*frameInterval;
    if (howLong > 0.0)
    {
        ChangeSet changes;
        _displayControl->getLoader()->startUpdates(changes);
        [_layerThread addChangeRequests:changes];
        [self performSelector:@selector(frameEndThread) withObject:nil afterDelay:howLong];
    }
}

- (void)frameEndThread
{
    if (!_displayControl->getSomethingHappened())
        return;
    
    ChangeSet changes;
    _displayControl->frameEnd(changes);
    [_layerThread addChangeRequests:changes];
}

// Called every so often by the view watcher
// It's here that we evaluate what to load
- (void)viewUpdate:(WhirlyKit::ViewState *)inViewState
{
    if (!_displayControl)
    {
        NSLog(@"GlobeQuadDisplayLayer: Called viewUpdate: after being shutdown.");
        return;
    }
    
    // Just put ourselves on hold for a while
    if (!inViewState)
        return;
    
    _displayControl->viewUpdate(inViewState);
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Run the evaluation step for outstanding nodes
- (void)evalStep:(id)Sender
{
    // If the renderer hasn't been set up, punt and try again later
    Point2f frameSize = renderer->getFramebufferSize();
    if (frameSize.x() == 0 || frameSize.y() == 0 || _displayControl->getFirstUpdate())
    {
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.1];
        return;
    }
    
    ChangeSet changes;
    bool didSomething = _displayControl->evalStep(frameStart,frameInterval,AvailableFrame,changes);
    [_layerThread addChangeRequests:changes];
    
    if (didSomething)
        [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

- (void)refresh
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(refresh) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    // We're still dealing with the last one
    if (_displayControl->getWaitForLocalLoads())
        return;
    
    // Clean out anything we might be currently evaluating
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    
    ChangeSet changes;
    _displayControl->refresh(changes);
    [_layerThread addChangeRequests:changes];
    
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

- (void)wakeUp
{
    if ([NSThread currentThread] != _layerThread)
    {
        [self performSelector:@selector(wakeUp) onThread:_layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    _displayControl->wakeUp();
    
    // Note: Might be better to check if an eval is scheduled, rather than cancel it
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(evalStep:) object:nil];
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}


@end
