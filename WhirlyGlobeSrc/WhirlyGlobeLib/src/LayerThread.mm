/*
 *  LayerThread.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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

#import "LayerThread.h"
#import "GlobeLayerViewWatcher.h"
#import "GlobeScene.h"
#import "GlobeView.h"

using namespace WhirlyKit;

@implementation WhirlyKitLayerThread

@synthesize runLoop;
@synthesize viewWatcher;
@synthesize glContext;
@synthesize memManager;

- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES1 *)renderer;
{
	if ((self = [super init]))
	{
		scene = inScene;
		layers = [NSMutableArray array];
        // Note: This could be better
        if (dynamic_cast<WhirlyGlobe::GlobeScene *>(scene))
            viewWatcher = [[WhirlyGlobeLayerViewWatcher alloc] initWithView:(WhirlyGlobeView *)inView thread:self];
        
        glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1 sharegroup:renderer.context.sharegroup];
        memManager = new OpenGLMemManager();
        thingsToRelease = [NSMutableArray array];
	}
	
	return self;
}

- (void)dealloc
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    if (memManager)
        delete memManager;
    memManager = nil;
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
    [layer startWithThread:self scene:scene];
}

- (void)removeLayer:(NSObject<WhirlyKitLayer> *)layer
{
    [self performSelector: @selector(removeLayerThread:) onThread: self withObject:layer waitUntilDone:NO];
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

// Called every so often to move OpenGL IDs between threads
- (void)openglMemCleanup
{
    if (![self isCancelled])
    {
        // This will block, thus letting us mess with both memory objects at the same time
        [self performSelectorOnMainThread:@selector(openglMemCleanupMain) withObject:nil waitUntilDone:YES];
        
        // Do this every second or so
        [self performSelector:@selector(openglMemCleanup) withObject:nil afterDelay:1.0];
    }
}

// We'll move free'd IDs back from the main thread to the layer thread
- (void)openglMemCleanupMain
{
    memManager->copyIDsFrom(scene->getMemManager());
}

// Called to start the thread
// We'll just spend our time in here
- (void)main
{
    @autoreleasepool {
        runLoop = [NSRunLoop currentRunLoop];
        
        // Wake up our layers.  It's up to them to do the rest
        for (unsigned int ii=0;ii<[layers count];ii++)
        {
            NSObject<WhirlyKitLayer> *layer = [layers objectAtIndex:ii];
            [layer startWithThread:self scene:scene];
        }
        
        [self performSelector:@selector(openglMemCleanup) withObject:nil afterDelay:0.0];

        // Process the run loop until we're cancelled
        // We'll check every 10th of a second
        while (![self isCancelled])
        {
            @autoreleasepool {
                [runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
            }
        }
        
        [NSObject cancelPreviousPerformRequestsWithTarget:self];

        runLoop = nil;
        // For some reason we need to do this explicitly in some cases
        while ([layers count] > 0)
            [self removeLayerThread:[layers objectAtIndex:0]];
        layers = nil;
    }

    // Clean up the things the main thread has asked us to
    for (unsigned int ii=0;ii<thingsToDelete.size();ii++)
        delete thingsToDelete[ii];
    thingsToDelete.clear();
    while ([thingsToRelease count] > 0)
        [thingsToRelease removeObject:[thingsToRelease objectAtIndex:0]];
}

@end
