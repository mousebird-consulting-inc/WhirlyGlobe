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
	}
	
	return self;
}


- (void)addLayer:(NSObject<WhirlyKitLayer> *)layer
{
	[layers addObject:layer];
}

- (void)removeLayer:(NSObject<WhirlyKitLayer> *)layer
{
    [self performSelector:@selector(removeLayerThread:) onThread:self withObject:layer waitUntilDone:NO];
}

// This runs in the layer thread
- (void)removeLayerThread:(NSObject<WhirlyKitLayer> *)layer
{
    if ([layers containsObject:layer])
    {
        [layer shutdown];
        [layers removeObject:layer];
    }
}

// Called to start the thread
// We'll just spend our time in here
- (void)main
{
    @autoreleasepool {
        self.runLoop = [NSRunLoop currentRunLoop];
        
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

        runLoop = nil;
        layers = nil;
    }
}

@end
