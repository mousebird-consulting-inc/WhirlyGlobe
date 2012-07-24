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
#import "WhirlyKitView.h"
#import "Scene.h"

namespace WhirlyKit
{
    
/// Simple performance timing class
class PerformanceTimer
{
public:
    /// Used to track a category of timing
    class TimeEntry
    {
    public:
        TimeEntry();
        bool operator < (const TimeEntry &that) const;
        
        void addTime(NSTimeInterval dur);
        
        std::string name;
        NSTimeInterval minDur,maxDur,avgDur;
        int numRuns;
    };
    
    /// Used to track a category of counts
    class CountEntry
    {
    public:
        CountEntry();
        bool operator < (const CountEntry &that) const;
        
        void addCount(int count);
        
        std::string name;
        int minCount,maxCount,avgCount;
        int numRuns;
    };

    /// Start timing the given thing
    void startTiming(const std::string &);
    
    /// Stop timing the given thing and add it to the existing timings
    void stopTiming(const std::string &);
    
    /// Add a count for a particular instance
    void addCount(const std::string &what,int count);
    
    /// Clean out existing timings
    void clear();
    
    /// Write out the timings to NSLog
    void log();
    
protected:
    std::map<std::string,NSTimeInterval> actives;
    std::map<std::string,TimeEntry> timeEntries;
    std::map<std::string,CountEntry> countEntries;
};

}

/// @cond
@class WhirlyKitSceneRendererES1;
/// @endcond

/** Renderer Frame Info
    Data about the current frame, passed around by the renderer.
 */
@interface WhirlyKitRendererFrameInfo : NSObject
{
    /// Renderer itself
    WhirlyKitSceneRendererES1 * __weak sceneRenderer;
    
    /// View
    WhirlyKitView * __weak theView;
    
    /// Current model matrix from the view
    Eigen::Matrix4f modelTrans;
    
    /// Current projection matrix
    Eigen::Matrix4f projMat;
    
    /// Scene itself.  Don't mess with this
    WhirlyKit::Scene *scene;
    
    /// Vector pointing up from the globe describing where the view point is
    Vector3f eyeVec;
    
    /// Expected length of the current frame
    float frameLen;
    
    /// Time at the start of frame
    NSTimeInterval currentTime;
}

@property (nonatomic,weak) WhirlyKitSceneRendererES1 *sceneRenderer;
@property (nonatomic,weak) WhirlyKitView *theView;
@property (nonatomic,assign) Eigen::Matrix4f modelTrans;
@property (nonatomic,assign) Eigen::Matrix4f &projMat;
@property (nonatomic,assign) WhirlyKit::Scene *scene;
@property (nonatomic,assign) float frameLen;
@property (nonatomic,assign) NSTimeInterval currentTime;
@property (nonatomic,assign) Vector3f eyeVec;

@end

/** Protocol for the scene render callbacks.
    These are all optional, but if set will be called
     at various points within the rendering process.
 */
@protocol WhirlyKitSceneRendererDelegate

@optional

/// Return true if the lighting has changed since last lightingSetup: was called
- (BOOL)lightingChanged:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// This overrides the setup view, including lights and modes
/// Be sure to do *all* the setup if you do this
- (void)lightingSetup:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// Called right before a frame is rendered
- (void)preFrame:(WhirlyKitSceneRendererES1 *)sceneRenderer;

/// Called right after a frame is rendered
- (void)postFrame:(WhirlyKitSceneRendererES1 *)sceneRenderer;
@end

/** Scene Renderer for OpenGL ES1.
    This implements the actual rendering.  In theory it's
    somewhat composable, but in reality not all that much.
    Just set this up as in the examples and let it run.
 */
@interface WhirlyKitSceneRendererES1 : NSObject <WhirlyKitESRenderer>
{
    /// Rendering context
	EAGLContext *context;

    /// Scene we're drawing.  This is set from outside
	WhirlyKit::Scene *scene;
    /// The view controls how we're looking at the scene
	WhirlyKitView * __weak theView;
    
    /// Set this to turn z buffering on or off.
    /// If you turn z buffering off, drawPriority in the drawables is still used
    bool zBuffer;

    /// The pixel width of the CAEAGLLayer.
    GLint framebufferWidth;
    /// The pixel height of the CAEAGLLayer.
    GLint framebufferHeight;
    
    /// Scale, to reflect the device's screen
    float scale;

    /// OpenGL ES Name for the frame buffer
    GLuint defaultFramebuffer;
    /// OpenGL ES Name for the color buffer
    GLuint colorRenderbuffer;
    /// OpenGL ES Name for the depth buffer
    GLuint depthRenderbuffer;	
	
	/// Statistic: Frames per second
	float framesPerSec;
	unsigned int frameCount;
	NSTimeInterval frameCountStart;
    /// Period over which we measure performance
    int perfInterval;
    WhirlyKit::PerformanceTimer perfTimer;
	
	/// Statistic: Number of drawables drawn in last frame
	unsigned int numDrawables;
    
    /// Delegate called at specific points in the rendering process
    NSObject<WhirlyKitSceneRendererDelegate> * __weak delegate;

    /// This is the color used to clear the screen.  Defaults to black
    WhirlyKit::RGBAColor clearColor;
    
    /// Set if we're using the view based change mechanism to tell when to draw.
    /// This works well for figuring out when the model matrix changes, but
    ///  not so well with animation such as fades, particles systems and such.
    bool useViewChanged;
    
    /// Last time we rendered
    NSTimeInterval lastDraw;
    
    // View state from the last render, for comparison
    Matrix4f modelMat,viewMat;
}

@property (nonatomic,readonly) EAGLContext *context;
@property (nonatomic,assign) WhirlyKit::Scene *scene;
@property (nonatomic,weak) WhirlyKitView *theView;
@property (nonatomic,assign) bool zBuffer;

@property (nonatomic,readonly) GLint framebufferWidth,framebufferHeight;
@property (nonatomic,readonly) float scale;

@property (nonatomic,readonly) float framesPerSec;
@property (nonatomic,readonly) unsigned int numDrawables;
@property (nonatomic,assign) int perfInterval;

@property (nonatomic,assign) bool useViewChanged;

@property (nonatomic,weak) NSObject<WhirlyKitSceneRendererDelegate> *delegate;

/// Attempt to render the frame in the time given.
/// Ignoring the time at the moment.
- (void) render:(NSTimeInterval)duration;

/// Called when the underlying layer resizes and we need to adjust
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;

/// Use this to set the clear color for the screen.  Defaults to black
- (void) setClearColor:(UIColor *)color;

/// If you're setting up resources within OpenGL, you need to have that
///  context active.  Call this to do that.
- (void)useContext;

/// Call this to force a draw on the next frame.
/// This turns off the draw optimization, but just for one frame.
- (void)forceDrawNextFrame;

@end
