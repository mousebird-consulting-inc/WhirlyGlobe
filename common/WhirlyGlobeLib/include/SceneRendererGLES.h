/*
 *  ESRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
 *  Copyright 2011-2019 mousebird consulting
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

#import "UtilsGLES.h"

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Lighting.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{
class SceneRendererGLES;

/** Renderer Frame Info.
 Data about the current frame, passed around by the renderer.
 */
class RendererFrameInfoGLES : public RendererFrameInfo
{
public:
    RendererFrameInfoGLES();
    RendererFrameInfoGLES(const RendererFrameInfoGLES &that);
    
    /// Renderer version (e.g. OpenGL ES 1 vs 2)
    int glesVersion;
    /// Renderer itself
    WhirlyKit::SceneRendererES *sceneRenderer;
    /// If using OpenGL ES 2.x, this is the shader
    OpenGLES2Program *program;
};

/** We support three different ways of using z buffer.  (1) Regular mode where it's on.
    (2) Completely off, priority sorting only.  (3) Priority sorting, but drawables
        are allowed to force the z buffer on temporarily.
  */
typedef enum {zBufferOn,zBufferOff,zBufferOffDefault} WhirlyKitSceneRendererZBufferMode;

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
class SceneRendererGLES : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    SceneRendererGLES();
    virtual ~SceneRendererGLES();
    
    /// Called right after the constructor
    virtual bool setup(int apiVersion,int sizeX,int sizeY);
    
    /// Resize framebuffer because something changed
    virtual bool resize(int sizeX,int sizeY);
            
    /// Set the render until time.  This is used by things like fade to keep
    ///  the rendering optimization from cutting off animation.
    void setRenderUntil(TimeInterval newTime);
    
    /// A drawable wants continuous rendering (bleah!)
    void addContinuousRenderRequest(SimpleIdentity drawID);
    
    /// Drawable is done with continuous rendering
    void removeContinuousRenderRequest(SimpleIdentity drawID);
    
    /// Call this to force a draw on the next frame.
    /// This turns off the draw optimization, but just for one frame.
    void forceDrawNextFrame();
    
    /// Return true if we have changes to process or display
    virtual bool hasChanges() { return false; }
    
    /// Use this to set the clear color for the screen.  Defaults to black
    void setClearColor(const RGBAColor &color);
    
    /// Return the current clear color
    RGBAColor getClearColor();
    
    /// Get the framebuffer size (in pixels)
    Point2f getFramebufferSize();
    
    /// Get the framebuffer size (divided by scale)
    Point2f getFramebufferSizeScaled();
    
    /// Return the attached Scene
    Scene *getScene() { return scene; }
    
    /// Return the map view
    View *getView() { return theView; }
    
    /// Return the device scale (e.g. retina vs. not)
    float getScale() { return scale; }
    
    /// Used by the subclasses to determine if the view changed and needs to be updated
    virtual bool viewDidChange();
    
    /// Force a draw at the next opportunity
    virtual void setTriggerDraw();
    
    /// Set the current z buffer mode
    virtual void setZBufferMode(WhirlyKitSceneRendererZBufferMode inZBufferMode) { zBufferMode = inZBufferMode; }
    
    /// Assign a new scene.  Just at startup
    virtual void setScene(WhirlyKit::Scene *newScene);
    
    /// Set the performance counting interval (0 is off)
    virtual void setPerfInterval(int howLong) { perfInterval = howLong; }
    
    /// If set, we'll use the view changes to trigger rendering
    virtual void setUseViewChanged(bool newVal) { useViewChanged = newVal; }
    
    /// Current view (opengl view) we're tied to
    virtual void setView(WhirlyKit::View *newView) { theView = newView; }
    
    /// If set, we'll draw one more frame than needed after updates stop
    virtual void setExtraFrameMode(bool newMode) { extraFrameMode = newMode; }
    
    /// Add a render target to start rendering too
    void addRenderTarget(RenderTargetRef newTarget);
    
    /// Stop rendering to the matching render target
    void removeRenderTarget(SimpleIdentity targetID);
    
    /// Called before we present the render buffer.  Can do snapshot logic here.
    virtual void snapshotCallback() { };

    /// Add a light to the existing set
    void addLight(const DirectionalLight &light);
    
    /// Replace all the lights at once. nil turns off lighting
    void replaceLights(const std::vector<DirectionalLight> &lights);
    
    /// Set the default material
    void setDefaultMaterial(const Material &mat);
    
    /// The next time through we'll redo the render setup.
    /// We might need this if the view has switched away and then back.
    void forceRenderSetup();
    
    virtual void setScene(Scene *inScene);
    
    void setClearColor(const RGBAColor &color);
    
    void processScene();
    
    void render(TimeInterval duration);
    
    bool hasChanges();

public:
    // Possible post-target creation init
    virtual void defaultTargetInit(RenderTarget *) { };
    
    // Presentation, if required
    virtual void presentRender() { };
    
    // OpenGL Version
    int glesVersion;
    
    /// Scene we're drawing.  This is set from outside
    WhirlyKit::Scene *scene;
    /// The view controls how we're looking at the scene
    WhirlyKit::View *theView;
    /// Set this mode to modify how Z buffering is used (if at all)
    WhirlyKitSceneRendererZBufferMode zBufferMode;
    
    /// The pixel width of the CAEAGLLayer.
    GLint framebufferWidth;
    /// The pixel height of the CAEAGLLayer.
    GLint framebufferHeight;
    /// Scale, to reflect the device's screen
    float scale;
    
    /// Statistic: Frames per second
    float framesPerSec;
    /// Statistic: Number of drawables drawn in last frame
    unsigned int numDrawables;
    /// Period over which we measure performance
    int perfInterval;
    
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
    
    /// Force a draw at the next opportunity
    bool triggerDraw;

	unsigned int frameCount;
	TimeInterval frameCountStart;
    WhirlyKit::PerformanceTimer perfTimer;
        
    /// Last time we rendered
    TimeInterval lastDraw;
    
    /// Something wants to make sure we render until at least this point.
    TimeInterval renderUntil;
    
    // The drawables that want continuous rendering on
    WhirlyKit::SimpleIDSet contRenderRequests;
    
    WhirlyKit::RGBAColor clearColor;

    // View state from the last render, for comparison
    Eigen::Matrix4d modelMat,viewMat,projMat;
    
    // If set we draw one extra frame after updates stop
    bool extraFrameMode;
    
    std::vector<RenderTargetRef> renderTargets;

    // If we're an offline renderer, the texture we're rendering into
    WhirlyKit::Texture *framebufferTex;
    
    TimeInterval lightsLastUpdated;
    Material defaultMat;
    
    bool extraFrameDrawn;
    std::vector<DirectionalLight> lights;
};
    
typedef std::shared_ptr<SceneRendererES> SceneRendererESRef;

}
