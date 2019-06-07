/*
 *  BasicDrawableInstanceMTL.mm
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

#import "BasicDrawableInstanceMTL.h"
#import "ProgramMTL.h"
#import "SceneRendererMTL.h"
#import "DefaultShadersMTL.h"
#import "TextureMTL.h"

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawableInstanceMTL::BasicDrawableInstanceMTL(const std::string &name)
    : BasicDrawableInstance(name), vertDesc(nil), renderState(nil)
{
}

void BasicDrawableInstanceMTL::setupForRenderer(const RenderSetupInfo *setupInfo)
{
}

void BasicDrawableInstanceMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
}

void BasicDrawableInstanceMTL::draw(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    BasicDrawableMTL *basicDrawMTL = (BasicDrawableMTL *)basicDraw.get();
    if (!basicDrawMTL)
        return;

    switch (instanceStyle)
    {
        case ReuseStyle:
            drawReuse(frameInfo,sceneRender,scene,basicDrawMTL);
            break;
        case LocalStyle:
            drawLocal(frameInfo,sceneRender,scene,basicDrawMTL);
            break;
    }
}
    
void BasicDrawableInstanceMTL::drawReuse(RendererFrameInfoMTL *frameInfo,SceneRendererMTL *sceneRender,SceneMTL *scene,BasicDrawableMTL *basicDrawMTL)
{
    // TODO: Implement
}
    
    
id<MTLRenderPipelineState> BasicDrawableInstanceMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo,BasicDrawableMTL *basicDrawMTL)
{
    if (renderState)
        return renderState;
    
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.vertexDescriptor = basicDrawMTL->getVertexDescriptor(program->vertFunc,defaultAttrs);

    // Set up a render state
    NSError *err = nil;
    renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableInstanceMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }
    
    return renderState;
}

    
void BasicDrawableInstanceMTL::drawLocal(RendererFrameInfoMTL *frameInfo,SceneRendererMTL *sceneRender,SceneMTL *scene,BasicDrawableMTL *basicDrawMTL)
{
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo,basicDrawMTL);
    
    // Wire up the various inputs that we know about
    // TODO: Some of these we need to override
    for (auto vertAttr : basicDrawMTL->vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer)
            [frameInfo->cmdEncode setVertexBuffer:vertAttrMTL->buffer offset:0 atIndex:vertAttrMTL->bufferIndex];
    }
    
    // And provide defaults for the ones we don't
    for (auto defAttr : defaultAttrs)
        [frameInfo->cmdEncode setVertexBytes:&defAttr.data length:sizeof(defAttr.data) atIndex:defAttr.bufferIndex];
    
    [frameInfo->cmdEncode setRenderPipelineState:renderState];
    
    // TODO: Fill this in
    float fade = 1.0;
    
    // Pass in the textures (and offsets)
    // Note: We could precalculate most of then when the texture changes
    //       And we should figure out how many textures there actually have
    int numTextures = 0;
    for (unsigned int texIndex=0;texIndex<std::max((int)texInfo.size(),2);texIndex++) {
        TexInfo *thisTexInfo = (texIndex < texInfo.size()) ? &texInfo[texIndex] : NULL;
        
        // Figure out texture adjustment for parent textures
        float texScale = 1.0;
        Vector2f texOffset(0.0,0.0);
        // Adjust for border pixels
        if (thisTexInfo && thisTexInfo->borderTexel > 0 && thisTexInfo->size > 0) {
            texScale = (thisTexInfo->size - 2 * thisTexInfo->borderTexel) / (double)thisTexInfo->size;
            float offset = thisTexInfo->borderTexel / (double)thisTexInfo->size;
            texOffset = Vector2f(offset,offset);
        }
        // Adjust for a relative texture lookup (using lower zoom levels)
        if (thisTexInfo && thisTexInfo->relLevel > 0) {
            texScale = texScale/(1<<thisTexInfo->relLevel);
            texOffset = Vector2f(texScale*thisTexInfo->relX,texScale*thisTexInfo->relY) + texOffset;
        }
        
        // Calculate offset and scales
        WhirlyKitShader::TexIndirect texInd;
        texInd.offset[0] = texOffset.x();  texInd.offset[1] = texOffset.y();
        texInd.scale[0] = texScale; texInd.scale[1] = texScale;
        
        [frameInfo->cmdEncode setVertexBytes:&texInd length:sizeof(texInd) atIndex:WKSTexIndirectStartBuffer+texIndex];
        
        // And the texture itself
        // Note: Should we be setting up the sampler?
        TextureBaseMTL *tex = NULL;
        if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
            tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
        if (tex) {
            [frameInfo->cmdEncode setVertexTexture:tex->getMTLID() atIndex:texIndex];
            [frameInfo->cmdEncode setFragmentTexture:tex->getMTLID() atIndex:texIndex];
            numTextures++;
        } else {
            [frameInfo->cmdEncode setVertexTexture:nil atIndex:texIndex];
            [frameInfo->cmdEncode setFragmentTexture:nil atIndex:texIndex];
        }
    }
    
    // Set the per-drawable draw state
    WhirlyKitShader::UniformDrawStateA uni;
    sceneRender->setupDrawStateA(uni,frameInfo);
    uni.numTextures = numTextures;
    uni.fade = fade;
    uni.clipCoords = basicDrawMTL->clipCoords;
    BasicDrawableMTL::applyUniformsToDrawState(uni, uniforms);
    // TODO: Fill in the override matrix
    bzero(&uni.singleMat,sizeof(uni.singleMat));
    [frameInfo->cmdEncode setVertexBytes:&uni length:sizeof(uni) atIndex:WKSUniformDrawStateBuffer];
    [frameInfo->cmdEncode setFragmentBytes:&uni length:sizeof(uni) atIndex:WKSUniformDrawStateBuffer];

    
    // Render the primitives themselves
    switch (basicDraw->type) {
        case Lines:
            [frameInfo->cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:basicDrawMTL->numPts];
            break;
        case Triangles:
            if (!basicDrawMTL->triBuffer) {
                // TODO: Figure out why this happens
//                NSLog(@"BasicDrawableInstanceMTL: Bad basic drawable with no triangles.");
                return;
            }
            // This actually draws the triangles (well, in a bit)
            [frameInfo->cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:basicDrawMTL->numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:basicDrawMTL->triBuffer indexBufferOffset:0];
            break;
        default:
            break;
    }

}

}
