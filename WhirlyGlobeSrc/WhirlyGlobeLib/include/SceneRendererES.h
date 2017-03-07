/*
 *  ESRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
 *  Copyright 2011-2015 mousebird consulting
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
    DrawListSortStruct(bool useAlpha,bool useZBuffer,WhirlyKitRendererFrameInfo *frameInfo) : useAlpha(useAlpha), useZBuffer(useZBuffer), frameInfo(frameInfo)
    {
    }
    ~DrawListSortStruct() { }
    DrawListSortStruct(const DrawListSortStruct &that) : useAlpha(that.useAlpha), useZBuffer(that.useZBuffer), frameInfo(that.frameInfo)
    {
    }
    DrawListSortStruct & operator = (const DrawListSortStruct &that)
    {
        useAlpha = that.useAlpha;
        useZBuffer= that.useZBuffer;
        frameInfo = that.frameInfo;
        return *this;
    }
    bool operator()(Drawable *a,Drawable *b)
    {
        if (useZBuffer)
        {
            bool bufferA = a->getRequestZBuffer();
            bool bufferB = b->getRequestZBuffer();
            if (bufferA != bufferB)
                return !bufferA;
        }
        // We may or may not sort all alpha containing drawables to the end
        if (useAlpha)
            if (a->hasAlpha(frameInfo) != b->hasAlpha(frameInfo))
                return !a->hasAlpha(frameInfo);
                
        return a->getDrawPriority() < b->getDrawPriority();
    }
    
    bool useAlpha,useZBuffer;
    WhirlyKitRendererFrameInfo * __unsafe_unretained frameInfo;
};

/** What and where we're rendering.  This can be a regular framebuffer
 to the screen or to a texture.
 */
class RenderTarget : public Identifiable
{
public:
    RenderTarget();
    RenderTarget(SimpleIdentity newID) : Identifiable(newID) { }
    
    // Set up the render target
    bool init(Scene *scene,SimpleIdentity targetTexID);
    
    // Clear up resources from the render target
    void clear();
    
    /// Make this framebuffer active
    void setActiveFramebuffer(WhirlyKitSceneRendererES *renderer);
    
    /// OpenGL ES Name for the frame buffer
    GLuint framebuffer;
    /// OpenGL ES Name for the color buffer
    GLuint colorbuffer;
    /// OpenGL ES Name for the depth buffer
    GLuint depthbuffer;
    /// Output framebuffer size fo glViewport
    int width,height;
    /// Set if we've set up background and such
    bool isSetup;
};

// Add a new render target
class AddRenderTargetReq : public ChangeRequest
{
public:
    AddRenderTargetReq(SimpleIdentity renderTargetID,int width,int height,SimpleIdentity texID);
    
    /// Add the render target to the renderer
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    int width,height;
    SimpleIdentity renderTargetID;
    SimpleIdentity texID;
};

class RemRenderTargetReq : public ChangeRequest
{
public:
    RemRenderTargetReq(SimpleIdentity targetID);
    
    /// Remove the render target from the renderer
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    SimpleIdentity targetID;
};

}

/// OpenGL ES state optimizer.  This short circuits many of the OGL state
///  changes that would otherwise be redundant.
@interface WhirlyKitOpenGLStateOptimizer : NSObject

/// Calls glActiveTextures
- (void)setActiveTexture:(GLenum)activeTexture;

/// Calls glDepthMask
- (void)setDepthMask:(bool)depthMask;

/// Calls glEnable(GL_DEPTH_TEST) or glDisable(GL_DEPTH_TEST)
- (void)setEnableDepthTest:(bool)enable;

/// Calls glDepthFunc
- (void)setDepthFunc:(GLenum)depthFuncVal;

/// Calls glUseProgram
- (void)setUseProgram:(GLuint)progId;

/// Calls glLineWidth
- (void)setLineWidth:(GLfloat)lineWidth;

/// Called by the render to clear state
- (void)reset;

@end

/** Renderer Frame Info.
 Data about the current frame, passed around by the renderer.
 */
@interface WhirlyKitRendererFrameInfo : NSObject

/// Renderer version (e.g. OpenGL ES 1 vs 2)
@property (nonatomic,assign) EAGLRenderingAPI oglVersion;
/// Renderer itself
@property (nonatomic,weak) WhirlyKitSceneRendererES * __weak sceneRenderer;
/// View
@property (nonatomic,weak) WhirlyKitView *theView;
/// Current model matrix from the view
@property (nonatomic,assign) Eigen::Matrix4f modelTrans,viewTrans;
@property (nonatomic,assign) Eigen::Matrix4d modelTrans4d,viewTrans4d;
/// Current projection matrix
@property (nonatomic,assign) Eigen::Matrix4f &projMat;
@property (nonatomic,assign) Eigen::Matrix4d &projMat4d;
/// What's currently in the GL model matrix.
/// We combine view and model together
@property (nonatomic,assign) Eigen::Matrix4f &viewAndModelMat;
@property (nonatomic,assign) Eigen::Matrix4d &viewAndModelMat4d;
/// The model, view, and projection matrix all rolled into one
@property (nonatomic,assign) Eigen::Matrix4f &mvpMat;
/// Normal matrix for model/view/projection
@property (nonatomic,assign) Eigen::Matrix4f &mvpNormalMat;
/// Model, and view matrix but for normal transformation
@property (nonatomic,assign) Eigen::Matrix4f &viewModelNormalMat;
/// Projection, view, and offset matrices rolled together
@property (nonatomic,assign) Eigen::Matrix4d &pvMat4d;
@property (nonatomic,assign) Eigen::Matrix4f &pvMat;
/// If the visual view supports wrapping, these are the available offset matrices
@property (nonatomic,assign) std::vector<Eigen::Matrix4d> &offsetMatrices;
/// Scene itself.  Don't mess with this
@property (nonatomic,assign) WhirlyKit::Scene *scene;
/// Expected length of the current frame
@property (nonatomic,assign) float frameLen;
/// Time at the start of frame
@property (nonatomic,assign) NSTimeInterval currentTime;
/// Vector pointing up from the globe describing where the view point is
@property (nonatomic,assign) Eigen::Vector3f eyeVec;
/// Vector out from the eye point, including tilt
@property (nonatomic,assign) Eigen::Vector3f fullEyeVec;
/// Position of user
@property (nonatomic,assign) Eigen::Vector3d eyePos;
/// Location of the middle of the screen in display coordinates
@property (nonatomic,assign) Eigen::Vector3d dispCenter;
/// Height above surface, if that makes sense
@property (nonatomic,assign) float heightAboveSurface;
/// Screen size in display coordinates
@property (nonatomic,assign) WhirlyKit::Point2d &screenSizeInDisplayCoords;
/// If using OpenGL ES 2.x, this is the shader
@property (nonatomic,assign) WhirlyKit::OpenGLES2Program *program;
/// Lights, if applicableNSArray *lights;
@property (nonatomic,strong) NSArray *lights;
/// State optimizer.  Used when setting state for drawing
@property (nonatomic,strong) WhirlyKitOpenGLStateOptimizer *stateOpt;

// Make a copy of the frame info
- (id)initWithFrameInfo:(WhirlyKitRendererFrameInfo *)info;

@end

/** We support three different ways of using z buffer.  (1) Regular mode where it's on.
    (2) Completely off, priority sorting only.  (3) Priority sorting, but drawables
        are allowed to force the z buffer on temporarily.
  */
typedef enum {zBufferOn,zBufferOff,zBufferOffDefault} WhirlyKitSceneRendererZBufferMode;

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
@interface WhirlyKitSceneRendererES : NSObject
{
@public
	unsigned int frameCount;
	NSTimeInterval frameCountStart;
    WhirlyKit::PerformanceTimer perfTimer;
        
    /// Last time we rendered
    NSTimeInterval lastDraw;
    
    /// Something wants to make sure we render until at least this point.
    NSTimeInterval renderUntil;
    
    // The drawables that want continuous rendering on
    WhirlyKit::SimpleIDSet contRenderRequests;
    
    WhirlyKit::RGBAColor _clearColor;
    
    // What we're rendering to (and where)
    std::vector<WhirlyKit::RenderTarget> renderTargets;
}

/// Rendering context
@property (nonatomic,readonly) EAGLContext *context;
/// Scene we're drawing.  This is set from outside
@property (nonatomic,assign) WhirlyKit::Scene *scene;
/// The view controls how we're looking at the scene
@property (nonatomic,weak) WhirlyKitView *theView;
/// Set this mode to modify how Z buffering is used (if at all)
@property (nonatomic,assign) WhirlyKitSceneRendererZBufferMode zBufferMode;
/// Set this to turn culling on or off.
/// By default it's on, so leave it alone unless you know you want it off.
@property (nonatomic,assign) bool doCulling;

/// The pixel width of the CAEAGLLayer.
@property (nonatomic,readonly) GLint framebufferWidth;
/// The pixel height of the CAEAGLLayer.
@property (nonatomic,readonly) GLint framebufferHeight;
/// Scale, to reflect the device's screen
@property (nonatomic,readonly) float scale;

/// Statistic: Frames per second
@property (nonatomic,assign) float framesPerSec;
/// Statistic: Number of drawables drawn in last frame
@property (nonatomic,readonly) unsigned int numDrawables;
/// Period over which we measure performance
@property (nonatomic,assign) int perfInterval;

/// Set if we're using the view based change mechanism to tell when to draw.
/// This works well for figuring out when the model matrix changes, but
///  not so well with animation such as fades, particles systems and such.
@property (nonatomic,assign) bool useViewChanged;
/// By default we'll sort all alpha-containing drawables to the end.
/// Turn this off to tell the renderer you knew what you're doing and
///  don't mess with my draw priorities.
@property (nonatomic,assign) bool sortAlphaToEnd;
// If this is set, we'll turn off the depth buffering the first time
//  we hit a drawable with alpha.  Off by default (not surprisingly).
@property (nonatomic,assign) bool depthBufferOffForAlpha;

/// Force a draw at the next opportunity
@property (nonatomic,assign) bool triggerDraw;

/// Initialize with API version
- (id) initWithOpenGLESVersion:(EAGLRenderingAPI)apiVersion;

/// Render to the screen, ideally within the given duration.
/// The subclasses fill this in
- (void)render:(NSTimeInterval)duration;

/// Rather do any rendering, just process the outstanding scene changes
- (void)processScene;

/// Called when the layer gets resized.  Need to resize ourselves
- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer;

/// Call this before defining things within the OpenGL context
- (void)useContext;

/// Set the render until time.  This is used by things like fade to keep
///  the rendering optimization from cutting off animation.
- (void)setRenderUntil:(NSTimeInterval)newTime;

// A drawable wants continuous rendering (bleah!)
- (void)addContinuousRenderRequest:(WhirlyKit::SimpleIdentity)drawID;

// Drawable is done with continuous rendering
- (void)removeContinuousRenderRequest:(WhirlyKit::SimpleIdentity)drawID;

/// Call this to force a draw on the next frame.
/// This turns off the draw optimization, but just for one frame.
- (void)forceDrawNextFrame;

/// Use this to set the clear color for the screen.  Defaults to black
- (void)setClearColor:(UIColor *)inClearColor;

/// Used by the subclasses for culling
- (void)findDrawables:(WhirlyKit::Cullable *)cullable view:(WhirlyGlobeView *)globeView frameSize:(WhirlyKit::Point2f)frameSize modelTrans:(Eigen::Matrix4d *)modelTrans eyeVec:(Eigen::Vector3f)eyeVec frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(WhirlyKit::Mbr)screenMbr topLevel:(bool)isTopLevel toDraw:(std::set<WhirlyKit::DrawableRef> *) toDraw considered:(int *)drawablesConsidered;

/// Used by the subclasses to determine if the view changed and needs to be updated
- (bool) viewDidChange;

/// Add the given rendering target and, you know, render to it
- (void) addRenderTarget:(WhirlyKit::RenderTarget &)newTarget;

/// Clear out the given render target
- (void) clearRenderTarget:(WhirlyKit::SimpleIdentity)targetID;

@end
