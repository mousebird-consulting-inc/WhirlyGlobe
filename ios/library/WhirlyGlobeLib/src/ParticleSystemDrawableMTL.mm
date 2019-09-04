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
    : ParticleSystemDrawable(name), setupForMTL(false), calcRenderState(nil), visRenderState(nil),
    vertDesc(nil), curPointBuffer(0), rectBuffer(nil)
{
    pointBuffer[0] = nil;
    pointBuffer[1] = nil;
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

    // Set up a particle buffers to read from and render to
    // Note: Not clear we really need two, but it simplifies debugging
    int len = numTotalPoints * vertexSize;
    curPointBuffer = 0;
    pointBuffer[0] = [setupInfo->mtlDevice newBufferWithLength:len options:MTLStorageModeShared];
    pointBuffer[1] = [setupInfo->mtlDevice newBufferWithLength:len options:MTLStorageModeShared];
    for (unsigned int ii=0;ii<2;ii++) {
        memset([pointBuffer[ii] contents], 0, len);
        [pointBuffer[ii] setLabel:[NSString stringWithFormat:@"%s particle buffer %d",name.c_str(),(int)ii]];
    }
    
    if (useRectangles) {
        // Make up a simple rectangle to feed into the instancing
        Point2f verts[2*6];
        verts[0] = Point2f(-1,-1);
        verts[1] = Point2f(0,0);
        verts[2] = Point2f(1,-1);
        verts[3] = Point2f(1.0,0);
        verts[4] = Point2f(1,1);
        verts[5] = Point2f(1.0,1.0);
        verts[6] = Point2f(-1,-1);
        verts[7] = Point2f(0,0);
        verts[8] = Point2f(1,1);
        verts[9] = Point2f(1.0,1.0);
        verts[10] = Point2f(-1,1);
        verts[11] = Point2f(0,1.0);
        
        rectBuffer = [setupInfo->mtlDevice newBufferWithBytes:&verts[0] length:sizeof(Point2f)*12 options:MTLStorageModeShared];
    }

    setupForMTL = true;
}

void ParticleSystemDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
    calcRenderState = nil;
    visRenderState = nil;
    pointBuffer[0] = nil;
    pointBuffer[1] = nil;
    rectBuffer = nil;
}
    
// Render state for calculation pass
id<MTLRenderPipelineState> ParticleSystemDrawableMTL::getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    if (calcRenderState)
        return calcRenderState;
    
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.rasterizationEnabled = false;
    renderDesc.vertexDescriptor = nil;
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];
    
    // Set up a render state
    NSError *err = nil;
    calcRenderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }
    
    return calcRenderState;
}
    
// Render state for particle rendering
id<MTLRenderPipelineState> ParticleSystemDrawableMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    if (visRenderState)
        return visRenderState;
    
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.vertexDescriptor = nil;
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];
    
    // Set up a render state
    NSError *err = nil;
    visRenderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }
    
    return visRenderState;
}

void ParticleSystemDrawableMTL::calculate(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
    // Render state is pretty simple, so apply that
    id<MTLRenderPipelineState> renderState = getCalcRenderPipelineState(sceneRender,frameInfo);
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
    
    // Uniforms just for this particle drawable
    WhirlyKitShader::UniformDrawStateParticle uniPart;
    uniPart.pointSize = pointSize;
    uniPart.time = frameInfo->currentTime-baseTime;
    uniPart.lifetime = lifetime;
    uniPart.frameLen = frameInfo->frameLen;
    [frameInfo->cmdEncode setVertexBytes:&uniPart length:sizeof(uniPart) atIndex:WKSUniformDrawStateParticleBuffer];
    [frameInfo->cmdEncode setFragmentBytes:&uniPart length:sizeof(uniPart) atIndex:WKSUniformDrawStateParticleBuffer];

    // Note: Do we want UniformDrawStateA here too?

    // Send along the uniform blocks
    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks);

    // Switch between buffers, one input & one output
    [frameInfo->cmdEncode setVertexBuffer:pointBuffer[curPointBuffer] offset:0 atIndex:0];
    [frameInfo->cmdEncode setVertexBuffer:pointBuffer[curPointBuffer == 0 ? 1 : 0] offset:0 atIndex:1];
    curPointBuffer = (curPointBuffer == 0) ? 1 : 0;

    [frameInfo->cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:numTotalPoints];
}

void ParticleSystemDrawableMTL::draw(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
    // Render state is pretty simple, so apply that
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo);
    [frameInfo->cmdEncode setRenderPipelineState:renderState];
    
    // Pass in the textures (and offsets)
    // TODO: Call out textures we need for calculation vs. rendering
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
    
    // Note: Do we want UniformDrawStateA here too?
    
    // Send along the uniform blocks
    // TODO: Differentiate between the ones we want for each shader
    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks);

//    if (useRectangles) {
//        // TODO: Use rectangles
//    } else {
        [frameInfo->cmdEncode setVertexBuffer:pointBuffer[curPointBuffer] offset:0 atIndex:0];
        [frameInfo->cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:numTotalPoints];
//    }
}

}
