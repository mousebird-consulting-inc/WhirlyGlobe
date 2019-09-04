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
class ParticleSystemDrawableMTL : public ParticleSystemDrawable
{
    friend class ParticleSystemDrawableBuilderMTL;
public:
    ParticleSystemDrawableMTL(const std::string &name);
    
    /// Add the vertex data (all of it) at once
    void addAttributeData(const RenderSetupInfo *setupInfo,const std::vector<AttributeData> &attrData,const Batch &batch);
    
    /// Create our buffers in GL
    virtual void setupForRenderer(const RenderSetupInfo *);
    
    /// Destroy GL buffers
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Particles can calculate their positions
    void calculate(RendererFrameInfo *frameInfo,Scene *scene);
    
    /// Called on the rendering thread to draw
    void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
protected:
    id<MTLRenderPipelineState> getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);

    bool setupForMTL;
    id<MTLRenderPipelineState> calcRenderState,visRenderState;
    MTLVertexDescriptor *vertDesc;
    int curPointBuffer;
    id<MTLBuffer> pointBuffer[2];
    int numRectTris;
    id<MTLBuffer> rectVertBuffer,rectTriBuffer;
};

    
}
