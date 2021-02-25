/*
 *  LayerThread.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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

#import "LayerThread.h"
#import "Scene.h"
#import "GlobeView.h"
#import "Platform.h"
#import "SceneRendererMTL.h"

using namespace WhirlyKit;

@implementation WhirlyKitLayerThread
{
    const RenderSetupInfo *setupInfo;
    /// The various data layers we'll display
    NSMutableArray *layers;
    
    /// Used to keep track of things to delete
    std::vector<WhirlyKit::DelayedDeletable *> thingsToDelete;
    
    /// Used to keep track of things to release
    NSMutableArray *thingsToRelease;
    
    /// Threads to shut down
    NSMutableArray *threadsToShutdown;
    
    /// Change requests to merge soonish
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
    
    /// We can get change requests from other threads (!)
    std::mutex changeLock;
    
    NSCondition *pauseLock;
    BOOL paused;
    BOOL inRunAddChangeRequests;
}

- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(View *)inView renderer:(SceneRenderer *)inRenderer mainLayerThread:(bool)mainLayerThread
{
	if ((self = [super init]))
	{
        setupInfo = inRenderer->getRenderSetupInfo();
        _mainLayerThread = mainLayerThread;
		_scene = inScene;
        _renderer = inRenderer;
		layers = [NSMutableArray array];
        inRunAddChangeRequests = false;
        _viewWatcher = [[WhirlyKitLayerViewWatcher alloc] initWithView:inView thread:self];
        
        thingsToRelease = [NSMutableArray array];
        threadsToShutdown = [NSMutableArray array];
        
        _allowFlush = true;
        
        pauseLock = [[NSCondition alloc] init];
	}
	
	return self;
}

- (void)dealloc
{
}

- (void)addLayer:(NSObject<WhirlyKitLayer> *)layer
{
    if (self.runLoop)
        [self performSelector:@selector(addLayerThread:) onThread:self withObject:layer waitUntilDone:NO];
    else
        [layers addObject:layer];    
}

- (void)addLayerThread:(NSObject<WhirlyKitLayer> *)layer
{
	[layers addObject:layer];    
    [layer startWithThread:self scene:_scene];
}

- (void)removeLayer:(NSObject<WhirlyKitLayer> *)layer
{
    bool wasPaused = paused;
    if (paused)
        [self unpause];
    [self performSelector: @selector(removeLayerThread:) onThread: self withObject:layer waitUntilDone:YES];
    if (wasPaused)
        [self pause];
}

// This runs in the layer thread
- (void)removeLayerThread:(NSObject<WhirlyKitLayer> *)layer
{
    if ([layers containsObject:layer])
    {
        // If we're done, we won't bother shutting down things nicely
        if (![self isCancelled])
            [layer teardown];
        else {
            if ([layer respondsToSelector:@selector(systemTeardown)])
                [layer systemTeardown];
        }
        [layers removeObject:layer];
    }
}

- (void)addThingToDelete:(WhirlyKit::DelayedDeletable *)thing
{
    if (!thing)
        return;
    
    thingsToDelete.push_back(thing);
}

- (void)addThingToRelease:(NSObject *)thing
{
    if (!thing)
        return;
    
    [thingsToRelease addObject:thing];
}

- (void)addThreadToShutdown:(WhirlyKitLayerThread *)thread
{
    if (!thread)
        return;
    
    [threadsToShutdown addObject:thread];
}

- (void)addChangeRequest:(WhirlyKit::ChangeRequest *)changeRequest
{
    std::vector<WhirlyKit::ChangeRequest *> requests;
    requests.push_back(changeRequest);
    
    [self addChangeRequests:requests];
}

- (void)addChangeRequests:(std::vector<WhirlyKit::ChangeRequest *> &)newChangeRequests
{
    if (newChangeRequests.empty())
        return;
    
    std::lock_guard<std::mutex> guardLock(changeLock);

    // If we don't have one coming, schedule a merge
    if (!inRunAddChangeRequests && changeRequests.empty())
        [self performSelector:@selector(runAddChangeRequests) onThread:self withObject:nil waitUntilDone:NO];
    
    changeRequests.insert(changeRequests.end(), newChangeRequests.begin(), newChangeRequests.end());
}

- (void)flushChangeRequests
{
    if (inRunAddChangeRequests)
        return;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(runAddChangeRequests) object:nil];
    [self runAddChangeRequests];
}

- (void)requestFlush
{
    [self addChangeRequest:NULL];
}

- (void)runAddChangeRequests
{
    if ([self isCancelled])
        // Note: Hey, should we be deleting these?
        return;
    
    inRunAddChangeRequests = true;
    for (NSObject<WhirlyKitLayer> *layer in layers) {
        if ([layer respondsToSelector:@selector(preSceneFlush:)])
            [layer preSceneFlush:self];
    }
    inRunAddChangeRequests = false;
    
    std::vector<WhirlyKit::ChangeRequest *> changesToProcess;
    {
        std::lock_guard<std::mutex> guardLock(changeLock);
        changesToProcess = changeRequests;
        changeRequests.clear();
    }

    bool requiresFlush = false;
    // Set up anything that needs to be set up
    ChangeSet changesToAdd;
    for (unsigned int ii=0;ii<changesToProcess.size();ii++)
    {
        ChangeRequest *change = changesToProcess[ii];
        if (change)
        {
            requiresFlush |= change->needsFlush();
            change->setupForRenderer(_renderer->getRenderSetupInfo(),_scene);
            changesToAdd.push_back(changesToProcess[ii]);
        } else
            // A NULL change request is just a flush request
            requiresFlush = true;
    }
    
    // If anything needed a flush after that, let's do it
    if (requiresFlush && _allowFlush)
    {        
        // If there were no changes to add we probably still want to poke the scene
        // Otherwise texture changes don't show up
        if (changesToAdd.empty())
            changesToAdd.push_back(nullptr);
    }
    
    _scene->addChangeRequests(changesToAdd);
}

- (void)log
{
    if ([NSThread currentThread] != self)
    {
        [self performSelector:@selector(log) onThread:self withObject:nil waitUntilDone:NO];
        return;
    }
    
    for (NSObject<WhirlyKitLayer> *layer in layers)
        if ([layer respondsToSelector:@selector(log)])
            [layer log];
}

- (void)nothingInteresting
{
}

- (void)cancel
{
    [super cancel];
    CFRunLoopStop(self.runLoop.getCFRunLoop);
}

// Empty routine used for NSTimer selector
- (void)noop
{
}

// Called to start the thread
// We'll just spend our time in here
- (void)main
{
    if ([self isCancelled]) {
        return;
    }

    existenceLock.lock();

    @autoreleasepool {
        _runLoop = [NSRunLoop currentRunLoop];

        // Wake up our layers.  It's up to them to do the rest
        for (unsigned int ii=0;ii<[layers count];ii++)
        {
            NSObject<WhirlyKitLayer> *layer = [layers objectAtIndex:ii];
            [layer startWithThread:self scene:_scene];
        }
      
        // Process the run loop until we're cancelled
        // We'll check every 10th of a second
        while (![self isCancelled])
        {
            [pauseLock lock];
            while(paused)
            {
                [pauseLock wait];
            }
            @autoreleasepool {
                // Add a timer so the run loop doesn't return immediately
                NSTimer *timer = [NSTimer timerWithTimeInterval:1000000.0 target:self selector:@selector(noop) userInfo:nil repeats:NO];
//                NSTimer *timer = [NSTimer timerWithTimeInterval:1000000.0 repeats:NO block:^(NSTimer * _Nonnull timer) {
//                    // Does nothing but keeps CFRunLoopRun() from returning quite so quickly
//                }];
                [_runLoop addTimer:timer forMode:NSDefaultRunLoopMode];
                CFRunLoopRun();
                [timer invalidate];
            }
            [pauseLock unlock];
        }
        
        [NSObject cancelPreviousPerformRequestsWithTarget:self];
        
        [_viewWatcher stop];
        
        if ([threadsToShutdown count] > 0)
        {
            // We'll ask any other layer threads to shut down first, and wait for them to do it
            for (WhirlyKitLayerThread *theThread in threadsToShutdown)
                [theThread cancel];
            // And wait for them to do it
            bool finished = true;
            do {
                finished = true;
                for (WhirlyKitLayerThread *theThread in threadsToShutdown)
                    finished &= [theThread isFinished];
                if (!finished)
                    [NSThread sleepForTimeInterval:0.01];
            } while (!finished);

            [threadsToShutdown removeAllObjects];
            threadsToShutdown = nil;
        }

        // If we're not the main thread, let's clean up our layers before we shut down
        if (!_mainLayerThread)
        {
            for (NSObject<WhirlyKitLayer> *layer in layers)
                [layer teardown];
            
            [self runAddChangeRequests];
        }

        _runLoop = nil;
        // For some reason we need to do this explicitly in some cases
        while ([layers count] > 0)
            [self removeLayerThread:[layers objectAtIndex:0]];
        layers = nil;
    }
    
//    NSLog(@"Layer thread shutting down");
    
    if (_mainLayerThread)
    {
        // If any of the things we're to release are other layer threads
        //  we need to wait for them to shut down.
        for (WhirlyKitLayerThread *otherLayerThread in threadsToShutdown)
        {
            otherLayerThread->existenceLock.lock();
        }

        // Tear the scene down.  It's unsafe to do it elsewhere
        _scene->teardown();
    } else {
        // Okay, we're shutting down, so release the existence lock
        existenceLock.unlock();
    }
    
    // Delete outstanding change requests
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
        delete changeRequests[ii];
    changeRequests.clear();

    // Clean up the things the main thread has asked us to
    for (unsigned int ii=0;ii<thingsToDelete.size();ii++)
        delete thingsToDelete[ii];
    thingsToDelete.clear();
    while ([thingsToRelease count] > 0)
        [thingsToRelease removeObject:[thingsToRelease objectAtIndex:0]];
    
    // If this is the main thread, we are well and truly done
    if (_mainLayerThread) {
        // Okay, we're shutting down, so release the existence lock
        existenceLock.unlock();
    }
}


- (void)pause
{
    paused = true;
}

- (void)unpause
{
    paused = false;
    [pauseLock signal];
}

@end
