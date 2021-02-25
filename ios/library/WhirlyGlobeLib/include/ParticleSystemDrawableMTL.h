/*
 *  ParticleSystemDrawableMTL.h
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

#import "ParticleSystemDrawable.h"
#import "ProgramMTL.h"
#import "WrapperMTL.h"
#import "BasicDrawableMTL.h"
#import "VertexAttributeMTL.h"

namespace WhirlyKit
{

// Maximum size of particle buffers (8MB)
//#define kMaxParticleMemory (8*1024*1024)

/// Metal version of the particle system drawable
class ParticleSystemDrawableMTL : virtual public ParticleSystemDrawable, virtual public DrawableMTL
{
    friend class ParticleSystemDrawableBuilderMTL;
public:
    ParticleSystemDrawableMTL(const std::string &name);
    virtual ~ParticleSystemDrawableMTL();

    /// Add the vertex data (all of it) at once
    void addAttributeData(const RenderSetupInfo *setupInfo,const RawDataRef &data,const Batch &batch);
    
    /// Create our buffers in GL
    virtual void setupForRenderer(const RenderSetupInfo *,Scene *scene);
    
    /// Destroy GL buffers
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown);
    
    // An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
    // Also adds to the list of resources being used by this drawable
    bool preProcess(SceneRendererMTL *sceneRender,
                    id<MTLCommandBuffer> cmdBuff,
                    id<MTLBlitCommandEncoder> bltEncode,
                    SceneMTL *scene);

    /// List all the resources used by the drawable
    virtual void enumerateResources(RendererFrameInfoMTL *frameInfo,ResourceRefsMTL &resources);

    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene);

    /// Draw directly, once per frame
    virtual void encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene);
    
    /// Indirect version of calculate encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget);

    /// Indirect version of regular encoding.  Called only when things change enough to re-encode.
    API_AVAILABLE(ios(13.0))
    virtual void encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget);

protected:
    id<MTLRenderPipelineState> getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);
    void bindParticleUniforms(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode);
    
    bool setupForMTL;
    id<MTLRenderPipelineState> calcRenderState,visRenderState;
    MTLVertexDescriptor *vertDesc;
    int curPointBuffer;
    id<MTLBuffer> pointBuffer[2];
    int numRectTris;
    id<MTLBuffer> rectVertBuffer,rectTexCoordBuffer,rectTriBuffer;
};

    
}
