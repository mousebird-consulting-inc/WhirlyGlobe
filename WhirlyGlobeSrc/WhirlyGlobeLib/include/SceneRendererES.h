/*
 *  ESRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Cullable.h"
#import "Lighting.h"

/// @cond
@class WhirlyKitSceneRendererES;
/// @endcond

namespace WhirlyKit
{
// Alpha stuff goes at the end
// Otherwise sort by draw priority
    class DrawListSortStruct
    {
    public:
        DrawListSortStruct(bool useAlpha,bool useLines,WhirlyKitRendererFrameInfo *frameInfo) : useAlpha(useAlpha), useLines(useLines), frameInfo(frameInfo)
        {
        }
        ~DrawListSortStruct() { }
        DrawListSortStruct(const DrawListSortStruct &that) : useAlpha(that.useAlpha), useLines(that.useLines), frameInfo(that.frameInfo)
        {
        }
        DrawListSortStruct & operator = (const DrawListSortStruct &that)
        {
            useAlpha = that.useAlpha;
            useLines= that.useLines;
            frameInfo = that.frameInfo;
            return *this;
        }
        bool operator()(Drawable *a,Drawable *b)
        {
            if (useLines)
            {
                bool linesA = (a->getType() == GL_LINES) || (a->getType() == GL_LINE_LOOP) || (a->getType() == GL_POINTS) || a->getForceZBufferOn();
                bool linesB = (b->getType() == GL_LINES) || (b->getType() == GL_LINE_LOOP) || (b->getType() == GL_POINTS) || b->getForceZBufferOn();
                if (linesA != linesB)
                    return !linesA;
            }
            // We may or may not sort all alpha containing drawables to the end
            if (useAlpha)
                if (a->hasAlpha(frameInfo) != b->hasAlpha(frameInfo))
                    return !a->hasAlpha(frameInfo);
                    
            return a->getDrawPriority() < b->getDrawPriority();
        }
        
        bool useAlpha,useLines;
        WhirlyKitRendererFrameInfo * __unsafe_unretained frameInfo;
    };
}

/** Renderer Frame Info.
 Data about the current frame, passed around by the renderer.
 */
@interface WhirlyKitRendererFrameInfo : NSObject
{
    /// Renderer version (e.g. OpenGL ES 1 vs 2)
    EAGLRenderingAPI oglVersion;
    
    /// Renderer itself
    WhirlyKitSceneRendererES * __weak sceneRenderer;
    
    /// View
    WhirlyKitView * __weak theView;
    
    /// Current model matrix from the view
    Eigen::Matrix4f modelTrans,viewTrans;
    
    /// Current projection matrix
    Eigen::Matrix4f projMat;
    
    /// What's currently in the GL model matrix.
    /// We combine view and model together
    Eigen::Matrix4f viewAndModelMat;
    
    /// The model, view, and projection matrix all rolled into one
    Eigen::Matrix4f mvpMat;
    
    /// Scene itself.  Don't mess with this
    WhirlyKit::Scene *scene;
    
    /// Vector pointing up from the globe describing where the view point is
    Eigen::Vector3f eyeVec;
    
    /// Expected length of the current frame
    float frameLen;
    
    /// Time at the start of frame
    NSTimeInterval currentTime;
    
    /// If using OpenGL ES 2.x, this is the shader
    WhirlyKit::OpenGLES2Program *program;
    
    /// Lights, if applicable
    NSArray *lights;
}

@property (nonatomic,assign) EAGLRenderingAPI oglVersion;
@property (nonatomic,weak) WhirlyKitSceneRendererES *sceneRenderer;
@property (nonatomic,weak) WhirlyKitView *theView;
@property (nonatomic,assign) Eigen::Matrix4f modelTrans,viewTrans;
@property (nonatomic,assign) Eigen::Matrix4f &projMat;
@property (nonatomic,assign) Eigen::Matrix4f &viewAndModelMat;
@property (nonatomic,assign) Eigen::Matrix4f &mvpMat;
@property (nonatomic,assign) WhirlyKit::Scene *scene;
@property (nonatomic,assign) float frameLen;
@property (nonatomic,assign) NSTimeInterval currentTime;
@property (nonatomic,assign) Eigen::Vector3f eyeVec;
@property (nonatomic,assign) WhirlyKit::OpenGLES2Program *program;
@property (nonatomic,strong) NSArray *lights;

@end

/** We support three different ways of using z buffer.  (1) Regular mode where it's on.
    (2) Completely off, priority sorting only.  (3) Priority sorting, but lines are sorted to
    the back, the z buffer is turned on and then they're drawn.
  */
typedef enum {zBufferOn,zBufferOff,zBufferOffUntilLines} WhirlyKitSceneRendererZBufferMode;

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
@interface WhirlyKitSceneRendererES : NSObject
{
    /// Rendering context
	EAGLContext *context;
    
    /// Scene we're drawing.  This is set from outside
	WhirlyKit::Scene *scene;
    /// The view controls how we're looking at the scene
	WhirlyKitView * __weak theView;
    
    /// Set this mode to modify how Z buffering is used (if at all)
    WhirlyKitSceneRendererZBufferMode zBufferMode;
    
    /// Set this to turn culling on or off.
    /// By default it's on, so leave it alone unless you know you want it off.
    bool doCulling;
    
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
    
    /// This is the color used to clear the screen.  Defaults to black
    WhirlyKit::RGBAColor clearColor;
    
    /// Set if we're using the view based change mechanism to tell when to draw.
    /// This works well for figuring out when the model matrix changes, but
    ///  not so well with animation such as fades, particles systems and such.
    bool useViewChanged;
    
    /// By default we'll sort all alpha-containing drawables to the end.
    /// Turn this off to tell the renderer you knew what you're doing and
    ///  don't mess with my draw priorities.
    bool sortAlphaToEnd;
    
    // If this is set, we'll turn off the depth buffering the first time
    //  we hit a drawable with alpha.  Off by default (not surprisingly).
    bool depthBufferOffForAlpha;
    
    /// Last time we rendered
    NSTimeInterval lastDraw;
    
    /// Something wants to make sure we render until at least this point.
    NSTimeInterval renderUntil;
    
    /// We use this to trigger a draw at the next opportunity.
    /// For some reason something we can't see changed.
    bool triggerDraw;
    
    // View state from the last render, for comparison
    Eigen::Matrix4f modelMat,viewMat;
}

@property (nonatomic,readonly) EAGLContext *context;
@property (nonatomic,assign) WhirlyKit::Scene *scene;
@property (nonatomic,weak) WhirlyKitView *theView;
@property (nonatomic,assign) WhirlyKitSceneRendererZBufferMode zBufferMode;
@property (nonatomic,assign) bool doCulling;

@property (nonatomic,readonly) GLint framebufferWidth,framebufferHeight;
@property (nonatomic,readonly) float scale;

@property (nonatomic,readonly) float framesPerSec;
@property (nonatomic,readonly) unsigned int numDrawables;
@property (nonatomic,assign) int perfInterval;

@property (nonatomic,assign) bool useViewChanged;
@property (nonatomic,assign) bool sortAlphaToEnd;
@property (nonatomic,assign) bool depthBufferOffForAlpha;

/// Initialize with API version
- (id) initWithOpenGLESVersion:(EAGLRenderingAPI)apiVersion;

/// Render to the screen, ideally within the given duration.
/// The subclasses fill this in
- (void)render:(NSTimeInterval)duration;

/// Called when the layer gets resized.  Need to resize ourselves
- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer;

/// Call this before defining things within the OpenGL context
- (void)useContext;

/// Use this to set the clear color for the screen.  Defaults to black
- (void) setClearColor:(UIColor *)color;

/// Set the render until time.  This is used by things like fade to keep
///  the rendering optimization from cutting off animation.
- (void)setRenderUntil:(NSTimeInterval)newTime;

/// Force a draw at the next opportunity
- (void)setTriggerDraw;

/// Call this to force a draw on the next frame.
/// This turns off the draw optimization, but just for one frame.
- (void)forceDrawNextFrame;

/// Used by the subclasses for culling
- (void)findDrawables:(WhirlyKit::Cullable *)cullable view:(WhirlyGlobeView *)globeView frameSize:(WhirlyKit::Point2f)frameSize modelTrans:(Eigen::Matrix4f *)modelTrans eyeVec:(Eigen::Vector3f)eyeVec frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(WhirlyKit::Mbr)screenMbr topLevel:(bool)isTopLevel toDraw:(std::set<WhirlyKit::DrawableRef> *) toDraw considered:(int *)drawablesConsidered;

/// Used by the subclasses to determine if the view changed and needs to be updated
- (bool) viewDidChange;

@end
