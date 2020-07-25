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
#import "ProgramGLES.h"
#import "MemManagerGLES.h"

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
};

/// Base class for the scene renderer.
/// It's subclassed for the specific version of OpenGL ES
class SceneRendererGLES : public SceneRenderer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    SceneRendererGLES();
    virtual ~SceneRendererGLES();
    
    // GL (obviously)
    virtual Type getType();

    // Various information about the renderer passed around to call
    virtual const RenderSetupInfo *getRenderSetupInfo() const;
    
    virtual void setView(View *newView);
    virtual void setScene(Scene *newScene);

    /// Called right after the constructor
    virtual bool setup(int apiVersion,int sizeX,int sizeY,float scale);
    
    /// Resize framebuffer because something changed
    virtual bool resize(int sizeX,int sizeY);

    /// Return true if we have changes to process or display
    virtual bool hasChanges();

    /// If set, we'll draw one more frame than needed after updates stop
    virtual void setExtraFrameMode(bool newMode);
    
    /// Draw stuff (the whole point!)
    void render(TimeInterval period);
    
    /// Construct a basic drawable builder for the appropriate rendering type
    virtual BasicDrawableBuilderRef makeBasicDrawableBuilder(const std::string &name) const;
    
    /// Construct a basic drawables instance builder for the current rendering type
    virtual BasicDrawableInstanceBuilderRef makeBasicDrawableInstanceBuilder(const std::string &name) const;
    
    /// Construct a billboard drawable builder for the current rendering type
    virtual BillboardDrawableBuilderRef makeBillboardDrawableBuilder(const std::string &name) const;
    
    /// Construct a screnspace drawable builder for the current rendering type
    virtual ScreenSpaceDrawableBuilderRef makeScreenSpaceDrawableBuilder(const std::string &name) const;
    
    /// Construct a particle system builder of the appropriate rendering type
    virtual ParticleSystemDrawableBuilderRef  makeParticleSystemDrawableBuilder(const std::string &name) const;
    
    /// Construct a wide vector drawable builder of the appropriate rendering type
    virtual WideVectorDrawableBuilderRef makeWideVectorDrawableBuilder(const std::string &name) const;
    
    /// Construct a renderer-specific render target
    virtual RenderTargetRef makeRenderTarget() const;
    
    /// Construct a renderer-specific dynamic texture
    virtual DynamicTextureRef makeDynamicTexture(const std::string &name) const;

    /** Return the snapshot for the given render target.
     *  EmptyIdentity refers to the whole
     *  width <= 0 means the whole screen.
     */
    virtual RawDataRef getSnapshotAt(SimpleIdentity renderTargetID, int x, int y, int width, int height);
    
public:
    // Possible post-target creation init
    virtual void defaultTargetInit(RenderTarget *) { };
    
    // Presentation, if required
    virtual void presentRender() { };
    
    // Information about the renderer passed around to various calls
    RenderSetupInfoGLES setupInfo;
    
    // If set we draw one extra frame after updates stop
    bool extraFrameMode;
    int extraFrameCount;
};
    
typedef std::shared_ptr<SceneRendererGLES> SceneRendererGLESRef;

}
