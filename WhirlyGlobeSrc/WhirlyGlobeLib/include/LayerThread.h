/*
 *  LayerThread.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import <UIKit/UIKit.h>
#import <vector>
#import "GlobeScene.h"
#import "DataLayer.h"

/** The Layer Thread manages the objects that put data into a scene
    and take it out later.  These objects also handle the interaction
    with the user, if there is significant calculation or lookup involved.
    It starts its own thread, obviously, and does all the work there.
 */
@interface WhirlyKitLayerThread : NSThread
{
	/// Scene we're messing with
	WhirlyKit::Scene *scene;
	
	/// The various data layers we'll display
	NSMutableArray<NSObject> *layers;
	
	/// Run loop created within our main
	NSRunLoop *runLoop;
}

@property (nonatomic) NSRunLoop *runLoop;

/// Set it up with a renderer (for context) and a scene
- (id)initWithScene:(WhirlyKit::Scene *)scene;

/// Add these before you kick off the thread
- (void)addLayer:(NSObject<WhirlyKitLayer> *)layer;

// We're overriding the main entry point
- (void)main;

@end
