/*
 *  BasicDrawableMTL.mm
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

#import "BasicDrawableMTL.h"
#import "ProgramMTL.h"
#import "SceneRendererMTL.h"
#import "VertexAttributeMTL.h"
#import "TextureMTL.h"
#import "SceneMTL.h"
#import "DefaultShadersMTL.h"

using namespace Eigen;

namespace WhirlyKit
{
    
BasicDrawableMTL::BasicDrawableMTL(const std::string &name)
    : BasicDrawable(name), Drawable(name), triBuffer(nil), setupForMTL(false), vertDesc(nil), renderState(nil), numPts(0), numTris(0)
{
}

VertexAttributeMTL *BasicDrawableMTL::findVertexAttribute(int nameID)
{
    VertexAttributeMTL *foundVertAttr = NULL;
    for (auto vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->nameID == nameID) {
            foundVertAttr = vertAttrMTL;
            break;
        }
    }
    
    return foundVertAttr;
}
 
// Create a buffer per vertex attribute
void BasicDrawableMTL::setupForRenderer(const RenderSetupInfo *inSetupInfo)
{
    if (setupForMTL)
        return;
    
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    
    // Set up the buffers for each vertex attribute
    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        
        int bufferSize = vertAttrMTL->sizeMTL() * vertAttrMTL->numElements();
        if (bufferSize > 0) {
            numPts = vertAttrMTL->numElements();
            vertAttrMTL->buffer = [setupInfo->mtlDevice newBufferWithBytes:vertAttr->addressForElement(0) length:bufferSize options:MTLStorageModeShared];
            if (!name.empty())
                [vertAttrMTL->buffer setLabel:[NSString stringWithFormat:@"%s vert attr",name.c_str()]];
            vertAttrMTL->clear();
        }
    }
    
    // And put the triangles in their own
    // Note: Could use 1 byte some of the time
    int bufferSize = 3*2*tris.size();
    numTris = tris.size();
    if (bufferSize > 0) {
        triBuffer = [setupInfo->mtlDevice newBufferWithBytes:&tris[0] length:bufferSize options:MTLStorageModeShared];
        if (!name.empty())
            [triBuffer setLabel:[NSString stringWithFormat:@"%s tri buffer",name.c_str()]];
        tris.clear();
    }
    
    setupForMTL = true;
}

void BasicDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;
    setupForMTL = false;
    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        scene->releaseBuffer(vertAttrMTL->buffer);
        vertAttrMTL->buffer = nil;
    }
    
    vertDesc = nil;
    scene->releaseBuffer(triBuffer);
    triBuffer = nil;
    renderState = nil;
    defaultAttrs.clear();
}
    
float BasicDrawableMTL::calcFade(RendererFrameInfo *frameInfo)
{
    // Figure out if we're fading in or out
    float fade = 1.0;
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < fadeDown)
            fade = 0.0;
        else
            if (frameInfo->currentTime > fadeUp)
                fade = 1.0;
            else
                fade = (frameInfo->currentTime - fadeDown)/(fadeUp - fadeDown);
    } else {
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < fadeUp)
                fade = 1.0;
            else
                if (frameInfo->currentTime > fadeDown)
                    fade = 0.0;
                else
                    fade = 1.0-(frameInfo->currentTime - fadeUp)/(fadeDown - fadeUp);
        }
    }
    // Deal with the range based fade
    if (frameInfo->heightAboveSurface > 0.0)
    {
        float factor = 1.0;
        if (minVisibleFadeBand != 0.0)
        {
            float a = (frameInfo->heightAboveSurface - minVisible)/minVisibleFadeBand;
            if (a >= 0.0 && a < 1.0)
                factor = a;
        }
        if (maxVisibleFadeBand != 0.0)
        {
            float b = (maxVisible - frameInfo->heightAboveSurface)/maxVisibleFadeBand;
            if (b >= 0.0 && b < 1.0)
                factor = b;
        }
        
        fade = fade * factor;
    }

    return fade;
}
    
MTLVertexDescriptor *BasicDrawableMTL::getVertexDescriptor(id<MTLFunction> vertFunc,std::vector<AttributeDefault> &defAttrs)
{
    if (vertDesc)
        return vertDesc;
    
    vertDesc = [[MTLVertexDescriptor alloc] init];
    defAttrs.clear();
    std::set<int> buffersFilled;
    
    // Work through the buffers we know about
    for (VertexAttribute *vertAttr : vertexAttributes) {
        MTLVertexAttributeDescriptor *attrDesc = [[MTLVertexAttributeDescriptor alloc] init];
        VertexAttributeMTL *ourVertAttr = (VertexAttributeMTL *)vertAttr;
        
        if (ourVertAttr->bufferIndex < 0)
            continue;
        
        // Describe the vertex attribute
        attrDesc.format = ourVertAttr->formatMTL();
        attrDesc.bufferIndex = ourVertAttr->bufferIndex;
        attrDesc.offset = 0;
        
        // Add in the buffer
        MTLVertexBufferLayoutDescriptor *layoutDesc = [[MTLVertexBufferLayoutDescriptor alloc] init];
        if (ourVertAttr->buffer) {
            // Normal case with one per vertex
            layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
            layoutDesc.stepRate = 1;
            layoutDesc.stride = ourVertAttr->sizeMTL();
        } else {
            // Provides just a default value for the whole thing
            layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
            layoutDesc.stepRate = 0;
            layoutDesc.stride = ourVertAttr->sizeMTL();
            
            AttributeDefault defAttr;
            bzero(&defAttr.data,sizeof(defAttr.data));
            switch (ourVertAttr->dataType) {
                case BDFloat4Type:
                    defAttr.dataType = MTLDataTypeFloat4;
                    for (unsigned int ii=0;ii<4;ii++)
                        defAttr.data.fVals[ii] = ourVertAttr->defaultData.vec4[ii];
                    break;
                case BDFloat3Type:
                    defAttr.dataType = MTLDataTypeFloat3;
                    for (unsigned int ii=0;ii<3;ii++)
                        defAttr.data.fVals[ii] = ourVertAttr->defaultData.vec3[ii];
                    break;
                case BDChar4Type:
                    defAttr.dataType = MTLDataTypeUChar4;
                    for (unsigned int ii=0;ii<4;ii++)
                        defAttr.data.chars[ii] = ourVertAttr->defaultData.color[ii];
                    break;
                case BDFloat2Type:
                    defAttr.dataType = MTLDataTypeFloat2;
                    for (unsigned int ii=0;ii<2;ii++)
                        defAttr.data.fVals[ii] = ourVertAttr->defaultData.vec4[ii];
                    break;
                case BDFloatType:
                    defAttr.dataType = MTLDataTypeFloat;
                    defAttr.data.fVals[0] = ourVertAttr->defaultData.floatVal;
                    break;
                case BDIntType:
                    defAttr.dataType = MTLDataTypeInt;
                    defAttr.data.iVal = ourVertAttr->defaultData.intVal;
                    break;
                default:
                    break;
            }
            defAttr.bufferIndex = ourVertAttr->bufferIndex;
            defAttrs.push_back(defAttr);
        }
        vertDesc.attributes[attrDesc.bufferIndex] = attrDesc;
        vertDesc.layouts[attrDesc.bufferIndex] = layoutDesc;
        
        buffersFilled.insert(ourVertAttr->bufferIndex);
    }

    // Link up the vertex attributes with the buffers
    // Note: Put the preferred attribute index in the vertex attribute
    //       And we can identify unknown attributes that way too
    NSArray<MTLAttribute *> *vertAttrsMTL = vertFunc.stageInputAttributes;
    for (MTLAttribute *vertAttrMTL : vertAttrsMTL) {
        // We don't have this one at all, so let's provide some sort of default anyway
        // This happens with texture coordinates
        if (buffersFilled.find(vertAttrMTL.attributeIndex) == buffersFilled.end()) {
            MTLVertexAttributeDescriptor *attrDesc = [[MTLVertexAttributeDescriptor alloc] init];
            MTLVertexBufferLayoutDescriptor *layoutDesc = [[MTLVertexBufferLayoutDescriptor alloc] init];
            AttributeDefault defAttr;
            bzero(&defAttr.data,sizeof(defAttr.data));
            defAttr.dataType = vertAttrMTL.attributeType;
            defAttr.bufferIndex = vertAttrMTL.attributeIndex;
            switch (vertAttrMTL.attributeType) {
                case MTLDataTypeFloat:
                    attrDesc.format = MTLVertexFormatFloat;
                    layoutDesc.stride = 4;
                    break;
                case MTLDataTypeFloat2:
                    attrDesc.format = MTLVertexFormatFloat2;
                    layoutDesc.stride = 8;
                    break;
                case MTLDataTypeFloat3:
                    attrDesc.format = MTLVertexFormatFloat3;
                    layoutDesc.stride = 12;
                    break;
                case MTLDataTypeFloat4:
                    attrDesc.format = MTLVertexFormatFloat4;
                    layoutDesc.stride = 16;
                    break;
                case MTLDataTypeInt:
                    attrDesc.format = MTLVertexFormatInt;
                    layoutDesc.stride = 4;
                    break;
                default:
                    break;
            }
            attrDesc.bufferIndex = vertAttrMTL.attributeIndex;
            attrDesc.offset = 0;
            vertDesc.attributes[vertAttrMTL.attributeIndex] = attrDesc;
            
            layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
            layoutDesc.stepRate = 0;
            vertDesc.layouts[vertAttrMTL.attributeIndex] = layoutDesc;
            
            defAttrs.push_back(defAttr);
        }
    }

    return vertDesc;
}
    
id<MTLRenderPipelineState> BasicDrawableMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
{
    if (renderState)
        return renderState;
    
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;

    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.vertexDescriptor = getVertexDescriptor(program->vertFunc,defaultAttrs);
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
    
void BasicDrawableMTL::applyUniformsToDrawState(WhirlyKitShader::UniformDrawStateA &drawState,const SingleVertexAttributeSet &uniforms)
{
    for (auto uni : uniforms) {
        if (uni.nameID == u_interpNameID) {
            drawState.interp = uni.data.floatVal;
        } else if (uni.nameID == u_screenOriginNameID) {
            drawState.screenOrigin[0] = uni.data.vec2[0];
            drawState.screenOrigin[1] = uni.data.vec2[1];
        }
    }
}
    
void BasicDrawableMTL::encodeUniBlocks(RendererFrameInfoMTL *frameInfo,const std::vector<BasicDrawable::UniformBlock> &uniBlocks,id<MTLRenderCommandEncoder> cmdEncode)
{
    for (const UniformBlock &uniBlock : uniBlocks) {
        [cmdEncode setVertexBytes:uniBlock.blockData->getRawData() length:uniBlock.blockData->getLen() atIndex:uniBlock.bufferID];
        [cmdEncode setFragmentBytes:uniBlock.blockData->getRawData() length:uniBlock.blockData->getLen() atIndex:uniBlock.bufferID];
    }
}

void BasicDrawableMTL::draw(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;

    float fade = calcFade(frameInfo);
    
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo);
    
    // Wire up the various inputs that we know about
    for (auto vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer && (vertAttrMTL->bufferIndex >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer offset:0 atIndex:vertAttrMTL->bufferIndex];
    }
    
    // And provide defaults for the ones we don't
    for (auto defAttr : defaultAttrs)
        [cmdEncode setVertexBytes:&defAttr.data length:sizeof(defAttr.data) atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    // Pass in the textures (and offsets)
    // Note: We could precalculate most of then when the texture changes
    //       And we should figure out how many textures they actually have
    int numTextures = 0;
    for (unsigned int texIndex=0;texIndex<texInfo.size();texIndex++) {
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

        [cmdEncode setVertexBytes:&texInd length:sizeof(texInd) atIndex:WKSTexIndirectStartBuffer+numTextures];

        // And the texture itself
        // Note: Should we be setting up the sampler?
        TextureBaseMTL *tex = NULL;
        if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
            tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
        if (tex && tex->getMTLID()) {
            [cmdEncode setVertexTexture:tex->getMTLID() atIndex:numTextures];
            [cmdEncode setFragmentTexture:tex->getMTLID() atIndex:numTextures];
            numTextures++;
        } else {
//            [frameInfo->cmdEncode setVertexTexture:nil atIndex:texIndex];
//            [frameInfo->cmdEncode setFragmentTexture:nil atIndex:texIndex];
        }
    }
    
    // The shaders get bitchy if we don't supply all the buffers
    if (numTextures == 0) {
        WhirlyKitShader::TexIndirect texInd;
        texInd.offset[0] = 0.0;  texInd.offset[1] = 0.0;
        texInd.scale[0] = 1.0; texInd.scale[1] = 1.0;

        [cmdEncode setVertexBytes:&texInd length:sizeof(texInd) atIndex:WKSTexIndirectStartBuffer+0];
        [cmdEncode setVertexBytes:&texInd length:sizeof(texInd) atIndex:WKSTexIndirectStartBuffer+1];
    }
    
    // Set the per-drawable draw state
    WhirlyKitShader::UniformDrawStateA uni;
    sceneRender->setupDrawStateA(uni,frameInfo);
    uni.numTextures = numTextures;
    uni.fade = fade;
    uni.clipCoords = clipCoords;
    applyUniformsToDrawState(uni,uniforms);
    [cmdEncode setVertexBytes:&uni length:sizeof(uni) atIndex:WKSUniformDrawStateBuffer];
    [cmdEncode setFragmentBytes:&uni length:sizeof(uni) atIndex:WKSUniformDrawStateBuffer];
    
    // And the uniforms passed through the drawable
    encodeUniBlocks(frameInfo, uniBlocks, cmdEncode);
    
    // Render the primitives themselves
    switch (type) {
        case Lines:
            [cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:numPts];
            break;
        case Triangles:
            // This actually draws the triangles (well, in a bit)
            [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer indexBufferOffset:0];
            break;
        default:
            break;
    }
}
    
}
