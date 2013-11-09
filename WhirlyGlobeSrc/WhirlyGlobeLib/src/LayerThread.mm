/*
 *  LayerThread.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2013 mousebird consulting
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
#import "GlobeLayerViewWatcher.h"
#import "MaplyLayerViewWatcher.h"
#import "GlobeScene.h"
#import "MaplyScene.h"
#import "GlobeView.h"

using namespace WhirlyKit;

@implementation WhirlyKitLayerThread
{
    WhirlyKitGLSetupInfo *glSetupInfo;
    /// The various data layers we'll display
    NSMutableArray<NSObject> *layers;
    
    /// Used to keep track of things to delete
    std::vector<WhirlyKit::DelayedDeletable *> thingsToDelete;
    
    /// Used to keep track of things to release
    NSMutableArray *thingsToRelease;
    
    /// Threads to shut down
    NSMutableArray *threadsToShutdown;
    
    /// Change requests to merge soonish
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
    
    /// We can get change requests from other threads (!)
    pthread_mutex_t changeLock;
    
    /// We lock this in the main loop.  If anyone else can lock it, that means we're gone.
    /// Yes, I'm certain there's a better way to do this.
    pthread_mutex_t existenceLock;
}

- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES *)inRenderer mainLayerThread:(bool)mainLayerThread
{
	if ((self = [super init]))
	{
        _mainLayerThread = mainLayerThread;
		_scene = inScene;
        _renderer = inRenderer;
		layers = [NSMutableArray array];
        // Note: This could be better
        if (dynamic_cast<WhirlyGlobe::GlobeScene *>(_scene))
            _viewWatcher = [[WhirlyGlobeLayerViewWatcher alloc] initWithView:(WhirlyGlobeView *)inView thread:self];
        else
            if (dynamic_cast<Maply::MapScene *>(_scene))
                _viewWatcher = [[MaplyLayerViewWatcher alloc] initWithView:(MaplyView *)inView thread:self];
        
        // We'll create the context here and set it in the layer thread, always
        _glContext = [[EAGLContext alloc] initWithAPI:_renderer.context.API sharegroup:_renderer.context.sharegroup];

        thingsToRelease = [NSMutableArray array];
        threadsToShutdown = [NSMutableArray array];
        
        glSetupInfo = [[WhirlyKitGLSetupInfo alloc] init];
        glSetupInfo->minZres = [inView calcZbufferRes];
        _allowFlush = true;
        
        pthread_mutex_init(&changeLock,NULL);
        pthread_mutex_init(&existenceLock,NULL);
	}
	
	return self;
}

- (void)dealloc
{
    pthread_mutex_destroy(&changeLock);
    pthread_mutex_destroy(&existenceLock);
    // Note: It's not clear why we'd do this here.
    //       What run loop would it be referring to?
//    [NSObject cancelPreviousPerformRequestsWithTarget:self];    
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
    [self performSelector: @selector(removeLayerThread:) onThread: self withObject:layer waitUntilDone:YES];
}

// This runs in the layer thread
- (void)removeLayerThread:(NSObject<WhirlyKitLayer> *)layer
{
    if ([layers containsObject:layer])
    {
        // If we're done, we won't bother shutting down things nicely
        if (![self isCancelled])
            [layer shutdown];
        [layers removeObject:layer];
    }
}

- (void)addThingToDelete:(WhirlyKit::DelayedDeletable *)thing
{
    thingsToDelete.push_back(thing);
}

- (void)addThingToRelease:(NSObject *)thing
{
    [thingsToRelease addObject:thing];
}

- (void)addThreadToShutdown:(WhirlyKitLayerThread *)thread
{
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
    pthread_mutex_lock(&changeLock);

    // If we don't have one coming, schedule a merge
    if (changeRequests.empty())
        [self performSelector:@selector(runAddChangeRequests) withObject:nil afterDelay:0.0];
    
    changeRequests.insert(changeRequests.end(), newChangeRequests.begin(), newChangeRequests.end());
    
    pthread_mutex_unlock(&changeLock);
}

- (void)flushChangeRequests
{
    pthread_mutex_lock(&changeLock);
    
    [self runAddChangeRequests];
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(runAddChangeRequests) object:nil];
    
    pthread_mutex_unlock(&changeLock);
}

- (void)requestFlush
{
    [self addChangeRequest:NULL];
}

- (void)runAddChangeRequests
{
    [EAGLContext setCurrentContext:_glContext];

    bool requiresFlush = false;
    // Set up anything that needs to be set up
    ChangeSet changesToAdd;
    for (unsigned int ii=0;ii<changeRequests.size();ii++)
    {
        ChangeRequest *change = changeRequests[ii];
        if (change)
        {
            requiresFlush |= change->needsFlush();
            change->setupGL(glSetupInfo, _scene->getMemManager());
            changesToAdd.push_back(changeRequests[ii]);
        } else
            // A NULL change request is just a flush request
            requiresFlush = true;
    }
    
    // If anything needed a flush after that, let's do it
    if (requiresFlush && _allowFlush)
    {
        glFlush();
    }
    
    _scene->addChangeRequests(changesToAdd);
    changeRequests.clear();
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

// Called to start the thread
// We'll just spend our time in here
- (void)main
{
    pthread_mutex_lock(&existenceLock);
    
    // This should be the default context.  If you change it yourself, change it back
    [EAGLContext setCurrentContext:_glContext];

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
            @autoreleasepool {
                [_runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            }
        }
        
        [NSObject cancelPreviousPerformRequestsWithTarget:self];
        
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
        }

        // If we're not the main thread, let's clean up our layers before we shut down
        if (!_mainLayerThread)
        {
            for (NSObject<WhirlyKitLayer> *layer in layers)
                [layer shutdown];
            
            [self runAddChangeRequests];
        }

        _runLoop = nil;
        // For some reason we need to do this explicitly in some cases
        while ([layers count] > 0)
            [self removeLayerThread:[layers objectAtIndex:0]];
        layers = nil;
    }
    
    // Okay, we're shutting down, so release the existence lock
    pthread_mutex_unlock(&existenceLock);
    
    if (_mainLayerThread)
    {
        // If any of the things we're to releas are other layer threads
        //  we need to wait for them to shut down.
        for (NSObject *thing in thingsToRelease)
        {
            if ([thing isKindOfClass:[WhirlyKitLayerThread class]])
            {
                WhirlyKitLayerThread *otherLayerThread = (WhirlyKitLayerThread *)thing;
                pthread_mutex_lock(&otherLayerThread->existenceLock);
            }
        }

        // Tear the scene down.  It's unsafe to do it elsewhere
        _scene->teardownGL();
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
    
    _glContext = nil;
}

@end
