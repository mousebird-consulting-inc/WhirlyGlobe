/*
 *  SceneRenderer.h
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

#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Lighting.h"
#import "RenderTarget.h"

namespace WhirlyKit
{
class SceneRenderer;
class BasicDrawableBuilder;
typedef std::shared_ptr<BasicDrawableBuilder> BasicDrawableBuilderRef;
class BasicDrawableInstanceBuilder;
typedef std::shared_ptr<BasicDrawableInstanceBuilder> BasicDrawableInstanceBuilderRef;
class BillboardDrawableBuilder;
typedef std::shared_ptr<BillboardDrawableBuilder> BillboardDrawableBuilderRef;
class ParticleSystemDrawableBuilder;
typedef std::shared_ptr<ParticleSystemDrawableBuilder> ParticleSystemDrawableBuilderRef;
class ScreenSpaceDrawableBuilder;
typedef std::shared_ptr<ScreenSpaceDrawableBuilder> ScreenSpaceDrawableBuilderRef;
class WideVectorDrawableBuilder;
typedef std::shared_ptr<WideVectorDrawableBuilder> WideVectorDrawableBuilderRef;
class DynamicTexture;
typedef std::shared_ptr<DynamicTexture> DynamicTextureRef;

/** Renderer Frame Info.
 Data about the current frame, passed around by the renderer.
 */
class RendererFrameInfo
{
public:
    RendererFrameInfo();
    RendererFrameInfo(const RendererFrameInfo &that);
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Renderer itself
    SceneRenderer *sceneRenderer;
    /// View
    View *theView;
    /// Current model matrix from the view
    Eigen::Matrix4f modelTrans,viewTrans;
    Eigen::Matrix4d modelTrans4d,viewTrans4d;
    /// Current projection matrix
    Eigen::Matrix4f projMat;
    Eigen::Matrix4d projMat4d;
    /// What's currently in the GL model matrix.
    /// We combine view and model together
    Eigen::Matrix4f viewAndModelMat;
    Eigen::Matrix4d viewAndModelMat4d;
    /// The model, view, and projection matrix all rolled into one
    Eigen::Matrix4f mvpMat;
    /// Inverse of the model/view/projection matrix
    Eigen::Matrix4f mvpInvMat;
    /// Normal matrix for MVP
    Eigen::Matrix4f mvpNormalMat;
    /// Model, and view matrix but for normal transformation
    Eigen::Matrix4f viewModelNormalMat;
    /// Projection, view, and offset matrices rolled together
    Eigen::Matrix4d pvMat4d;
    Eigen::Matrix4f pvMat;
    /// If the visual view supports wrapping, these are the available offset matrices
    std::vector<Eigen::Matrix4d> offsetMatrices;
    /// Scene itself.  Don't mess with this
    Scene *scene;
    /// Expected length of the current frame
    float frameLen;
    /// Time at the start of frame
    TimeInterval currentTime;
    /// Vector pointing up from the globe describing where the view point is
    Eigen::Vector3f eyeVec;
    /// Vector out from the eye point, including tilt
    Eigen::Vector3f fullEyeVec;
    /// Position of user
    Eigen::Vector3d eyePos;
    /// Location of the middle of the screen in display coordinates
    Eigen::Vector3d dispCenter;
    /// Height above surface, if that makes sense
    float heightAboveSurface;
    /// Screen size in display coordinates
    Point2d screenSizeInDisplayCoords;
    /// Lights, if applicable
    std::vector<DirectionalLight> *lights;
    /// Program being used for this frame
    Program *program;
};

/** We support three different ways of using z buffer.  (1) Regular mode where it's on.
 (2) Completely off, priority sorting only.  (3) Priority sorting, but drawables
 are allowed to force the z buffer on temporarily.
 */
typedef enum {zBufferOn,zBufferOff,zBufferOffDefault} WhirlyKitSceneRendererZBufferMode;

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
class SceneRenderer : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SceneRenderer();
    virtual ~SceneRenderer();
    
    /// Renderer type.  Just two for now.
    typedef enum {RenderGLES,RenderMetal} Type;
    virtual Type getType() = 0;
    
    /// Set the render until time.  This is used by things like fade to keep
    ///  the rendering optimization from cutting off animation.
    virtual void setRenderUntil(TimeInterval newTime);
    
    /// A drawable wants continuous rendering (bleah!)
    virtual void addContinuousRenderRequest(SimpleIdentity drawID);
    
    /// Drawable is done with continuous rendering
    virtual void removeContinuousRenderRequest(SimpleIdentity drawID);
    
    /// Call this to force a draw on the next frame.
    /// This turns off the draw optimization, but just for one frame.
    virtual void forceDrawNextFrame();
    
    /// Return true if we have changes to process or display
    virtual bool hasChanges();
    
    /// Use this to set the clear color for the screen.  Defaults to black
    virtual void setClearColor(const RGBAColor &color);
    
    /// Return the current clear color
    RGBAColor getClearColor();
    
    /// Get the framebuffer size (in pixels)
    Point2f getFramebufferSize();
    
    /// Get the framebuffer size (divided by scale)
    Point2f getFramebufferSizeScaled();
    
    /// Return the attached Scene
    Scene *getScene();
    
    /// Return the map view
    View *getView();
    
    /// Return the device scale (e.g. retina vs. not)
    float getScale();
    
    /// Set the screen scale (can vary)
    void setScale(float newScale);
    
    /// Used by the subclasses to determine if the view changed and needs to be updated
    virtual bool viewDidChange();
    
    /// Force a draw at the next opportunity
    virtual void setTriggerDraw();
    
    /// Set the current z buffer mode
    virtual void setZBufferMode(WhirlyKitSceneRendererZBufferMode inZBufferMode);
    
    /// Assign a new scene.  Just at startup
    virtual void setScene(Scene *newScene);
    
    /// The next time through we'll redo the render setup.
    /// We might need this if the view has switched away and then back.
    virtual void forceRenderSetup();
    
    /// Set the performance counting interval (0 is off)
    virtual void setPerfInterval(int howLong);
    
    /// If set, we'll use the view changes to trigger rendering
    virtual void setUseViewChanged(bool newVal);
    
    /// Current view (opengl view) we're tied to
    virtual void setView(View *newView);
        
    /// Add a render target to start rendering too
    virtual void addRenderTarget(RenderTargetRef newTarget);
    
    /// Stop rendering to the matching render target
    virtual void removeRenderTarget(SimpleIdentity targetID);
    
    /// Called before we present the render buffer.  Can do snapshot logic here.
    virtual void snapshotCallback(TimeInterval now) = 0;
    
    /// Add a light to the existing set
    virtual void addLight(const DirectionalLight &light);
    
    /// Replace all the lights at once. nil turns off lighting
    virtual void replaceLights(const std::vector<DirectionalLight> &lights);
    
    /// Set the default material
    virtual void setDefaultMaterial(const Material &mat);
    
    /// Run the scene changes
    void processScene();
    
    /// Return the render setup info for the appropriate rendering type
    virtual const RenderSetupInfo *getRenderSetupInfo() const = 0;
    
    /// Construct a basic drawable builder for the appropriate rendering type
    virtual BasicDrawableBuilderRef makeBasicDrawableBuilder(const std::string &name) const = 0;
    
    /// Construct a basic drawables instance builder for the current rendering type
    virtual BasicDrawableInstanceBuilderRef makeBasicDrawableInstanceBuilder(const std::string &name) const = 0;
    
    /// Construct a billboard drawable builder for the current rendering type
    virtual BillboardDrawableBuilderRef makeBillboardDrawableBuilder(const std::string &name) const = 0;
    
    /// Construct a screnspace drawable builder for the current rendering type
    virtual ScreenSpaceDrawableBuilderRef makeScreenSpaceDrawableBuilder(const std::string &name) const = 0;
    
    /// Construct a particle system builder of the appropriate rendering type
    virtual ParticleSystemDrawableBuilderRef  makeParticleSystemDrawableBuilder(const std::string &name) const = 0;
    
    /// Construct a wide vector drawable builder of the appropriate rendering type
    virtual WideVectorDrawableBuilderRef makeWideVectorDrawableBuilder(const std::string &name) const = 0;
    
    /// Construct a renderer-specific render target
    virtual RenderTargetRef makeRenderTarget() const = 0;
    
    /// Construct a renderer-specific dynamic texture
    virtual DynamicTextureRef makeDynamicTexture(const std::string &name) const = 0;

    /// The pixel width of the CAEAGLLayer.
    int framebufferWidth;
    /// The pixel height of the CAEAGLLayer.
    int framebufferHeight;
    
    /// Scale, to reflect the device's screen
    float scale;

    std::vector<RenderTargetRef> renderTargets;

protected:
    // Called by the subclass
    virtual void init();
    
    // Possible post-target creation init
    virtual void defaultTargetInit(RenderTarget *);
    
    // Presentation, if required
    virtual void presentRender();
    
    /// Scene we're drawing.  This is set from outside
    Scene *scene;
    /// The view controls how we're looking at the scene
    View *theView;
    /// Set this mode to modify how Z buffering is used (if at all)
    WhirlyKitSceneRendererZBufferMode zBufferMode;
    
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
    PerformanceTimer perfTimer;
    
    /// Last time we rendered
    TimeInterval lastDraw;
    
    /// Something wants to make sure we render until at least this point.
    TimeInterval renderUntil;
    
    // The drawables that want continuous rendering on
    SimpleIDSet contRenderRequests;
    
    RGBAColor clearColor;
    
    // View state from the last render, for comparison
    Eigen::Matrix4d modelMat,viewMat,projMat;
    
    // If we're an offline renderer, the texture we're rendering into
    TextureRef framebufferTex;
    
    TimeInterval lightsLastUpdated;
    Material defaultMat;    
    std::vector<DirectionalLight> lights;
};

typedef std::shared_ptr<SceneRenderer> SceneRendererRef;
    
}
