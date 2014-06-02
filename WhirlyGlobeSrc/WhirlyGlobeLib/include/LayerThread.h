/*
 *  LayerThread.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

/// Scene we're messing with
@property (nonatomic,readonly) WhirlyKit::Scene *scene;
/// Run loop created within our main
@property (nonatomic,readonly) NSRunLoop *runLoop;
/// Used to let layers get view change notices
@property (nonatomic,strong) WhirlyKitLayerViewWatcher *viewWatcher;
/// Our own EAGLContext, connected by a share group to the main one
@property (nonatomic,readonly) EAGLContext *glContext;
/// The renderer we're working with
@property (nonatomic,weak) WhirlyKitSceneRendererES *renderer;
/// Turn this off to disable flushes to GL on the layer thread.
/// The only reason to do this is going to background.  This is a temporary fix
@property (nonatomic,assign) bool allowFlush;
/// Set if this is the main layer thread, responsible for shutting down the scene
@property (nonatomic,assign) bool mainLayerThread;

/// Set up with a scene and a view
- (id)initWithScene:(WhirlyKit::Scene *)inScene view:(WhirlyKitView *)inView renderer:(WhirlyKitSceneRendererES *)renderer mainLayerThread:(bool)mainLayerThread;

/// Add these before you kick off the thread
- (void)addLayer:(NSObject<WhirlyKitLayer> *)layer;

/// Remove the given layer.
- (void)removeLayer:(NSObject<WhirlyKitLayer> *)layer;

/// Add a C++ object to be deleted after the thread has stopped
/// Always call this from the main thread before you cancel the layer thread
- (void)addThingToDelete:(WhirlyKit::DelayedDeletable *)thing;

/// Add an Objective C object to release after the thread has stopped
/// Always call this from the main thread before you cancel the layer thread
- (void)addThingToRelease:(NSObject *)thing;

/// If this is the main thread, we'll ask it to shut down any other layer threads.
/// Doing it like this avoids any problems we may have with shutdown synchronization
- (void)addThreadToShutdown:(WhirlyKitLayerThread *)thread;

/// Layers need to send their change requests throgh here.
/// You can call this from any thread.
- (void)addChangeRequest:(WhirlyKit::ChangeRequest *)changeRequest;

/// Layers should send their change requests through here
/// You can call this from any thread.
- (void)addChangeRequests:(std::vector<WhirlyKit::ChangeRequest *> &)changeRequests;

/// Called by a layer to request a flush at the next opportunity.
/// Presumably the layer did something worth flushing
/// You can call this from any thread.
- (void)requestFlush;

/// Explicitly flush the change requests out to the scene.
/// Only call this if you're trying to reduce latency between the layer thread
///  and the render.  And you know what this does.
- (void)flushChangeRequests;

/// Dump out logging info.  Call this anywhere, but it'll run on the layer thread.
- (void)log;

/// We're overriding the main entry point
- (void)main;

@end
