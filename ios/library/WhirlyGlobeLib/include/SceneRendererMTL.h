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
#import "Snapshot_iOS.h"

/// Caller to render() fills this in so we don't allocate the drawable
///  until it's needed.
@protocol SceneRendererMTLDrawableGetter

/// Return a CAMetalDrawable when requested
- (id<CAMetalDrawable>) getDrawable;

@end

namespace WhirlyKit
{

class RenderTargetMTL;
typedef std::shared_ptr<RenderTargetMTL> RenderTargetMTLRef;

/// Metal stores a bit more per-frame information
class RendererFrameInfoMTL : public RendererFrameInfo
{
public:
    RendererFrameInfoMTL();
    RendererFrameInfoMTL(const RendererFrameInfoMTL &that);

    // Render pass descriptor from the view
    MTLRenderPassDescriptor *renderPassDesc;
    // Current render target
    RenderTargetMTL *renderTarget;
};

/// Copy one of our matrices into Metal format
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,Eigen::Matrix4f &src);
void CopyIntoMtlFloat4x4(simd::float4x4 &dest,Eigen::Matrix4d &src);

/// Copy one of our points into Metal form
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3d &src);

/// Copy one of our points into Metal form
void CopyIntoMtlFloat3(simd::float3 &dest,const Point3f &src);

/// Copy one of our 4D points into Metal form
void CopyIntoMtlFloat4(simd::float4 &dest,const Eigen::Vector4f &src);
void CopyIntoMtlFloat4(simd::float4 &dest,const float vals[4]);
    
/// Metal version of the Scene Renderer
class SceneRendererMTL : public SceneRenderer
{
public:
    SceneRendererMTL(id<MTLDevice> mtlDevice,float scale);
    virtual ~SceneRendererMTL();
    
    // Metal (obviously)
    virtual Type getType();
    
    // Various information about the renderer passed around to call
    virtual const RenderSetupInfo *getRenderSetupInfo() const;
    
    virtual void setView(View *newView);
    virtual void setScene(Scene *newScene);
    
    /// Called right after the constructor
    bool setup(int sizeX,int sizeY,bool offscreen);
    
    /// Resize framebuffer because something changed
    virtual bool resize(int sizeX,int sizeY);
    
    /// Draw stuff (the whole point!)
    void render(TimeInterval period,MTLRenderPassDescriptor *renderPassDesc,id<SceneRendererMTLDrawableGetter> drawGetter);
    
    /// Set the clear color we're using
    virtual void setClearColor(const RGBAColor &color);
        
    /// Want a snapshot, set up this delegate
    void addSnapshotDelegate(NSObject<WhirlyKitSnapshot> *);
    
    /// Remove an existing snapshot delegate
    void removeSnapshotDelegate(NSObject<WhirlyKitSnapshot> *);

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
    void setupUniformBuffer(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,CoordSystemDisplayAdapter *coordAdapter,int texLevel);

    /// Set the lights and tie them to a vertex buffer index
    void setupLightBuffer(SceneMTL *scene,id<MTLRenderCommandEncoder> cmdEncode);
    
    // Apply the various defaults to DrawStateA
    void setupDrawStateA(WhirlyKitShader::UniformDrawStateA &drawState,RendererFrameInfoMTL *frameInfo);
    
    // Generate a render pipeline descriptor matching the given frame
    MTLRenderPipelineDescriptor *defaultRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);
    
    // Return the whole buffer for a given render target
    RawDataRef getSnapshot(SimpleIdentity renderTargetID);
    
    // Return data values at a single pixel for the given render target
    RawDataRef getSnapshotAt(SimpleIdentity renderTargetID,int x,int y);
    
    // Return the min/max values (assuming that option is on) for a render target
    RawDataRef getSnapshotMinMax(SimpleIdentity renderTargetID);
    
public:
    RenderTargetMTLRef getRenderTarget(SimpleIdentity renderTargetID);
    
    // By default offscreen rendering turns on or off blend enable
    bool offscreenBlendEnable;
    // Information about the renderer passed around to various calls
    RenderSetupInfoMTL setupInfo;
    std::vector<NSObject<WhirlyKitSnapshot> *> snapshotDelegates;
};
    
typedef std::shared_ptr<SceneRendererMTL> SceneRendererMTLRef;

}
