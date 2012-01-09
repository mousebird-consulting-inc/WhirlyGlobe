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

@interface WhirlyGlobeLayerThread()
@property(nonatomic,retain) NSMutableArray<NSObject> *layers;
@end

@implementation WhirlyGlobeLayerThread

@synthesize layers;
@synthesize runLoop;

- (id)initWithScene:(WhirlyGlobe::GlobeScene *)inScene
{
	if ((self = [super init]))
	{
		scene = inScene;
		self.layers = [[[NSMutableArray alloc] init] autorelease];
	}
	
	return self;
}

- (void)dealloc
{
	// This should release the layers
	self.layers = nil;
	
	[super dealloc];
}

- (void)addLayer:(NSObject<WhirlyGlobeLayer> *)layer
{
	[layers addObject:layer];
}

// Called to start the thread
// We'll just spend our time in here
- (void)main
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	self.runLoop = [NSRunLoop currentRunLoop];
	
	// Wake up our layers.  It's up to them to do the rest
	for (unsigned int ii=0;ii<[layers count];ii++)
	{
		NSObject<WhirlyGlobeLayer> *layer = [layers objectAtIndex:ii];
		[layer startWithThread:self scene:scene];
	}

	// Process the run loop until we're cancelled
	// We'll check every 10th of a second
	while (![self isCancelled])
	{
		[runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
		[pool release];
		pool = [[NSAutoreleasePool alloc] init];
	}
	
	self.runLoop = nil;
    self.layers = nil;
	[pool release];
}

@end
