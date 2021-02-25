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
    : BasicDrawable(name), Drawable(name), setupForMTL(false), vertDesc(nil), renderState(nil), numPts(0), numTris(0),vertHasTextures(false),fragHasTextures(false),vertHasLighting(false),fragHasLighting(false)
{
}

BasicDrawableMTL::~BasicDrawableMTL()
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
            buffBuild.addData(vertAttrMTL->addressForElement(0), bufferSize, &vertAttrMTL->buffer);
            vertAttrMTL->clear();
        }
    }
    
    // And put the triangles in their own
    // Note: Could use 1 byte some of the time
    int bufferSize = 3*2*tris.size();
    numTris = tris.size();
    if (bufferSize > 0) {
        buffBuild.addData(&tris[0], bufferSize, &triBuffer);
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
        buffBuild.addData(&defAttr.data, sizeof(defAttr.data), &defAttr.buffer);
    
    // Construct the buffer we've been adding to
    mainBuffer = buffBuild.buildBuffer();
    
    setupForMTL = true;
}

void BasicDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene,RenderTeardownInfoRef inTeardown)
{
    RenderTeardownInfoMTLRef teardown = std::dynamic_pointer_cast<RenderTeardownInfoMTL>(inTeardown);
    
    if (teardown)
        enumerateBuffers(*teardown->resources);
    
    setupForMTL = false;

    renderState = nil;
    vertDesc = nil;
    triBuffer.clear();
    defaultAttrs.clear();
    mainBuffer.clear();
    vertABInfo.reset();
    fragABInfo.reset();
    vertTexInfo.reset();
    fragTexInfo.reset();
    activeTextures.clear();

    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        vertAttrMTL->buffer.clear();
        delete vertAttrMTL;
    }
    vertexAttributes.clear();
    uniBlocks.clear();
    uniforms.clear();
    renderTargetCon.reset();
    tweakers.clear();
}
    
MTLVertexDescriptor *BasicDrawableMTL::getVertexDescriptor(id<MTLFunction> vertFunc,std::vector<AttributeDefault> &defAttrs)
{
    if (vertDesc)
        return vertDesc;
    
    vertDesc = [[MTLVertexDescriptor alloc] init];
    defAttrs.clear();
    std::set<int> buffersFilled;
    
    // Work through the buffers we know about
    int whichEntry = 0;
    for (VertexAttribute *vertAttr : vertexAttributes) {
        MTLVertexAttributeDescriptor *attrDesc = [[MTLVertexAttributeDescriptor alloc] init];
        VertexAttributeMTL *ourVertAttr = (VertexAttributeMTL *)vertAttr;
        
        if (ourVertAttr->slot < 0) {
            continue;
        }
        
        // Describe the vertex attribute
        attrDesc.format = ourVertAttr->formatMTL();
        attrDesc.bufferIndex = ourVertAttr->slot;
        attrDesc.offset = 0;
        
        // Add in the buffer
        MTLVertexBufferLayoutDescriptor *layoutDesc = [[MTLVertexBufferLayoutDescriptor alloc] init];
        if (ourVertAttr->buffer.valid) {
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
            defAttr.entry = whichEntry;
            defAttr.bufferIndex = ourVertAttr->slot;
            defAttrs.push_back(defAttr);
        }
        vertDesc.attributes[attrDesc.bufferIndex] = attrDesc;
        vertDesc.layouts[attrDesc.bufferIndex] = layoutDesc;
        
        buffersFilled.insert(ourVertAttr->slot);
        whichEntry++;
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
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,program,renderTarget);
    renderDesc.vertexDescriptor = getVertexDescriptor(program->vertFunc,defaultAttrs);
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];

    // Set up a render state
    NSError *err = nil;
    renderState = [sceneRender->setupInfo.mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
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

void BasicDrawableMTL::setOverrideColor(RGBAColor inColor)
{
    BasicDrawable::setOverrideColor(inColor);
    
    // Look for it in the default attributes
    if (colorEntry < 0)
        return;
    
    // Change the vertex attributes first
    auto vertAttr = vertexAttributes[colorEntry];
    inColor.asUChar4(vertAttr->defaultData.color);

    // Need to find the corresponding default attribute if we've already set this up
    for (auto &attr: defaultAttrs) {
        if (attr.entry == colorEntry) {
            inColor.asUChar4(attr.data.chars);
            return;
        }
    }
}

namespace {
    const static std::string hasTextures("hasTextures");
    const static std::string hasLighting("hasLighting");
}

void BasicDrawableMTL::setupArgBuffers(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfo,SceneMTL *scene,BufferBuilderMTL &buffBuild)
{
    ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programId);
    if (!prog)  // This happens if we're being used by an instance
        return;
    
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
        vertABInfo = std::make_shared<ArgBuffContentsMTL>(mtlDevice,
                                                          setupInfo,
                                                          prog->vertFunc,
                                                          WhirlyKitShader::WKSVertexArgBuffer,
                                                          buffBuild);
        vertHasTextures = vertABInfo->hasConstant(hasTextures);
        vertHasLighting = vertABInfo->hasConstant(hasLighting);
        if (vertABInfo->isEmpty())
            vertABInfo = nullptr;
        if (vertHasTextures)
            vertTexInfo = std::make_shared<ArgBuffRegularTexturesMTL>(mtlDevice,
                                                                      setupInfo,
                                                                      prog->vertFunc,
                                                                      WhirlyKitShader::WKSVertTextureArgBuffer,
                                                                      buffBuild);
    }
    if (prog->fragFunc) {
        fragABInfo = std::make_shared<ArgBuffContentsMTL>(mtlDevice,
                                                          setupInfo,
                                                          prog->fragFunc,
                                                          WhirlyKitShader::WKSFragmentArgBuffer,
                                                          buffBuild);
        fragHasTextures = fragABInfo->hasConstant(hasTextures);
        fragHasLighting = fragABInfo->hasConstant(hasLighting);
        if (fragABInfo->isEmpty())
            fragABInfo = nullptr;
        if (fragHasTextures)
            fragTexInfo = std::make_shared<ArgBuffRegularTexturesMTL>(mtlDevice,
                                                                      setupInfo,
                                                                      prog->fragFunc,
                                                                      WhirlyKitShader::WKSFragTextureArgBuffer,
                                                                      buffBuild);
    }
}

// Called before anything starts calculating or drawing to fill in buffers and such
bool BasicDrawableMTL::preProcess(SceneRendererMTL *sceneRender,id<MTLCommandBuffer> cmdBuff,id<MTLBlitCommandEncoder> bltEncode,SceneMTL *scene)
{
    bool ret = false;
    
    ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programId);
    if (!prog) {
        NSLog(@"Drawable %s missing program.",name.c_str());
        return false;
    }

    if (texturesChanged || valuesChanged || prog->changed) {
        ret = true;
        
        // We need to blit the default values into place because we let those change
        // Typically, this is color
        BufferBuilderMTL buffBuild(&sceneRender->setupInfo);
        BufferEntryMTL baseBuff;
        for (auto &defAttr : defaultAttrs) {
            if (!baseBuff.valid)
                baseBuff = defAttr.buffer;
            buffBuild.addData(&defAttr.data, sizeof(defAttr.data), &defAttr.buffer);
        }
        if (baseBuff.valid) {
            BufferEntryMTL srcBuff = buffBuild.buildBuffer();
            [bltEncode copyFromBuffer:srcBuff.buffer sourceOffset:0 toBuffer:baseBuff.buffer destinationOffset:baseBuff.offset size:srcBuff.buffer.length];
        }

        if (texturesChanged && (vertTexInfo || fragTexInfo)) {
            activeTextures.clear();
            
            int numEntries = texInfo.size();
            if (prog && !prog->textures.empty()) {
                int maxTex = -1;
                for (auto progTex: prog->textures)
                    maxTex = std::max(maxTex,progTex.slot);
                if (maxTex >= 0)
                    numEntries = WKSTextureEntryLookup+maxTex+1;
            }
            std::vector<TexInfo> allTexInfo(numEntries);
            
            // Copy in the textures from this drawable instance and the base drawable
            for (unsigned int texIndex=0;texIndex<allTexInfo.size();texIndex++) {
                if (texIndex < texInfo.size())
                    allTexInfo[texIndex] = TexInfo(texInfo[texIndex]);
            }
            // And then copy in the textures from the program
            for (auto progTex: prog->textures) {
                int texIndex = WKSTextureEntryLookup+progTex.slot;
                allTexInfo[texIndex].texId = progTex.texID;
            }

            // Wire up the textures and texture indirection values
            for (unsigned int texIndex=0;texIndex<WKSTextureMax;texIndex++) {
                TexInfo *thisTexInfo = (texIndex < allTexInfo.size()) ? &allTexInfo[texIndex] : NULL;
                if (thisTexInfo && thisTexInfo->texId == EmptyIdentity)
                    thisTexInfo = NULL;

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
                
                // And the texture itself
                TextureBaseMTLRef tex;
                if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
                    tex = std::dynamic_pointer_cast<TextureBaseMTL>(scene->getTexture(thisTexInfo->texId));
                if (tex)
                    activeTextures.push_back(tex->getMTLTex());
                if (vertTexInfo)
                    vertTexInfo->addTexture(texOffset, Point2f(texScale,texScale), tex != nil ? tex->getMTLID() : nil);
                if (fragTexInfo)
                    fragTexInfo->addTexture(texOffset, Point2f(texScale,texScale), tex != nil ? tex->getMTLID() : nil);
            }
            if (vertTexInfo) {
                vertTexInfo->updateBuffer(sceneRender->setupInfo.mtlDevice, &sceneRender->setupInfo, bltEncode);
            }
            if (fragTexInfo) {
                fragTexInfo->updateBuffer(sceneRender->setupInfo.mtlDevice, &sceneRender->setupInfo, bltEncode);
            }
        }

        if (valuesChanged || prog->changed) {
            if (vertABInfo)
                vertABInfo->startEncoding(sceneRender->setupInfo.mtlDevice);
            if (fragABInfo)
                fragABInfo->startEncoding(sceneRender->setupInfo.mtlDevice);
            
            // Uniform blocks associated with the program
            for (const UniformBlock &uniBlock : prog->uniBlocks) {
                vertABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode,uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
                fragABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode,uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
            }

            // And the uniforms passed through the drawable
            bool hasExp = false;
            for (const UniformBlock &uniBlock : uniBlocks) {
                if (uniBlock.bufferID == WhirlyKitShader::WKSUniformVecEntryExp)
                    hasExp = true;
                vertABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode, uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
                fragABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode, uniBlock.bufferID, (void *)uniBlock.blockData->getRawData(), uniBlock.blockData->getLen());
            }
            
            // Per drawable draw state in its own buffer
            // Has to update if either textures or values updated
            WhirlyKitShader::UniformDrawStateA uni;
            sceneRender->setupDrawStateA(uni);
            uni.zoomSlot = zoomSlot;
            uni.clipCoords = clipCoords;
            uni.hasExp = hasExp;
            if (hasMatrix)
                CopyIntoMtlFloat4x4(uni.singleMat, *getMatrix());
            else {
                Eigen::Matrix4d identMatrix = Eigen::Matrix4d::Identity();
                CopyIntoMtlFloat4x4(uni.singleMat, identMatrix);
            }
            double baseTime = scene->getBaseTime();
            uni.fadeUp = fadeUp - baseTime;
            uni.fadeDown = fadeDown - baseTime;
            uni.minVisible = minVisible;
            uni.maxVisible = maxVisible;
            uni.minVisibleFadeBand = minVisibleFadeBand;
            uni.maxVisibleFadeBand = maxVisibleFadeBand;
            applyUniformsToDrawState(uni,uniforms);
            if (vertABInfo)
                vertABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode, WhirlyKitShader::WKSUniformDrawStateEntry, &uni, sizeof(uni));
            if (fragABInfo)
                fragABInfo->updateEntry(sceneRender->setupInfo.mtlDevice,bltEncode, WhirlyKitShader::WKSUniformDrawStateEntry, &uni, sizeof(uni));

            if (vertABInfo) {
                vertABInfo->endEncoding(sceneRender->setupInfo.mtlDevice, bltEncode);
            }
            if (fragABInfo) {
                fragABInfo->endEncoding(sceneRender->setupInfo.mtlDevice, bltEncode);
            }
        }

        texturesChanged = false;
        valuesChanged = false;
    }
    
    return ret;
}

void BasicDrawableMTL::enumerateBuffers(ResourceRefsMTL &resources)
{
    resources.addEntry(mainBuffer);
    if (vertABInfo)
        vertABInfo->addResources(resources);
    if (fragABInfo)
        fragABInfo->addResources(resources);
    if (vertTexInfo)
        vertTexInfo->addResources(resources);
    if (fragTexInfo)
        fragTexInfo->addResources(resources);
    resources.addTextures(activeTextures);
}

void BasicDrawableMTL::enumerateResources(RendererFrameInfoMTL *frameInfo,ResourceRefsMTL &resources)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;

    resources.addEntry(sceneRender->setupInfo.uniformBuff);
    if (vertHasLighting || fragHasLighting)
        resources.addEntry(sceneRender->setupInfo.lightingBuff);
    enumerateBuffers(resources);
}

void BasicDrawableMTL::encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    // TODO: Fill this in
}

void BasicDrawableMTL::encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;

    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,scene,(ProgramMTL *)frameInfo->program,(RenderTargetMTL *)frameInfo->renderTarget);
    
    // Wire up the various inputs that we know about
    for (auto vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer.buffer && (vertAttrMTL->slot >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer.buffer offset:vertAttrMTL->buffer.offset atIndex:vertAttrMTL->slot];
    }
    
    // And provide defaults for the ones we don't
    for (auto &defAttr : defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer.buffer offset:defAttr.buffer.offset atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    // Everything takes the uniforms
    [cmdEncode setVertexBuffer:sceneRender->setupInfo.uniformBuff.buffer offset:sceneRender->setupInfo.uniformBuff.offset atIndex:WhirlyKitShader::WKSVertUniformArgBuffer];
    [cmdEncode setFragmentBuffer:sceneRender->setupInfo.uniformBuff.buffer offset:sceneRender->setupInfo.uniformBuff.offset atIndex:WhirlyKitShader::WKSFragUniformArgBuffer];

    // Some shaders take the lighting
    if (vertHasLighting)
        [cmdEncode setVertexBuffer:sceneRender->setupInfo.lightingBuff.buffer offset:sceneRender->setupInfo.lightingBuff.offset atIndex:WhirlyKitShader::WKSVertLightingArgBuffer];
    if (fragHasLighting)
        [cmdEncode setFragmentBuffer:sceneRender->setupInfo.lightingBuff.buffer offset:sceneRender->setupInfo.lightingBuff.offset atIndex:WhirlyKitShader::WKSFragLightingArgBuffer];

    // More flexible data structures passed in to the shaders
    if (vertABInfo) {
        BufferEntryMTL &buff = vertABInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTL &buff = fragABInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSFragmentArgBuffer];
    }

    // Textures may or may not be passed in to shaders
    if (vertTexInfo) {
        BufferEntryMTL &buff = vertTexInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSVertTextureArgBuffer];
    }
    if (fragTexInfo) {
        BufferEntryMTL &buff = fragTexInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSFragTextureArgBuffer];
    }

    // Render the primitives themselves
    switch (type) {
        case Lines:
            [cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:numPts];
            break;
        case Triangles:
            // This actually draws the triangles (well, in a bit)
            [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer.buffer indexBufferOffset:triBuffer.offset];
            break;
        default:
            break;
    }
}

void BasicDrawableMTL::encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget)
{
    // TODO: Fill this in
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
        if (vertAttrMTL->buffer.buffer && (vertAttrMTL->slot >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer.buffer offset:vertAttrMTL->buffer.offset atIndex:vertAttrMTL->slot];
    }
    
    // And provide defaults for the ones we don't
    for (auto defAttr : defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer.buffer offset:defAttr.buffer.offset atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    // Everything takes the uniforms
    [cmdEncode setVertexBuffer:sceneRender->setupInfo.uniformBuff.buffer offset:sceneRender->setupInfo.uniformBuff.offset atIndex:WhirlyKitShader::WKSVertUniformArgBuffer];
    [cmdEncode setFragmentBuffer:sceneRender->setupInfo.uniformBuff.buffer offset:sceneRender->setupInfo.uniformBuff.offset atIndex:WhirlyKitShader::WKSFragUniformArgBuffer];

    // Some shaders take the lighting
    if (vertHasLighting)
        [cmdEncode setVertexBuffer:sceneRender->setupInfo.lightingBuff.buffer offset:sceneRender->setupInfo.lightingBuff.offset atIndex:WhirlyKitShader::WKSVertLightingArgBuffer];
    if (fragHasLighting)
        [cmdEncode setFragmentBuffer:sceneRender->setupInfo.lightingBuff.buffer offset:sceneRender->setupInfo.lightingBuff.offset atIndex:WhirlyKitShader::WKSFragLightingArgBuffer];

    // More flexible data structures passed in to the shaders
    if (vertABInfo) {
        BufferEntryMTL &buff = vertABInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTL &buff = fragABInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSFragmentArgBuffer];
    }

    // Textures may or may not be passed in to shaders
    if (vertTexInfo) {
        BufferEntryMTL &buff = vertTexInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSVertTextureArgBuffer];
    }
    if (fragTexInfo) {
        BufferEntryMTL &buff = fragTexInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff.buffer offset:buff.offset atIndex:WhirlyKitShader::WKSFragTextureArgBuffer];
    }

    // Render the primitives themselves
    switch (type) {
        case Lines:
            [cmdEncode drawPrimitives:MTLPrimitiveTypeLine vertexStart:0 vertexCount:numPts instanceCount:1 baseInstance:0];
            break;
        case Triangles:
            if (numTris == 0) {
                NSLog(@"BasicDrawableMTL: Found a drawable with no triangles.");
                return;
            }
            // This actually draws the triangles (well, in a bit)
            [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer.buffer indexBufferOffset:triBuffer.offset instanceCount:1 baseVertex:0 baseInstance:0];
            break;
        default:
            break;
    }
}
    
}
