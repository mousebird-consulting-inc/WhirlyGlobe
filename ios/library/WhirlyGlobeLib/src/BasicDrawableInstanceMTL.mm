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
    : BasicDrawableInstance(name), Drawable(name), renderState(nil), setupForMTL(false), numInst(0)
{
}

BasicDrawableInstanceMTL::~BasicDrawableInstanceMTL()
{
}

// Place or update the default attribute corresponding to color
// TODO: Make this more general
void BasicDrawableInstanceMTL::updateColorDefaultAttr()
{
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
    if (!basicDrawMTL)
        return;

    if (hasColor) {
        VertexAttributeMTL *colorAttrMTL = basicDrawMTL->findVertexAttribute(a_colorNameID);
        if (colorAttrMTL) {
            BasicDrawableMTL::AttributeDefault defAttr;
            defAttr.dataType = MTLDataTypeUChar4;
            defAttr.bufferIndex = colorAttrMTL->bufferIndex;
            unsigned char chars[4];
            color.asUChar4(chars);
            for (unsigned int ii=0;ii<4;ii++)
                defAttr.data.chars[ii] = chars[ii];
            
            // Might need to replace this one
            bool found = false;
            for (int ii=0;ii<defaultAttrs.size();ii++)
                if (defaultAttrs[ii].bufferIndex == defAttr.bufferIndex) {
                    defaultAttrs[ii] = defAttr;
                    found = true;
                    break;
                }

            if (!found)
                defaultAttrs.push_back(defAttr);
        }
    }
}

void BasicDrawableInstanceMTL::setColor(RGBAColor inColor)
{
    BasicDrawableInstance::setColor(inColor);
    
    updateColorDefaultAttr();
}


void BasicDrawableInstanceMTL::setupForRenderer(const RenderSetupInfo *inSetupInfo,Scene *scene)
{
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    
    if (setupForMTL)
        return;
    
    if (instanceStyle == LocalStyle) {
        bzero(&uniMI,sizeof(uniMI));
        
        // Set up the instances in their own array
        std::vector<WhirlyKitShader::VertexTriModelInstance> insts(instances.size());
        for (int which = 0;which < instances.size();which++) {
            auto &inst = instances[which];
            auto &outInst = insts[which];
            
            // Color override
            if (inst.colorOverride) {
                uniMI.useInstanceColor = true;
                float colors[4];
                inst.color.asUnitFloats(colors);
                CopyIntoMtlFloat4(outInst.color, colors);
            } else {
                outInst.color[0] = 1.0;  outInst.color[1] = 1.0;  outInst.color[2] = 1.0;  outInst.color[3] = 1.0;
            }
            
            // Center
            CopyIntoMtlFloat3(outInst.center, inst.center);
            
            // Rotation/translation/scale
            CopyIntoMtlFloat4x4(outInst.mat, inst.mat);
            
            // EndCenter/direction
            Point3d dir = moving ? (inst.endCenter - inst.center)/inst.duration : Point3d(0.0,0.0,0.0);
            CopyIntoMtlFloat3(outInst.dir, dir);
            uniMI.hasMotion |= moving;
        }

        int bufferSize = sizeof(WhirlyKitShader::VertexTriModelInstance) * insts.size();
        instBuffer = [setupInfo->mtlDevice newBufferWithBytes:&insts[0] length:bufferSize options:MTLStorageModeShared];
        if (!name.empty())
            instBuffer.label = [NSString stringWithFormat:@"%s inst",name.c_str()];
        numInst = insts.size();
    } else if (instanceStyle == GPUStyle) {
        // Basic values for the uniforms
        bzero(&uniMI,sizeof(uniMI));
        uniMI.time = 0.0;
        uniMI.useInstanceColor = false;
        uniMI.hasMotion = false;
    }
    
    
        
    setupForMTL = true;
}

void BasicDrawableInstanceMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene)
{
    SceneMTL *scene = (SceneMTL *)inScene;

    setupForMTL = false;
    scene->releaseBuffer(instBuffer);
    instBuffer = nil;
    renderState = nil;
    defaultAttrs.clear();
}

// Render state for calculation pass
id<MTLRenderPipelineState> BasicDrawableInstanceMTL::getCalcRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo)
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

void BasicDrawableInstanceMTL::calculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
    if (!basicDrawMTL) {
        NSLog(@"BasicDrawableInstance pointing at a bad BasicDrawable");
        return;
    }
    if (!basicDrawMTL)
        return;
//    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
//    // Render state is pretty simple, so apply that
//    id<MTLRenderPipelineState> renderState = getCalcRenderPipelineState(sceneRender,frameInfo);
//    [cmdEncode setRenderPipelineState:renderState];
//
//    if (instanceStyle == GPUStyle) {
//        if (!indirectBuffer) {
//            // Set up a buffer for the indirect arguments
//            MTLDrawIndexedPrimitivesIndirectArguments args;
//            args.indexCount = basicDrawMTL->numTris*3;
//            args.instanceCount = 0;
//            args.indexStart = 0;
//            args.baseVertex = 0;
//            args.baseInstance = 0;
//            indirectBuffer = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&args length:sizeof(MTLDrawIndexedPrimitivesIndirectArguments) options:MTLStorageModeShared];
//        }
//        
//        TextureBaseMTL *tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(instanceTexSource));
//        if (!tex) {
//            NSLog(@"Missing texture for instance texture source in BasicDrawableInstanceMTL::draw() for tex %d",(int)instanceTexSource);
//            return;
//        }
//        
//        // Set up the inputs for the program to copy into the indirect buffer
//        [cmdEncode setVertexBuffer:indirectBuffer offset:0 atIndex:WKSInstanceIndirectBuffer];
//        [cmdEncode setVertexTexture:tex->getMTLID() atIndex:0];
//        Point3f pt(0.0,0.0,0.0);
//        [cmdEncode setVertexBytes:&pt length:sizeof(float)*3 atIndex:0];
//        [cmdEncode drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:1];
//    }
}

id<MTLBuffer> BasicDrawableInstanceMTL::encodeArgumentBuffer(SceneMTL *scene,
                                                            RendererFrameInfoMTL *frameInfo,
                                                            id<MTLFunction> func,
                                                            int bufferIndex,
                                                            std::set< id<MTLBuffer> > &buffers,
                                                            std::set< id<MTLTexture> > &textures)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());

    MTLAutoreleasedArgument argInfo;
    id<MTLArgumentEncoder> argEncode = [func newArgumentEncoderWithBufferIndex:bufferIndex reflection:&argInfo];
    if (!argEncode)
        return nil;
    
    // Figure out which entries are allowed within the argument buffer
    if (argInfo.bufferDataType != MTLDataTypeStruct) {
        NSLog(@"Unexpected buffer data type in Metal Function %@",func.name);
        return nil;
    }
    NSArray<MTLStructMember *> *members = argInfo.bufferStructType.members;
    if (!members) {
        NSLog(@"Unexpected buffer structure in Metal Function %@",func.name);
        return nil;
    }
    std::set<int> argEntries;
    for (MTLStructMember *mem in members) {
        argEntries.insert(mem.argumentIndex);
    }
    
    // Create a buffer to store the arguments in
    id<MTLBuffer> buff = [sceneRender->setupInfo.mtlDevice newBufferWithLength:[argEncode encodedLength] options:MTLStorageModeShared];
    buffers.insert(buff);
    [argEncode setArgumentBuffer:buff offset:0];
    
    // All of these are optional, but here's what we're expecting
    //   Uniforms
    //   UniformDrawStateA
    //   TexIndirect[WKSTextureMax]
    //   tex[WKTextureMax]
    //   [Program's custom uniforms]
    //   [Custom Uniforms]
    
//    if (argEntries.find(WKSUniformArgBuffer) != argEntries.end()) {
//        buffers.insert(frameInfo->uniformBuff);
//        [argEncode setBuffer:frameInfo->uniformBuff offset:0 atIndex:WKSUniformArgBuffer];
//    }
//    if (argEntries.find(WKSLightingArgBuffer) != argEntries.end()) {
//        buffers.insert(frameInfo->lightingBuff);
//        [argEncode setBuffer:frameInfo->lightingBuff offset:0 atIndex:WKSLightingArgBuffer];
//    }

    // Sometimes it's just boring geometry and the texture's in the base
    // Sometimes we're doing something clever and it's in the instance
    std::vector<TexInfo> allTexInfo(std::max(texInfo.size(),basicDraw->texInfo.size()));
    for (unsigned int texIndex=0;texIndex<allTexInfo.size();texIndex++) {
        if (texIndex < basicDraw->texInfo.size())
            allTexInfo[texIndex] = TexInfo(basicDraw->texInfo[texIndex]);
        if (texIndex < texInfo.size())
            allTexInfo[texIndex] = texInfo[texIndex];
    }

    // Pass in the textures (and offsets)
    // Note: We could precalculate most of then when the texture changes
    //       And we should figure out how many textures they actually have
    int numTextures = 0;
    WhirlyKitShader::TexIndirect texIndirect[WKSTextureMax];
    for (unsigned int texIndex=0;texIndex<WKSTextureMax;texIndex++) {
        TexInfo *thisTexInfo = (texIndex < allTexInfo.size()) ? &allTexInfo[texIndex] : NULL;

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
        // Note: Should we be setting up the sampler?
        TextureBaseMTL *tex = NULL;
        if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
            tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
        if (tex) {
            if (argEntries.find(WKSTextureArgBuffer+texIndex) != argEntries.end()) {
                textures.insert(tex->getMTLID());
                [argEncode setTexture:tex->getMTLID() atIndex:WKSTextureArgBuffer+texIndex];
            }
            numTextures++;
        } else {
            if (argEntries.find(WKSTextureArgBuffer+texIndex) != argEntries.end()) {
                [argEncode setTexture:nil atIndex:WKSTextureArgBuffer+texIndex];
            }
        }
    }
    if (argEntries.find(WKSTexIndirectArgBuffer) != argEntries.end()) {
        id<MTLBuffer> texIndBuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&texIndirect[0] length:sizeof(WhirlyKitShader::TexIndirect)*WKSTextureMax options:MTLStorageModeShared];
        buffers.insert(texIndBuff);
        [argEncode setBuffer:texIndBuff offset:0 atIndex:WKSTexIndirectArgBuffer];
    }

    // Set the per-drawable draw state
    if (argEntries.find(WKSUniformDrawStateArgBuffer) != argEntries.end()) {
        WhirlyKitShader::UniformDrawStateA uni;
        sceneRender->setupDrawStateA(uni);
        uni.numTextures = numTextures;
        // TODO: Turn fade back on
//        uni.fade = calcFade(frameInfo);
        uni.fade = 1.0;
        uni.clipCoords = basicDrawMTL->clipCoords;
        BasicDrawableMTL::applyUniformsToDrawState(uni, uniforms);
        // TODO: Fill in the override matrix
        bzero(&uni.singleMat,sizeof(uni.singleMat));
        id<MTLBuffer> uniABuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&uni length:sizeof(uni) options:MTLStorageModeShared];
        buffers.insert(uniABuff);
        [argEncode setBuffer:uniABuff offset:0 atIndex:WKSUniformDrawStateArgBuffer];
    }
    
    // Wire up the model instances if we have them
    if (instanceStyle == LocalStyle || instanceStyle == GPUStyle) {
        if (moving)
            uniMI.time = frameInfo->currentTime - startTime;
        id<MTLBuffer> miBuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&uniMI length:sizeof(uniMI) options:MTLStorageModeShared];
        [argEncode setBuffer:miBuff offset:0 atIndex:WKSUniformDrawStateModelInstanceArgBuffer];
        [argEncode setBuffer:instBuffer offset:0 atIndex:WKSModelInstanceArgBuffer];
        buffers.insert(miBuff);
        buffers.insert(instBuffer);
    }
    
    // Uniform blocks associated with the program
//    BasicDrawableMTL::encodeUniBlocks(frameInfo, program->uniBlocks, argEncode, argEntries, buffers);
    
    // And the uniforms passed through the drawable
//    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks, argEncode, argEntries, buffers);
    
    return buff;
}

// An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
// Also adds to the list of resources being used by this drawable
void BasicDrawableInstanceMTL::preProcess(SceneRendererMTL *sceneRender,
                id<MTLCommandBuffer> cmdBuff,
                id<MTLBlitCommandEncoder> bltEncode,
                SceneMTL *scene,
                ResourceRefsMTL &resources)
{
    // TODO: Fill this in
}

void BasicDrawableInstanceMTL::draw(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *inScene)
{
    return;
    
//    SceneMTL *scene = (SceneMTL *)inScene;
//    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
//    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
//    ProgramMTL *prog = (ProgramMTL *) frameInfo->program;
//    std::set< id<MTLBuffer> > buffers;
//
//    if (!basicDrawMTL->setupForMTL) {
//        NSLog(@"BasicDrawableInstance pointing at a bad BasicDrawable");
//        return;
//    }
//    if (!basicDrawMTL)
//        return;
//
//    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender,frameInfo,basicDrawMTL);
//
//    // Wire up the various inputs that we know about
//    // TODO: Some of these we need to override
//    for (auto vertAttr : basicDrawMTL->vertexAttributes) {
//        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
//        if (vertAttrMTL->buffer) {
//            [cmdEncode setVertexBuffer:vertAttrMTL->buffer offset:0 atIndex:vertAttrMTL->bufferIndex];
//            buffers.insert(vertAttrMTL->buffer);
//        }
//    }
//
//    // And provide defaults for the ones we don't
//    // TODO: Consolidate this
//    for (auto defAttr : basicDrawMTL->defaultAttrs)
//        [cmdEncode setVertexBytes:&defAttr.data length:sizeof(defAttr.data) atIndex:defAttr.bufferIndex];
//    for (auto defAttr : defaultAttrs)
//        [cmdEncode setVertexBytes:&defAttr.data length:sizeof(defAttr.data) atIndex:defAttr.bufferIndex];
//
//    [cmdEncode setRenderPipelineState:renderState];
//
//    // Encode the argument buffers and wire them up
//    std::set< id<MTLTexture> > textures;
//    id<MTLBuffer> argVertBuff = encodeArgumentBuffer(scene,frameInfo,prog->vertFunc,WKSVertexArgBuffer,buffers,textures);
//    if (argVertBuff)
//        [cmdEncode setVertexBuffer:argVertBuff offset:0 atIndex:WKSVertexArgBuffer];
//    id<MTLBuffer> argFragBuff = encodeArgumentBuffer(scene,frameInfo,prog->fragFunc,WKSFragmentArgBuffer,buffers,textures);
//    if (argFragBuff)
//        [cmdEncode setFragmentBuffer:argFragBuff offset:0 atIndex:WKSFragmentArgBuffer];
//    
//    // Wire up resources that we use
//    for (id<MTLBuffer> buff : buffers)
//        [cmdEncode useResource:buff usage:MTLResourceUsageRead];
//    for (id<MTLTexture> tex : textures)
//        [cmdEncode useResource:tex usage:MTLResourceUsageRead];
//
//    // Using the basic drawable geometry with a few tweaks
//    switch (instanceStyle) {
//        case ReuseStyle:
//            switch (basicDraw->type) {
//                case Lines:
//                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
//                                  vertexStart:0
//                                  vertexCount:basicDrawMTL->numPts];
//                    break;
//                case Triangles:
//                    if (!basicDrawMTL->triBuffer) {
//                        // TODO: Figure out why this happens
//                        // NSLog(@"BasicDrawableInstanceMTL: Bad basic drawable with no triangles.");
//                        return;
//                    }
//                    // This actually draws the triangles (well, in a bit)
//                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//                                          indexCount:basicDrawMTL->numTris*3
//                                           indexType:MTLIndexTypeUInt16
//                                         indexBuffer:basicDrawMTL->triBuffer
//                                   indexBufferOffset:0];
//                    break;
//                default:
//                    break;
//            }
//            break;
//        case LocalStyle:
//            // Model instancing
//            switch (basicDraw->type) {
//                case Lines:
//                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
//                                  vertexStart:0
//                                  vertexCount:basicDrawMTL->numPts
//                                instanceCount:numInst];
//                    break;
//                case Triangles:
//                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//                                          indexCount:basicDrawMTL->numTris*3
//                                           indexType:MTLIndexTypeUInt16
//                                         indexBuffer:basicDrawMTL->triBuffer
//                                   indexBufferOffset:0
//                                       instanceCount:numInst];
//                    break;
//                default:
//                    break;
//            }
//            break;
//        case GPUStyle:
//            // These use the indirect buffer defined above and, hopefully, with the number of instances copied in
//            switch (basicDraw->type) {
//                case Lines:
//                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
//                               indirectBuffer:indirectBuffer
//                         indirectBufferOffset:0];
//                    break;
//                case Triangles:
//                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//                                           indexType:MTLIndexTypeUInt16
//                                         indexBuffer:basicDrawMTL->triBuffer
//                                   indexBufferOffset:0
//                                      indirectBuffer:indirectBuffer
//                                indirectBufferOffset:0];
//                    break;
//                default:
//                    break;
//            }
//            break;
//    }
}
    
id<MTLRenderPipelineState> BasicDrawableInstanceMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,RendererFrameInfoMTL *frameInfo,BasicDrawableMTL *basicDrawMTL)
{
    if (renderState)
        return renderState;
    
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,frameInfo);
    renderDesc.vertexDescriptor = basicDrawMTL->getVertexDescriptor(program->vertFunc,basicDrawMTL->defaultAttrs);
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];
    
    // We also need to set up our default attribute overrides
    updateColorDefaultAttr();

    // Set up a render state
    NSError *err = nil;
    renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableInstanceMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }
    
    return renderState;
}
    
}
