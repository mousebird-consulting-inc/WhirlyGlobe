/*
 *  SceneRendererMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "SceneRenderer.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "Scene.h"
#import "PerformanceTimer.h"
#import "Lighting.h"
#import "SceneRenderer.h"
#import "WrapperMTL.h"
#import "ProgramMTL.h"
#import "SceneMTL.h"
#import <MetalKit/MetalKit.h>
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{
    
/// Metal stores a bit more per-frame information
class RendererFrameInfoMTL : public RendererFrameInfo
{
public:
    RendererFrameInfoMTL();
    RendererFrameInfoMTL(const RendererFrameInfoMTL &that);

    // Command encoder for just this frame
    id<MTLRenderCommandEncoder> cmdEncode;
    // Render pass descriptor from the view
    MTLRenderPassDescriptor *renderPassDesc;
};

/// Metal version of the Scene Renderer
class SceneRendererMTL : public SceneRenderer
{
public:
    SceneRendererMTL(id<MTLDevice> mtlDevice);
    virtual ~SceneRendererMTL();
    
    // Metal (obviously)
    virtual Type getType();
    
    // Various information about the renderer passed around to call
    virtual const RenderSetupInfo *getRenderSetupInfo() const;
    
    virtual void setView(View *newView);
    virtual void setScene(Scene *newScene);
    
    /// Called right after the constructor
    bool setup(int sizeX,int sizeY);
    
    /// Resize framebuffer because something changed
    virtual bool resize(int sizeX,int sizeY);
    
    /// Draw stuff (the whole point!)
    void render(TimeInterval period,MTLRenderPassDescriptor *renderPassDesc,id<CAMetalDrawable> drawable);
    
    /// Run a snapshot and callback the registered routine
    virtual void snapshotCallback(TimeInterval now);
    
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
    
    /// Set up the buffer for general uniforms and attach it to its vertex/fragment buffers
    void setupUniformBuffer(RendererFrameInfoMTL *frameInfo);

    /// Set the lights and tie them to a vertex buffer index
    void setupLightBuffer(SceneMTL *scene,id<MTLRenderCommandEncoder> cmdEncode);
    
    // Apply the various defaults to DrawStateA
    void setupDrawStateA(WhirlyKitShader::UniformDrawStateA &drawState,RendererFrameInfoMTL *frameInfo);
    
public:
    // Information about the renderer passed around to various calls
    RenderSetupInfoMTL setupInfo;
};
    
typedef std::shared_ptr<SceneRendererMTL> SceneRendererMTLRef;

}
