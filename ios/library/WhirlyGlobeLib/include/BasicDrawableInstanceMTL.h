/*
 *  BasicDrawableInstanceMTL.h
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

#import "BasicDrawableInstance.h"
#import "BasicDrawableMTL.h"
#import "WrapperMTL.h"

namespace WhirlyKit
{
    
/// Metal variant of BasicDrawableInstance
class BasicDrawableInstanceMTL : virtual public BasicDrawableInstance, virtual public DrawableMTL
{
    friend class BasicDrawableInstanceBuilderMTL;
public:
    BasicDrawableInstanceMTL(const std::string &name);
    virtual ~BasicDrawableInstanceMTL();
    
    // Color can change after setup
    virtual void setColor(RGBAColor inColor);
    
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene);
    
    /// We use the calculation step to set up indirect rendering when we're doing that
    virtual void calculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> frameEncode,Scene *scene);
    
    /// Fill this in to draw the basic drawable
    virtual void draw(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene);
    
protected:
    id<MTLRenderPipelineState> getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo,BasicDrawableMTL *basicDrawMTL);
    id<MTLRenderPipelineState> getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo);
    void updateColorDefaultAttr();
    
    id<MTLRenderPipelineState> renderState;
    id<MTLRenderPipelineState> calcRenderState;
    std::vector<BasicDrawableMTL::AttributeDefault> defaultAttrs;
    bool setupForMTL;
    WhirlyKitShader::UniformModelInstance uniMI;
    id<MTLBuffer> instBuffer;  // Stores instances
    id<MTLBuffer> indirectBuffer;   // Indirect arguments for drawIndexed
    int numInst;
};
    
}
