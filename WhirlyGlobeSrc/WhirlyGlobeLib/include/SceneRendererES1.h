/*
 *  SceneRendererES1.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import "ESRenderer.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#import "GlobeView.h"
#import "GlobeScene.h"

/// @cond
@class WhirlyGlobeSceneRendererES1;
/// @endcond

/** Renderer Frame Info
    Data about the current frame, passed around by the renderer.
 */
@interface WhirlyGlobeRendererFrameInfo : NSObject
{
    /// Renderer itself
    WhirlyGlobeSceneRendererES1 *sceneRenderer;
    
    /// View
    WhirlyKitView *theView;
    
    /// Scene itself.  Don't mess with this
    WhirlyGlobe::GlobeScene *scene;
    
    /// Vector pointing up from the globe describing where the view point is
    Vector3f eyeVec;

    /// Expected length of the current frame
    float frameLen;
    
    /// Time at the start of frame
    NSTimeInterval currentTime;
}

@property (nonatomic,assign) WhirlyGlobeSceneRendererES1 *sceneRenderer;
@property (nonatomic,assign) WhirlyKitView *theView;
@property (nonatomic,assign) WhirlyGlobe::GlobeScene *scene;
@property (nonatomic,assign) float frameLen;
@property (nonatomic,assign) NSTimeInterval currentTime;
@property (nonatomic,assign) Vector3f eyeVec;

@end

/** Protocol for the scene render callbacks.
    These are all optional, but if set will be called
     at various points within the rendering process.
 */
@protocol WhirlyGlobeSceneRendererDelegate

@optional
/// This overrides the setup view, including lights and modes
/// Be sure to do *all* the setup if you do this
- (void)lightingSetup:(WhirlyGlobeSceneRendererES1 *)sceneRenderer;

/// Called right before a frame is rendered
- (void)preFrame:(WhirlyGlobeSceneRendererES1 *)sceneRenderer;

/// Called right after a frame is rendered
- (void)postFrame:(WhirlyGlobeSceneRendererES1 *)sceneRenderer;
@end

/// Number of frames to use for counting frames/sec
static const unsigned int RenderFrameCount = 25;

/** Scene Renderer for OpenGL ES1.
    This implements the actual rendering.  In theory it's
    somewhat composable, but in reality not all that much.
    Just set this up as in the examples and let it run.
 */
@interface WhirlyGlobeSceneRendererES1 : NSObject <WhirlyGlobeESRenderer>
{
    /// Rendering context
	EAGLContext *context;

    /// Scene we're drawing.  This is assigned from outside
	WhirlyGlobe::GlobeScene *scene;
    /// The view controls how we're looking at the scene
	WhirlyKitView *theView;

    /// The pixel width of the CAEAGLLayer.
    GLint framebufferWidth;
    /// The pixel height of the CAEAGLLayer.
    GLint framebufferHeight;

    /// OpenGL ES Name for the frame buffer
    GLuint defaultFramebuffer;
    /// OpenGL ES Name for the color buffer
    GLuint colorRenderbuffer;
    /// OpenGL ES Name for the depth buffer
    GLuint depthRenderbuffer;	
	
	/// Statistic: Frames per second
	float framesPerSec;
	unsigned int frameCount;
	NSDate *frameCountStart;
	
	/// Statistic: Number of drawables drawn in last frame
	unsigned int numDrawables;
    
    /// Delegate called at specific points in the rendering process
    id<WhirlyGlobeSceneRendererDelegate> delegate;

    /// This is the color used to clear the screen.  Defaults to black
    WhirlyGlobe::RGBAColor clearColor;
}

@property (nonatomic,assign) WhirlyGlobe::GlobeScene *scene;
@property (nonatomic,assign) WhirlyKitView *theView;

@property (nonatomic,readonly) GLint framebufferWidth,framebufferHeight;

@property (nonatomic,readonly) float framesPerSec;
@property (nonatomic,readonly) unsigned int numDrawables;

@property (nonatomic,assign) id<WhirlyGlobeSceneRendererDelegate> delegate;

/// Attempt to render the frame in the time given.
/// Ignoring the time at the moment.
- (void) render:(CFTimeInterval)duration;

/// Called when the underlying layer resizes and we need to adjust
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;

/// Use this to set the clear color for the screen.  Defaults to black
- (void) setClearColor:(UIColor *)color;

/// If you're setting up resources within OpenGL, you need to have that
///  context active.  Call this to do that.
- (void)useContext;

@end
