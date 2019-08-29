/*
 *  ParticleSystemDrawableMTL.mm
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

#import "ParticleSystemDrawableMTL.h"
#import "TextureMTL.h"

namespace WhirlyKit
{

ParticleSystemDrawableMTL::ParticleSystemDrawableMTL(const std::string &name)
    : ParticleSystemDrawable(name), setupForMTL(false), renderState(nil),
    vertDesc(nil), pointBuffer(nil)
{
}

void ParticleSystemDrawableMTL::addAttributeData(const RenderSetupInfo *setupInfo,
                                                 const std::vector<AttributeData> &attrData,
                                                 const Batch &batch)
{
    // No attributes for Metal version.  Just memory.
}

void ParticleSystemDrawableMTL::setupForRenderer(const RenderSetupInfo *inSetupInfo)
{
    if (setupForMTL)
        return;

    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;

    // Set up a single buffer to calculate and then render into
    int len = numTotalPoints * vertexSize;
    pointBuffer = [setupInfo->mtlDevice newBufferWithLength:len options:MTLStorageModeShared];
    [pointBuffer setLabel:[NSString stringWithFormat:@"%s particle buffer",name.c_str()]];

    setupForMTL = true;
}

void ParticleSystemDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
    renderState = nil;
    pointBuffer = nil;
}
    
id<MTLRenderPipelineState> ParticleSystemDrawableMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    if (renderState)
        return renderState;
    
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.rasterizationEnabled = false;
    renderDesc.vertexDescriptor = nil;
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];
    
    // Set up a render state
    NSError *err = nil;
    renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }
    
    return renderState;
}

void ParticleSystemDrawableMTL::calculate(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
    // Render state is pretty simple, so apply that
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo);
    [frameInfo->cmdEncode setRenderPipelineState:renderState];

    // Pass in the textures (and offsets)
    // Note: We could precalculate most of then when the texture changes
    //       And we should figure out how many textures they actually have
    int numTextures = 0,texIndex = 0;
    for (auto texID : texIDs) {
        // And the texture itself
        // Note: Should we be setting up the sampler?
        TextureBaseMTL *tex = NULL;
        if (texID != EmptyIdentity)
            tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(texID));
        if (tex) {
            [frameInfo->cmdEncode setVertexTexture:tex->getMTLID() atIndex:texIndex];
            [frameInfo->cmdEncode setFragmentTexture:tex->getMTLID() atIndex:texIndex];
            numTextures++;
        } else {
            [frameInfo->cmdEncode setVertexTexture:nil atIndex:texIndex];
            [frameInfo->cmdEncode setFragmentTexture:nil atIndex:texIndex];
        }
        texIndex++;
    }

    // Note: Do we want UniforDrawStateA here too?

    // Send along the uniform blocks
    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks);

    // Note: Make this buffer configurable?
    [frameInfo->cmdEncode setVertexBuffer:pointBuffer offset:0 atIndex:0];

    [frameInfo->cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:numTotalPoints];
}

void ParticleSystemDrawableMTL::draw(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
}

}
