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
    : BasicDrawable(name), Drawable(name), setupForMTL(false), vertDesc(nil), renderState(nil), numPts(0), numTris(0)
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
void BasicDrawableMTL::setupForRenderer(const RenderSetupInfo *inSetupInfo,Scene *inScene)
{
    if (setupForMTL)
        return;
        
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    
    BufferBuilderMTL buffBuild(setupInfo);

    // Set up the buffers for each vertex attribute
    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        
        int bufferSize = vertAttrMTL->sizeMTL() * vertAttrMTL->numElements();
        if (bufferSize > 0) {
            numPts = vertAttrMTL->numElements();
            vertAttrMTL->buffer = buffBuild.addData(vertAttrMTL->addressForElement(0), bufferSize);
            vertAttrMTL->clear();
        }
    }
    
    // And put the triangles in their own
    // Note: Could use 1 byte some of the time
    int bufferSize = 3*2*tris.size();
    numTris = tris.size();
    if (bufferSize > 0) {
        triBuffer = buffBuild.addData(&tris[0], bufferSize);
        tris.clear();
    }
    
    // Build the argument buffers with all their attendent memory, ready to copy into
    setupArgBuffers(setupInfo->mtlDevice,setupInfo,scene,buffBuild);
    
    // And let's fault in the vertex descriptor as well
    ProgramMTL *program = (ProgramMTL *)scene->getProgram(programId);
    if (program && program->vertFunc)
        getVertexDescriptor(program->vertFunc, defaultAttrs);
    
    // Last, we'll set up the default attributes in the new buffer as well
    for (auto &defAttr : defaultAttrs)
        defAttr.buffer = buffBuild.addData(&defAttr.data, sizeof(defAttr.data));
    
    // Construct the buffer we've been adding to
    mainBuffer = buffBuild.buildBuffer();
    
    if (vertABInfo)
        vertABInfo->wireUpBuffers();
    if (fragABInfo)
        fragABInfo->wireUpBuffers();

    setupForMTL = true;
}

void BasicDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene)
{
    setupForMTL = false;
    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        vertAttrMTL->buffer.reset();
    }
    
    vertDesc = nil;
    triBuffer.reset();
    triBuffer = nil;
    renderState = nil;
    defaultAttrs.clear();
}
    
// TODO: Move into shader
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
    
id<MTLRenderPipelineState> BasicDrawableMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget)
{
    if (renderState)
        return renderState;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;

    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,program,renderTarget);
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
    
// Initialize the argument buffers and associated memory empty
void BasicDrawableMTL::setupArgBuffers(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfo,SceneMTL *scene,BufferBuilderMTL &buffBuild)
{
    ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programId);
    
    // All of these are optional, but here's what we're expecting
    //   Uniforms
    //   Lighting
    //   UniformDrawStateA
    //
    //   [Program's custom uniforms]
    //   [Drawable's custom Uniforms]
    //
    //   TexIndirect[WKSTextureMax]
    //   tex[WKTextureMax]
    
    // Set up the argument buffers if they're not in place
    // This allocates some of the buffer memory, so only do once
    if (prog->vertFunc) {
        vertABInfo = ArgBuffContentsMTLRef(new ArgBuffContentsMTL(mtlDevice,
                                                                  setupInfo,
                                                                  prog->vertFunc,
                                                                  WKSVertexArgBuffer,
                                                                  buffBuild));
        vertABInfo->setEntry(WKSUniformArgBuffer,setupInfo->uniformBuff);
        vertABInfo->setEntry(WKSLightingArgBuffer, setupInfo->lightingBuff);
        vertABInfo->createBuffers(mtlDevice,buffBuild);
    }
    if (prog->fragFunc) {
        fragABInfo = ArgBuffContentsMTLRef(new ArgBuffContentsMTL(mtlDevice,
                                                                  setupInfo,
                                                                  prog->fragFunc,
                                                                  WKSFragmentArgBuffer,
                                                                  buffBuild));
        fragABInfo->setEntry(WKSUniformArgBuffer,setupInfo->uniformBuff);
        fragABInfo->setEntry(WKSLightingArgBuffer, setupInfo->lightingBuff);
        fragABInfo->createBuffers(mtlDevice,buffBuild);
    }

    // Provide empty data for all the textures
    WhirlyKitShader::TexIndirect texIndirect[WKSTextureMax];
    for (unsigned int texIndex=0;texIndex<WKSTextureMax;texIndex++) {
        // Figure out texture adjustment for parent textures
        float texScale = 1.0;
        Vector2f texOffset(0.0,0.0);

        // Calculate offset and scales
        WhirlyKitShader::TexIndirect &texInd = texIndirect[texIndex];
        texInd.offset[0] = texOffset.x();  texInd.offset[1] = texOffset.y();
        texInd.scale[0] = texScale; texInd.scale[1] = texScale;
        
        // And the texture itself
        if (vertABInfo)
            vertABInfo->setTexture(texIndex, nil);
        if (fragABInfo)
            fragABInfo->setTexture(texIndex, nil);
    }
    // TODO: Clear out the tex indirect buffers to provide defaults
}

// Called before anything starts calculating or drawing to fill in buffers and such
void BasicDrawableMTL::preProcess(SceneRendererMTL *sceneRender,id<MTLCommandBuffer> cmdBuff,id<MTLBlitCommandEncoder> bltEncode,SceneMTL *scene,ResourceRefsMTL &resources)
{
    if (texturesChanged || valuesChanged) {
        ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programId);
        id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;

        if (!prog) {
            NSLog(@"Drawable %s missing program.",name.c_str());
            return;
        }

        int numTextures = 0;
        if (texturesChanged) {
            // Wire up the textures and texture indirection values
            WhirlyKitShader::TexIndirect texIndirect[WKSTextureMax];
            for (unsigned int texIndex=0;texIndex<WKSTextureMax;texIndex++) {
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
                WhirlyKitShader::TexIndirect &texInd = texIndirect[texIndex];
                texInd.offset[0] = texOffset.x();  texInd.offset[1] = texOffset.y();
                texInd.scale[0] = texScale; texInd.scale[1] = texScale;
                
                // And the texture itself
                TextureBaseMTL *tex = NULL;
                if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
                    tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
                if (tex && tex->getMTLID()) {
                    if (vertABInfo)
                        vertABInfo->setTexture(texIndex, tex->getMTLID());
                    if (fragABInfo)
                        fragABInfo->setTexture(texIndex, tex->getMTLID());
                    numTextures++;
                } else {
                    if (vertABInfo)
                        vertABInfo->setTexture(texIndex, nil);
                    if (fragABInfo)
                        fragABInfo->setTexture(texIndex, nil);
                }
            }
            if (vertABInfo)
                vertABInfo->updateEntry(mtlDevice,bltEncode,WKSTexIndirectArgBuffer, &texIndirect[0], sizeof(WhirlyKitShader::TexIndirect)*WKSTextureMax);
            if (fragABInfo)
                fragABInfo->updateEntry(mtlDevice,bltEncode,WKSTexIndirectArgBuffer, &texIndirect[0], sizeof(WhirlyKitShader::TexIndirect)*WKSTextureMax);
        }

        if (valuesChanged) {
            // Uniform blocks associated with the program
            for (const UniformBlock &uniBlock : prog->uniBlocks) {
                vertABInfo->updateEntry(mtlDevice,bltEncode,uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
                fragABInfo->updateEntry(mtlDevice,bltEncode,uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
            }

            // And the uniforms passed through the drawable
            for (const UniformBlock &uniBlock : uniBlocks) {
                vertABInfo->updateEntry(mtlDevice,bltEncode, uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
                fragABInfo->updateEntry(mtlDevice,bltEncode, uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
            }
        }

        // Per drawable draw state in its own buffer
        // Has to update if either textures or values updated
        WhirlyKitShader::UniformDrawStateA uni;
        sceneRender->setupDrawStateA(uni);
        uni.numTextures = numTextures;
        // TODO: Move into shader
        uni.fade = 1.0;
//        uni.fade = calcFade(frameInfo);
        uni.clipCoords = clipCoords;
        applyUniformsToDrawState(uni,uniforms);
        if (vertABInfo)
            vertABInfo->updateEntry(mtlDevice,bltEncode, WKSUniformDrawStateArgBuffer, &uni, sizeof(uni));
        if (fragABInfo)
            fragABInfo->updateEntry(mtlDevice,bltEncode, WKSUniformDrawStateArgBuffer, &uni, sizeof(uni));

        texturesChanged = false;
        valuesChanged = false;
    }
    
    // Always need the resource lists
    resourceRefs(resources);
}

// Add to the list of resources (buffers, textures, heaps) in use by this drawable
void BasicDrawableMTL::resourceRefs(ResourceRefsMTL &resourceRefs)
{
    // It should all be in one buffer
    resourceRefs.addEntry(mainBuffer);
}

void BasicDrawableMTL::encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
}

void BasicDrawableMTL::encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;

    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,scene,(ProgramMTL *)frameInfo->program,(RenderTargetMTL *)frameInfo->renderTarget);
    
    // Wire up the various inputs that we know about
    for (auto vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer && (vertAttrMTL->bufferIndex >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer->buffer offset:vertAttrMTL->buffer->offset atIndex:vertAttrMTL->bufferIndex];
    }
    
    // And provide defaults for the ones we don't
    for (auto defAttr : defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    if (vertABInfo) {
        BufferEntryMTLRef buff = vertABInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTLRef buff = fragABInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WKSFragmentArgBuffer];
    }

    // Render the primitives themselves
    switch (type) {
        case Lines:
            [cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:numPts];
            break;
        case Triangles:
            // This actually draws the triangles (well, in a bit)
            [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer->buffer indexBufferOffset:triBuffer->offset];
            break;
        default:
            break;
    }
}

void BasicDrawableMTL::encodeInirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget)
{
}

void BasicDrawableMTL::encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget)
{
    ProgramMTL *program = (ProgramMTL *)scene->getProgram(programId);
    if (!program) {
        NSLog(@"BasicDrawableMTL: Missing programId for %s",name.c_str());
        return;
    }
    
    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,scene,program,renderTarget);
    
    // Wire up the various inputs that we know about
    for (auto vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer && (vertAttrMTL->bufferIndex >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer->buffer offset:vertAttrMTL->buffer->offset atIndex:vertAttrMTL->bufferIndex];
    }
    
    // And provide defaults for the ones we don't
    for (auto defAttr : defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    if (vertABInfo) {
        BufferEntryMTLRef buff = vertABInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTLRef buff = fragABInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WKSFragmentArgBuffer];
    }

    // Render the primitives themselves
    switch (type) {
        case Lines:
            [cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:numPts instanceCount:1 baseInstance:0];
            break;
        case Triangles:
            // This actually draws the triangles (well, in a bit)
            [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer->buffer indexBufferOffset:triBuffer->offset instanceCount:1 baseVertex:0 baseInstance:0];
            break;
        default:
            break;
    }
}
    
}
