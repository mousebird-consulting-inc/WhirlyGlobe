/*
 *  LayerThread.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2012 mousebird consulting
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

@synthesize runLoop;
@synthesize viewWatcher;
@synthesize glContext;

- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES *)renderer;
{
	if ((self = [super init]))
	{
		scene = inScene;
		layers = [NSMutableArray array];
        // Note: This could be better
        if (dynamic_cast<WhirlyGlobe::GlobeScene *>(scene))
            viewWatcher = [[WhirlyGlobeLayerViewWatcher alloc] initWithView:(WhirlyGlobeView *)inView thread:self];
        else
            if (dynamic_cast<Maply::MapScene *>(scene))
                viewWatcher = [[MaplyLayerViewWatcher alloc] initWithView:(MaplyView *)inView thread:self];
        
        glContext = [[EAGLContext alloc] initWithAPI:renderer.context.API sharegroup:renderer.context.sharegroup];
        thingsToRelease = [NSMutableArray array];
	}
	
	return self;
}

- (void)dealloc
{
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
    [layer startWithThread:self scene:scene];
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
