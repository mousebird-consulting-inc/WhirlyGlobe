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
#import <MetalKit/MetalKit.h>
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{

ParticleSystemDrawableMTL::ParticleSystemDrawableMTL(const std::string &name)
    : ParticleSystemDrawable(name), Drawable(name), setupForMTL(false), calcRenderState(nil), visRenderState(nil),
    vertDesc(nil), curPointBuffer(0), rectVertBuffer(nil), rectTriBuffer(nil), numRectTris(0)
{
    pointBuffer[0] = nil;
    pointBuffer[1] = nil;
}

ParticleSystemDrawableMTL::~ParticleSystemDrawableMTL()
{
}

void ParticleSystemDrawableMTL::addAttributeData(const RenderSetupInfo *setupInfo,
                                                 const RawDataRef &data,
                                                 const Batch &batch)
{
    size_t offset = batch.batchID*vertexSize*batchSize;
    size_t size = vertexSize*batchSize;
    
    if (size != data->getLen()) {
        NSLog(@"Batch size doesn't match in addAttributeData()");
        return;
    }
    
    if (pointBuffer[0] != nil) {
        memcpy((unsigned char *)[pointBuffer[0] contents] + offset, data->getRawData(), size);
    }
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
    if (calculateProgramId != EmptyIdentity)
        pointBuffer[1] = [setupInfo->mtlDevice newBufferWithLength:len options:MTLStorageModeShared];
    for (unsigned int ii=0;ii<2;ii++) {
        if (pointBuffer[ii]) {
            memset([pointBuffer[ii] contents], 0, len);
            [pointBuffer[ii] setLabel:[NSString stringWithFormat:@"%s particle buffer %d",name.c_str(),(int)ii]];
        }
    }
    
    if (useRectangles) {
        // Make up a simple rectangle to feed into the instancing
        // Note: Should interleave these
        Point2f verts[4];
        verts[0] = Point2f(-1,-1);
        verts[1] = Point2f(1,-1);
        verts[2] = Point2f(1,1);
        verts[3] = Point2f(-1,1);
        Point2f texCoords[4];
        texCoords[0] = Point2f(0.0,0.0);
        texCoords[1] = Point2f(1.0,0.0);
        texCoords[2] = Point2f(1.0,1.0);
        texCoords[3] = Point2f(0.0,1.0);
        uint16_t idx[6];
        idx[0] = 0; idx[1] = 1; idx[2] = 2;
        idx[3] = 0; idx[4] = 2; idx[5] = 3;
        numRectTris = 2;
        
        rectVertBuffer = [setupInfo->mtlDevice newBufferWithBytes:&verts[0] length:sizeof(Point2f)*4 options:MTLStorageModeShared];
        rectTexCoordBuffer = [setupInfo->mtlDevice newBufferWithBytes:&texCoords[0] length:sizeof(Point2f)*4 options:MTLStorageModeShared];
        rectTriBuffer = [setupInfo->mtlDevice newBufferWithBytes:&idx[0] length:sizeof(uint16_t)*numRectTris*3 options:MTLStorageModeShared];
    }

    setupForMTL = true;
}

void ParticleSystemDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;

    calcRenderState = nil;
    visRenderState = nil;
    scene->releaseBuffer(pointBuffer[0]);
    scene->releaseBuffer(pointBuffer[1]);
    pointBuffer[0] = nil;
    pointBuffer[1] = nil;
    scene->releaseBuffer(rectVertBuffer);
    rectVertBuffer = nil;
    scene->releaseBuffer(rectTexCoordBuffer);
    rectTexCoordBuffer = nil;
    scene->releaseBuffer(rectTriBuffer);
    rectTriBuffer = nil;
}
    
// Render state for calculation pass
id<MTLRenderPipelineState> ParticleSystemDrawableMTL::getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    if (calcRenderState)
        return calcRenderState;
    
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    // Note: Disable this to debug the shader
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
    
void ParticleSystemDrawableMTL::bindParticleUniforms(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode)
{
    // Uniforms just for this particle drawable
    WhirlyKitShader::UniformDrawStateParticle uniPart;
    uniPart.pointSize = pointSize;
    uniPart.time = frameInfo->currentTime-baseTime;
    uniPart.lifetime = lifetime;
    uniPart.frameLen = frameInfo->frameLen;
    [cmdEncode setVertexBytes:&uniPart length:sizeof(uniPart) atIndex:WKSUniformDrawStateParticleBuffer];
    [cmdEncode setFragmentBytes:&uniPart length:sizeof(uniPart) atIndex:WKSUniformDrawStateParticleBuffer];
}

void ParticleSystemDrawableMTL::calculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
    // Render state is pretty simple, so apply that
    id<MTLRenderPipelineState> renderState = getCalcRenderPipelineState(sceneRender,frameInfo);
    [cmdEncode setRenderPipelineState:renderState];

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
            [cmdEncode setVertexTexture:tex->getMTLID() atIndex:texIndex];
            [cmdEncode setFragmentTexture:tex->getMTLID() atIndex:texIndex];
            numTextures++;
        } else {
//            [frameInfo->cmdEncode setVertexTexture:nil atIndex:texIndex];
//            [frameInfo->cmdEncode setFragmentTexture:nil atIndex:texIndex];
        }
        texIndex++;
    }
    
    // Uniforms we pass in for all particles
    bindParticleUniforms(frameInfo,cmdEncode);
    
    // Note: Do we want UniformDrawStateA here too?

    // Send along the uniform blocks
    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks, cmdEncode);

    // Switch between buffers, one input & one output
    [cmdEncode setVertexBuffer:pointBuffer[curPointBuffer] offset:0 atIndex:WKSParticleBuffer];
    [cmdEncode setVertexBuffer:pointBuffer[curPointBuffer == 0 ? 1 : 0] offset:0 atIndex:WKSParticleBuffer+1];
    curPointBuffer = (curPointBuffer == 0) ? 1 : 0;

    [cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:numTotalPoints];
}

void ParticleSystemDrawableMTL::draw(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    
    // Render state is pretty simple, so apply that
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo);
    [cmdEncode setRenderPipelineState:renderState];
    
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
            [cmdEncode setVertexTexture:tex->getMTLID() atIndex:texIndex];
            [cmdEncode setFragmentTexture:tex->getMTLID() atIndex:texIndex];
            numTextures++;
        } else {
            NSLog(@"Missing texture in particle system.  Skipping.");
            return;
        }
        texIndex++;
    }
    
    // Note: Do we want UniformDrawStateA here too?

    // Uniforms we pass in for all particles
    bindParticleUniforms(frameInfo,cmdEncode);

    // Send along the uniform blocks
    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks, cmdEncode);

    if (useRectangles) {
        [cmdEncode setVertexBuffer:rectVertBuffer offset:0 atIndex:WKSVertexPositionAttribute];
        [cmdEncode setVertexBuffer:rectTexCoordBuffer offset:0 atIndex:WKSVertexTextureBaseAttribute];
        [cmdEncode setVertexBuffer:pointBuffer[curPointBuffer] offset:0 atIndex:WKSParticleBuffer];
        [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numRectTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:rectTriBuffer indexBufferOffset:0 instanceCount:numTotalPoints];
    } else {
        [cmdEncode setVertexBuffer:pointBuffer[curPointBuffer] offset:0 atIndex:WKSParticleBuffer];
        [cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:numTotalPoints];
    }
}

}
