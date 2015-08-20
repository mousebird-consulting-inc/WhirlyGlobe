/*
 *  SceneRenderer_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/16/13.
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

#import <WhirlyGlobe.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#define kWKFrameMessage @"WhirlyKitFrameMessage"
/** This message is sent out by the renderer right
    before it does its thing.  We use it to loosely
    sync other threads to the render.
  */
@interface WhirlyKitFrameMessage : NSObject

/// When the message is sent, basically
@property (nonatomic) TimeInterval frameStart;

/// The interval between frames
@property (nonatomic) TimeInterval frameInterval;

/// The message is coming from this renderer
@property (nonatomic) WhirlyKit::SceneRendererES2 *renderer;

@end

namespace WhirlyKit
{

/// iOS level wrapper around the scene renderer
class MaplySceneRendererES2 : public SceneRendererES2
{
public:
    MaplySceneRendererES2();
    virtual ~MaplySceneRendererES2();
    
    /// Called when the layer gets resized.  Need to resize ourselves
    virtual BOOL resizeFromLayer(CAEAGLLayer *layer);
    
    /// Set the context as active
    void useContext();
    
    /// Set the Scene, once it exists
    virtual void setScene(WhirlyKit::Scene *scene);
    
    /// Return the EAGLContext
    EAGLContext *getContext() { return context; }

    // Kick off the render
    void render(TimeInterval duration);

protected:
    void renderAsync();
    
    /// Rendering context
    EAGLContext *context;
    
    /// If set, we'll let the render run on a dispatch queue.
    /// This lets the UI run in the main thread without interference,
    ///  but it does mean you can't mess with the rendering context.
    bool _dispatchRendering;

    dispatch_queue_t contextQueue;
    dispatch_semaphore_t frameRenderingSemaphore;
    bool renderSetup;
    WhirlyKit::OpenGLStateOptimizer *renderStateOptimizer;
};

}
