/*
 *  LayerThread.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import <UIKit/UIKit.h>
#import <vector>
#import "Scene.h"
#import "DataLayer.h"
#import "LayerViewWatcher.h"
#import "SceneRendererES.h"

/** The Layer Thread manages the objects that put data into a scene
    and take it out later.  These objects also handle the interaction
    with the user, if there is significant calculation or lookup involved.
    It starts its own thread, obviously, and does all the work there.
 */
@interface WhirlyKitLayerThread : NSThread
{
	/// Scene we're messing with
	WhirlyKit::Scene *scene;
    
    /// Used to let layers get view change notices
    WhirlyKitLayerViewWatcher *viewWatcher;
	
	/// The various data layers we'll display
	NSMutableArray<NSObject> *layers;
	
	/// Run loop created within our main
	NSRunLoop *runLoop;
    
    /// Our own EAGLContext, connected by a share group to the main one
    EAGLContext *glContext;
    
    /// The renderer we're working with
    WhirlyKitSceneRendererES __weak *renderer;
        
    /// Used to keep track of things to delete
    std::vector<WhirlyKit::DelayedDeletable *> thingsToDelete;
    
    /// Used to keep track of things to release
    NSMutableArray *thingsToRelease;
    
    /// Change requests to merge soonish
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
}

@property (nonatomic,readonly) NSRunLoop *runLoop;
@property (nonatomic,strong) WhirlyKitLayerViewWatcher *viewWatcher;
@property (nonatomic,readonly) EAGLContext *glContext;
@property (nonatomic,weak) WhirlyKitSceneRendererES *renderer;
/// Turn this off to disable flushes to GL on the layer thread.
/// The only reason to do this is going to background.  This is a temporary fix
@property (nonatomic,assign) bool allowFlush;

/// Set up with a scene and a view
- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES *)renderer;

/// Add these before you kick off the thread
- (void)addLayer:(NSObject<WhirlyKitLayer> *)layer;

/// Remove the given layer.
- (void)removeLayer:(NSObject<WhirlyKitLayer> *)layer;

/// Add a C++ object to be deleted after the thread has stopped
/// Always clal this from the main thread before you cancel the layer thread
- (void)addThingToDelete:(WhirlyKit::DelayedDeletable *)thing;

/// Add an Objective C object to release after the thread has stopped
/// Always call this from the main thread before you cancel the layer thread
- (void)addThingToRelease:(NSObject *)thing;

/// Layers need to send their change requests throgh here
- (void)addChangeRequest:(WhirlyKit::ChangeRequest *)changeRequest;

/// Layers should send their change requests through here
- (void)addChangeRequests:(std::vector<WhirlyKit::ChangeRequest *> &)changeRequests;

/// Called by a layer to request a flush at the next opportunity.
/// Presumably the layer did something worth flushing
- (void)requestFlush;

/// Explicitly flush the change requests out to the scene.
/// Only call this if you're trying to reduce latency between the layer thread
///  and the render.  And you know what this does.
- (void)flushChangeRequests;

/// We're overriding the main entry point
- (void)main;

@end
