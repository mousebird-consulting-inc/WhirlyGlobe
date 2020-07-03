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
    : BasicDrawableInstance(name), Drawable(name), renderState(nil), setupForMTL(false), numInst(0), vertHasTextures(false),fragHasTextures(false),vertHasLighting(false),fragHasLighting(false)
{
}

BasicDrawableInstanceMTL::~BasicDrawableInstanceMTL()
{
}

void BasicDrawableInstanceMTL::setColor(RGBAColor inColor)
{
    BasicDrawableInstance::setColor(inColor);
}

void BasicDrawableInstanceMTL::setupForRenderer(const RenderSetupInfo *inSetupInfo,Scene *inScene)
{
    if (setupForMTL)
        return;
    
    RenderSetupInfoMTL *setupInfo = (RenderSetupInfoMTL *)inSetupInfo;
    SceneMTL *scene = (SceneMTL *)inScene;
    BufferBuilderMTL buffBuild(setupInfo);
    
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(scene->getDrawable(masterID).get());
    if (!basicDrawMTL)
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
        instBuffer = buffBuild.addData(&insts[0], bufferSize);
        numInst = insts.size();
    } else if (instanceStyle == GPUStyle) {
        // Basic values for the uniforms
        bzero(&uniMI,sizeof(uniMI));
        uniMI.time = 0.0;
        uniMI.useInstanceColor = false;
        uniMI.hasMotion = false;
    }
    
    // Build the argument buffers with all their attendent memory, ready to copy into
    setupArgBuffers(setupInfo->mtlDevice,setupInfo,scene,buffBuild);
        
    // And let's fault in the vertex descriptor as well
    ProgramMTL *program = (ProgramMTL *)scene->getProgram(programID);
    if (program && program->vertFunc)
        basicDrawMTL->getVertexDescriptor(program->vertFunc, defaultAttrs);
    
    // Last, we'll set up the default attributes in the new buffer as well
    for (auto &defAttr : defaultAttrs)
        defAttr.buffer = buffBuild.addData(&defAttr.data, sizeof(defAttr.data));
    unsigned char data[4];
    color.asUChar4(&data[0]);
    colorBuffer = buffBuild.addData(&data[0], 4);
    
    // Construct the buffer we've been adding to
    mainBuffer = buffBuild.buildBuffer();
    baseMainBuffer = basicDrawMTL->mainBuffer;
    
    setupForMTL = true;
}

void BasicDrawableInstanceMTL::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *inScene)
{
    setupForMTL = false;

    instBuffer.reset();
    indirectBuffer.reset();
    renderState = nil;
    calcRenderState = nil;
    defaultAttrs.clear();
    mainBuffer.reset();
    baseMainBuffer.reset();
    vertABInfo.reset();
    fragABInfo.reset();
}

id<MTLRenderPipelineState> BasicDrawableInstanceMTL::getRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget,BasicDrawableMTL *basicDrawMTL)
{
    if (renderState)
        return renderState;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;

    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,program,renderTarget);
    renderDesc.vertexDescriptor = basicDrawMTL->getVertexDescriptor(program->vertFunc,defaultAttrs);
    if (!name.empty())
        renderDesc.label = [NSString stringWithFormat:@"%s",name.c_str()];

    // Update the attribute defaults if they're present
    if (hasColor) {
        VertexAttributeMTL *colorAttrMTL = basicDrawMTL->findVertexAttribute(a_colorNameID);
        if (colorAttrMTL) {
            BasicDrawableMTL::AttributeDefault defAttr;
            defAttr.buffer = colorBuffer;
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

    // Set up a render state
    NSError *err = nil;
    renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    if (err) {
        NSLog(@"BasicDrawableMTL: Failed to set up render state because:\n%@",err);
        return nil;
    }

    return renderState;
}

void BasicDrawableInstanceMTL::setupArgBuffers(id<MTLDevice> mtlDevice,RenderSetupInfoMTL *setupInfo,SceneMTL *scene,BufferBuilderMTL &buffBuild)
{
    ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programID);
    
    // All of these are optional, but here's what we're expecting
    //   Uniforms
    //   Lighting
    //   UniformDrawStateA
    //   UniformModelInstance
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
                                                                  WhirlyKitShader::WKSVertexArgBuffer,
                                                                  buffBuild));
        vertHasTextures = vertABInfo->hasConstant("hasTextures");
        vertHasLighting = vertABInfo->hasConstant("hasLighting");
        if (vertHasTextures)
            vertTexInfo = ArgBuffRegularTexturesMTLRef(new ArgBuffRegularTexturesMTL(mtlDevice, setupInfo, prog->vertFunc, WhirlyKitShader::WKSVertTextureArgBuffer, buffBuild));
    }
    if (prog->fragFunc) {
        fragABInfo = ArgBuffContentsMTLRef(new ArgBuffContentsMTL(mtlDevice,
                                                                  setupInfo,
                                                                  prog->fragFunc,
                                                                  WhirlyKitShader::WKSFragmentArgBuffer,
                                                                  buffBuild));
        fragHasTextures = fragABInfo->hasConstant("hasTextures");
        fragHasLighting = fragABInfo->hasConstant("hasLighting");
        if (fragHasTextures)
            fragTexInfo = ArgBuffRegularTexturesMTLRef(new ArgBuffRegularTexturesMTL(mtlDevice, setupInfo, prog->fragFunc, WhirlyKitShader::WKSFragTextureArgBuffer, buffBuild));
    }
}

// Render state for calculation pass
id<MTLRenderPipelineState> BasicDrawableInstanceMTL::getCalcRenderPipelineState(SceneRendererMTL *sceneRender,Scene *scene,ProgramMTL *program,RenderTargetMTL *renderTarget)
{
    if (calcRenderState)
        return calcRenderState;
    
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = sceneRender->defaultRenderPipelineState(sceneRender,(ProgramMTL *)program,renderTarget);
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

//void BasicDrawableInstanceMTL::calculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
//{
//    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
//    if (!basicDrawMTL) {
//        NSLog(@"BasicDrawableInstance pointing at a bad BasicDrawable");
//        return;
//    }
//    if (!basicDrawMTL)
//        return;
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
//}
//
//id<MTLBuffer> BasicDrawableInstanceMTL::encodeArgumentBuffer(SceneMTL *scene,
//                                                            RendererFrameInfoMTL *frameInfo,
//                                                            id<MTLFunction> func,
//                                                            int bufferIndex,
//                                                            std::set< id<MTLBuffer> > &buffers,
//                                                            std::set< id<MTLTexture> > &textures)
//{
//    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
//    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
//    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
//
//    MTLAutoreleasedArgument argInfo;
//    id<MTLArgumentEncoder> argEncode = [func newArgumentEncoderWithBufferIndex:bufferIndex reflection:&argInfo];
//    if (!argEncode)
//        return nil;
//
//    // Figure out which entries are allowed within the argument buffer
//    if (argInfo.bufferDataType != MTLDataTypeStruct) {
//        NSLog(@"Unexpected buffer data type in Metal Function %@",func.name);
//        return nil;
//    }
//    NSArray<MTLStructMember *> *members = argInfo.bufferStructType.members;
//    if (!members) {
//        NSLog(@"Unexpected buffer structure in Metal Function %@",func.name);
//        return nil;
//    }
//    std::set<int> argEntries;
//    for (MTLStructMember *mem in members) {
//        argEntries.insert(mem.argumentIndex);
//    }
//
//    // Create a buffer to store the arguments in
//    id<MTLBuffer> buff = [sceneRender->setupInfo.mtlDevice newBufferWithLength:[argEncode encodedLength] options:MTLStorageModeShared];
//    buffers.insert(buff);
//    [argEncode setArgumentBuffer:buff offset:0];
//
//    // All of these are optional, but here's what we're expecting
//    //   Uniforms
//    //   UniformDrawStateA
//    //   TexIndirect[WKSTextureMax]
//    //   tex[WKTextureMax]
//    //   [Program's custom uniforms]
//    //   [Custom Uniforms]
//
////    if (argEntries.find(WKSUniformArgBuffer) != argEntries.end()) {
////        buffers.insert(frameInfo->uniformBuff);
////        [argEncode setBuffer:frameInfo->uniformBuff offset:0 atIndex:WKSUniformArgBuffer];
////    }
////    if (argEntries.find(WKSLightingArgBuffer) != argEntries.end()) {
////        buffers.insert(frameInfo->lightingBuff);
////        [argEncode setBuffer:frameInfo->lightingBuff offset:0 atIndex:WKSLightingArgBuffer];
////    }
//
//    // Sometimes it's just boring geometry and the texture's in the base
//    // Sometimes we're doing something clever and it's in the instance
//    std::vector<TexInfo> allTexInfo(std::max(texInfo.size(),basicDraw->texInfo.size()));
//    for (unsigned int texIndex=0;texIndex<allTexInfo.size();texIndex++) {
//        if (texIndex < basicDraw->texInfo.size())
//            allTexInfo[texIndex] = TexInfo(basicDraw->texInfo[texIndex]);
//        if (texIndex < texInfo.size())
//            allTexInfo[texIndex] = texInfo[texIndex];
//    }
//
//    // Pass in the textures (and offsets)
//    // Note: We could precalculate most of then when the texture changes
//    //       And we should figure out how many textures they actually have
//    int numTextures = 0;
//    WhirlyKitShader::TexIndirect texIndirect[WKSTextureMax];
//    for (unsigned int texIndex=0;texIndex<WKSTextureMax;texIndex++) {
//        TexInfo *thisTexInfo = (texIndex < allTexInfo.size()) ? &allTexInfo[texIndex] : NULL;
//
//        // Figure out texture adjustment for parent textures
//        float texScale = 1.0;
//        Vector2f texOffset(0.0,0.0);
//        // Adjust for border pixels
//        if (thisTexInfo && thisTexInfo->borderTexel > 0 && thisTexInfo->size > 0) {
//            texScale = (thisTexInfo->size - 2 * thisTexInfo->borderTexel) / (double)thisTexInfo->size;
//            float offset = thisTexInfo->borderTexel / (double)thisTexInfo->size;
//            texOffset = Vector2f(offset,offset);
//        }
//        // Adjust for a relative texture lookup (using lower zoom levels)
//        if (thisTexInfo && thisTexInfo->relLevel > 0) {
//            texScale = texScale/(1<<thisTexInfo->relLevel);
//            texOffset = Vector2f(texScale*thisTexInfo->relX,texScale*thisTexInfo->relY) + texOffset;
//        }
//
//        // Calculate offset and scales
//        WhirlyKitShader::TexIndirect &texInd = texIndirect[texIndex];
//        texInd.offset[0] = texOffset.x();  texInd.offset[1] = texOffset.y();
//        texInd.scale[0] = texScale; texInd.scale[1] = texScale;
//
//        // And the texture itself
//        // Note: Should we be setting up the sampler?
//        TextureBaseMTL *tex = NULL;
//        if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
//            tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
//        if (tex) {
//            if (argEntries.find(WKSTextureArgBuffer+texIndex) != argEntries.end()) {
//                textures.insert(tex->getMTLID());
//                [argEncode setTexture:tex->getMTLID() atIndex:WKSTextureArgBuffer+texIndex];
//            }
//            numTextures++;
//        } else {
//            if (argEntries.find(WKSTextureArgBuffer+texIndex) != argEntries.end()) {
//                [argEncode setTexture:nil atIndex:WKSTextureArgBuffer+texIndex];
//            }
//        }
//    }
//    if (argEntries.find(WKSTexIndirectArgBuffer) != argEntries.end()) {
//        id<MTLBuffer> texIndBuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&texIndirect[0] length:sizeof(WhirlyKitShader::TexIndirect)*WKSTextureMax options:MTLStorageModeShared];
//        buffers.insert(texIndBuff);
//        [argEncode setBuffer:texIndBuff offset:0 atIndex:WKSTexIndirectArgBuffer];
//    }
//
//    // Set the per-drawable draw state
//    if (argEntries.find(WKSUniformDrawStateArgBuffer) != argEntries.end()) {
//        WhirlyKitShader::UniformDrawStateA uni;
//        sceneRender->setupDrawStateA(uni);
//        uni.numTextures = numTextures;
//        // TODO: Turn fade back on
////        uni.fade = calcFade(frameInfo);
//        uni.fade = 1.0;
//        uni.clipCoords = basicDrawMTL->clipCoords;
//        BasicDrawableMTL::applyUniformsToDrawState(uni, uniforms);
//        // TODO: Fill in the override matrix
//        bzero(&uni.singleMat,sizeof(uni.singleMat));
//        id<MTLBuffer> uniABuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&uni length:sizeof(uni) options:MTLStorageModeShared];
//        buffers.insert(uniABuff);
//        [argEncode setBuffer:uniABuff offset:0 atIndex:WKSUniformDrawStateArgBuffer];
//    }
//
//    // Wire up the model instances if we have them
//    if (instanceStyle == LocalStyle || instanceStyle == GPUStyle) {
//        if (moving)
//            uniMI.time = frameInfo->currentTime - startTime;
//        id<MTLBuffer> miBuff = [sceneRender->setupInfo.mtlDevice newBufferWithBytes:&uniMI length:sizeof(uniMI) options:MTLStorageModeShared];
//        [argEncode setBuffer:miBuff offset:0 atIndex:WKSUniformDrawStateModelInstanceArgBuffer];
//        [argEncode setBuffer:instBuffer offset:0 atIndex:WKSModelInstanceArgBuffer];
//        buffers.insert(miBuff);
//        buffers.insert(instBuffer);
//    }
//
//    // Uniform blocks associated with the program
////    BasicDrawableMTL::encodeUniBlocks(frameInfo, program->uniBlocks, argEncode, argEntries, buffers);
//
//    // And the uniforms passed through the drawable
////    BasicDrawableMTL::encodeUniBlocks(frameInfo, uniBlocks, argEncode, argEntries, buffers);
//
//    return buff;
//}

// An all-purpose pre-render that sets up textures, uniforms and such in preparation for rendering
// Also adds to the list of resources being used by this drawable
void BasicDrawableInstanceMTL::preProcess(SceneRendererMTL *sceneRender,
                id<MTLCommandBuffer> cmdBuff,
                id<MTLBlitCommandEncoder> bltEncode,
                SceneMTL *scene,
                ResourceRefsMTL &resources)
{
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());

    if (texturesChanged || valuesChanged) {
        ProgramMTL *prog = (ProgramMTL *)scene->getProgram(programID);
        id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;

        if (!prog) {
            NSLog(@"Drawable %s missing program.",name.c_str());
            return;
        }

        if (texturesChanged && (vertTexInfo || fragTexInfo)) {
            activeTextures.clear();

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

                // And the texture itself
                TextureBaseMTL *tex = NULL;
                if (thisTexInfo && thisTexInfo->texId != EmptyIdentity)
                    tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(thisTexInfo->texId));
                if (tex)
                    activeTextures.push_back(tex->getMTLID());
                if (vertTexInfo)
                    vertTexInfo->addTexture(texOffset, Point2f(texScale,texScale), tex != nil ? tex->getMTLID() : nil);
                if (fragTexInfo)
                    fragTexInfo->addTexture(texOffset, Point2f(texScale,texScale), tex != nil ? tex->getMTLID() : nil);
            }
            if (vertTexInfo) {
                vertTexInfo->updateBuffer(mtlDevice, bltEncode);
            }
            if (fragTexInfo) {
                fragTexInfo->updateBuffer(mtlDevice, bltEncode);
            }
        }

        if (valuesChanged) {
            if (vertABInfo)
                vertABInfo->startEncoding(mtlDevice);
            if (fragABInfo)
                fragABInfo->startEncoding(mtlDevice);
            
            // Put together a set of uniform blocks to apply
            std::map<int,const BasicDrawable::UniformBlock *> allUniBlocks;
            
            // First the ones on the program, then those on the BasicDrawable, then ours
            for (const BasicDrawable::UniformBlock &uniBlock : prog->uniBlocks)
                allUniBlocks[uniBlock.bufferID] = &uniBlock;
            for (const BasicDrawable::UniformBlock &uniBlock : basicDraw->uniBlocks)
                allUniBlocks[uniBlock.bufferID] = &uniBlock;
            for (const BasicDrawable::UniformBlock &uniBlock : uniBlocks)
                allUniBlocks[uniBlock.bufferID] = &uniBlock;

            for (auto it : allUniBlocks) {
                auto uniBlock = it.second;
                vertABInfo->updateEntry(mtlDevice,bltEncode,uniBlock->bufferID, (void *)uniBlock->blockData->getRawData(), uniBlock->blockData->getLen());
                fragABInfo->updateEntry(mtlDevice,bltEncode,uniBlock->bufferID, (void *)uniBlock->blockData->getRawData(), uniBlock->blockData->getLen());
            }
            
            // Per drawable draw state in its own buffer
            // Has to update if either textures or values updated
            WhirlyKitShader::UniformDrawStateA uni;
            sceneRender->setupDrawStateA(uni);
            // TODO: Move into shader
            uni.fade = 1.0;
    //        uni.fade = calcFade(frameInfo);
            uni.clipCoords = basicDraw->clipCoords;
            BasicDrawableMTL::applyUniformsToDrawState(uni,uniforms);
            CopyIntoMtlFloat4x4(uni.singleMat, *getMatrix());
            if (vertABInfo)
                vertABInfo->updateEntry(mtlDevice,bltEncode, WhirlyKitShader::WKSUniformDrawStateEntry, &uni, sizeof(uni));
            if (fragABInfo)
                fragABInfo->updateEntry(mtlDevice,bltEncode, WhirlyKitShader::WKSUniformDrawStateEntry, &uni, sizeof(uni));
            
            // And do the Model Instance specific stuff
            if (vertABInfo)
                vertABInfo->updateEntry(mtlDevice, bltEncode, WhirlyKitShader::WKSVertModelInstanceArgBuffer, &uniMI, sizeof(uniMI));
            
            if (vertABInfo) {
                vertABInfo->endEncoding(mtlDevice, bltEncode);
            }
            if (fragABInfo) {
                fragABInfo->endEncoding(mtlDevice, bltEncode);
            }
        }
        

        texturesChanged = false;
        valuesChanged = false;
    }

    resources.addTextures(activeTextures);
}

void BasicDrawableInstanceMTL::encodeDirectCalculate(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    // TODO: Fill this in
}

void BasicDrawableInstanceMTL::encodeDirect(RendererFrameInfoMTL *frameInfo,id<MTLRenderCommandEncoder> cmdEncode,Scene *scene)
{
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    RenderTargetMTL *renderTarget = frameInfo->renderTarget;
    if (!basicDrawMTL->setupForMTL) {
        NSLog(@"BasicDrawableInstance pointing at a bad BasicDrawable");
        return;
    }
    if (!basicDrawMTL)
        return;

    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender, scene, program, renderTarget, basicDrawMTL);
    
    // Wire up the various inputs that we know about
    for (auto vertAttr : basicDrawMTL->vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer && (vertAttrMTL->bufferIndex >= 0))
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer->buffer offset:vertAttrMTL->buffer->offset atIndex:vertAttrMTL->bufferIndex];
    }
    
    // And provide defaults for the ones we don't.  Both in the basic drawable and our instance
    for (auto defAttr : basicDrawMTL->defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    for (auto defAttr : defaultAttrs)
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    
    [cmdEncode setRenderPipelineState:renderState];
    
    // Everything takes the uniforms
    [cmdEncode setVertexBuffer:sceneRender->setupInfo.uniformBuff->buffer offset:sceneRender->setupInfo.uniformBuff->offset atIndex:WhirlyKitShader::WKSVertUniformArgBuffer];
    [cmdEncode setFragmentBuffer:sceneRender->setupInfo.uniformBuff->buffer offset:sceneRender->setupInfo.uniformBuff->offset atIndex:WhirlyKitShader::WKSFragUniformArgBuffer];

    // Some shaders take the lighting
    if (vertHasLighting) {
        [cmdEncode setVertexBuffer:sceneRender->setupInfo.lightingBuff->buffer offset:sceneRender->setupInfo.lightingBuff->offset atIndex:WhirlyKitShader::WKSVertLightingArgBuffer];
    }
    if (fragHasLighting) {
        [cmdEncode setFragmentBuffer:sceneRender->setupInfo.lightingBuff->buffer offset:sceneRender->setupInfo.lightingBuff->offset atIndex:WhirlyKitShader::WKSFragLightingArgBuffer];
    }
    
    // Instances go to the vertex shader if they're present
    if (instBuffer) {
        [cmdEncode setVertexBuffer:instBuffer->buffer offset:instBuffer->offset atIndex:WhirlyKitShader::WKSVertModelInstanceArgBuffer];
    }
    
    // More flexible data structures passed in to the shaders
    if (vertABInfo) {
        BufferEntryMTLRef buff = vertABInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTLRef buff = fragABInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSFragmentArgBuffer];
    }

    // Textures may or may not be passed in to shaders
    if (vertTexInfo) {
        BufferEntryMTLRef buff = vertTexInfo->getBuffer();
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSVertTextureArgBuffer];
    }
    if (fragTexInfo) {
        BufferEntryMTLRef buff = fragTexInfo->getBuffer();
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSFragTextureArgBuffer];
    }

    // Using the basic drawable geometry with a few tweaks
    switch (instanceStyle) {
        case ReuseStyle:
            switch (basicDraw->type) {
                case Lines:
                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
                                  vertexStart:0
                                  vertexCount:basicDrawMTL->numPts];
                    break;
                case Triangles:
                    if (!basicDrawMTL->triBuffer) {
                        // TODO: Figure out why this happens
                        // NSLog(@"BasicDrawableInstanceMTL: Bad basic drawable with no triangles.");
                        return;
                    }
                    // This actually draws the triangles (well, in a bit)
                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                          indexCount:basicDrawMTL->numTris*3
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:basicDrawMTL->triBuffer->buffer
                                   indexBufferOffset:basicDrawMTL->triBuffer->offset];
                    break;
                default:
                    break;
            }
            break;
        case LocalStyle:
            // Model instancing
            switch (basicDraw->type) {
                case Lines:
                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
                                  vertexStart:0
                                  vertexCount:basicDrawMTL->numPts
                                instanceCount:numInst];
                    break;
                case Triangles:
                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                          indexCount:basicDrawMTL->numTris*3
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:basicDrawMTL->triBuffer->buffer
                                   indexBufferOffset:basicDrawMTL->triBuffer->offset
                                       instanceCount:numInst];
                    break;
                default:
                    break;
            }
            break;
        case GPUStyle:
            // These use the indirect buffer defined above and, hopefully, with the number of instances copied in
            switch (basicDraw->type) {
                case Lines:
                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
                               indirectBuffer:indirectBuffer->buffer
                         indirectBufferOffset:indirectBuffer->offset];
                    break;
                case Triangles:
                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:basicDrawMTL->triBuffer->buffer
                                   indexBufferOffset:basicDrawMTL->triBuffer->offset
                                      indirectBuffer:indirectBuffer->buffer
                                indirectBufferOffset:indirectBuffer->offset];
                    break;
                default:
                    break;
            }
            break;
    }
}

void BasicDrawableInstanceMTL::encodeIndirectCalculate(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget,ResourceRefsMTL &resources)
{
    // TODO: Fill this in
}

void BasicDrawableInstanceMTL::encodeIndirect(id<MTLIndirectRenderCommand> cmdEncode,SceneRendererMTL *sceneRender,Scene *scene,RenderTargetMTL *renderTarget,ResourceRefsMTL &resources)
{
    BasicDrawableMTL *basicDrawMTL = dynamic_cast<BasicDrawableMTL *>(basicDraw.get());
    ProgramMTL *program = (ProgramMTL *)scene->getProgram(programID);
    if (!basicDrawMTL || !basicDrawMTL->setupForMTL) {
        NSLog(@"BasicDrawableInstance pointing at a bad BasicDrawable");
        return;
    }

    id<MTLRenderPipelineState> renderState = getRenderPipelineState(sceneRender, scene, program, renderTarget, basicDrawMTL);

    // Wire up the various inputs that we know about
    for (auto vertAttr : basicDrawMTL->vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        if (vertAttrMTL->buffer && (vertAttrMTL->bufferIndex >= 0)) {
            resources.addEntry(vertAttrMTL->buffer);
            [cmdEncode setVertexBuffer:vertAttrMTL->buffer->buffer offset:vertAttrMTL->buffer->offset atIndex:vertAttrMTL->bufferIndex];
        }
    }
    
    // And provide defaults for the ones we don't.  Both in the basic drawable and our instance
    for (auto defAttr : basicDrawMTL->defaultAttrs) {
        resources.addEntry(defAttr.buffer);
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    }
    for (auto defAttr : defaultAttrs) {
        resources.addEntry(defAttr.buffer);
        [cmdEncode setVertexBuffer:defAttr.buffer->buffer offset:defAttr.buffer->offset atIndex:defAttr.bufferIndex];
    }
    
    [cmdEncode setRenderPipelineState:renderState];

    // Everything takes the uniforms
    resources.addEntry(sceneRender->setupInfo.uniformBuff);
    [cmdEncode setVertexBuffer:sceneRender->setupInfo.uniformBuff->buffer offset:sceneRender->setupInfo.uniformBuff->offset atIndex:WhirlyKitShader::WKSVertUniformArgBuffer];
    [cmdEncode setFragmentBuffer:sceneRender->setupInfo.uniformBuff->buffer offset:sceneRender->setupInfo.uniformBuff->offset atIndex:WhirlyKitShader::WKSFragUniformArgBuffer];

    // Some shaders take the lighting
    if (vertHasLighting) {
        resources.addEntry(sceneRender->setupInfo.lightingBuff);
        [cmdEncode setVertexBuffer:sceneRender->setupInfo.lightingBuff->buffer offset:sceneRender->setupInfo.lightingBuff->offset atIndex:WhirlyKitShader::WKSVertLightingArgBuffer];
    }
    if (fragHasLighting) {
        resources.addEntry(sceneRender->setupInfo.lightingBuff);
        [cmdEncode setFragmentBuffer:sceneRender->setupInfo.lightingBuff->buffer offset:sceneRender->setupInfo.lightingBuff->offset atIndex:WhirlyKitShader::WKSFragLightingArgBuffer];
    }

    // More flexible data structures passed in to the shaders
    if (vertABInfo) {
        BufferEntryMTLRef buff = vertABInfo->getBuffer();
        resources.addEntry(buff);
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSVertexArgBuffer];
    }
    if (fragABInfo) {
        BufferEntryMTLRef buff = fragABInfo->getBuffer();
        resources.addEntry(buff);
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSFragmentArgBuffer];
    }

    // Textures may or may not be passed in to shaders
    if (vertTexInfo) {
        BufferEntryMTLRef buff = vertTexInfo->getBuffer();
        resources.addEntry(buff);
        [cmdEncode setVertexBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSVertTextureArgBuffer];
    }
    if (fragTexInfo) {
        BufferEntryMTLRef buff = fragTexInfo->getBuffer();
        resources.addEntry(buff);
        [cmdEncode setFragmentBuffer:buff->buffer offset:buff->offset atIndex:WhirlyKitShader::WKSFragTextureArgBuffer];
    }

    // Using the basic drawable geometry with a few tweaks
    switch (instanceStyle) {
        case ReuseStyle:
            switch (basicDraw->type) {
                case Lines:
                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
                                  vertexStart:0
                                  vertexCount:basicDrawMTL->numPts
                                instanceCount:1
                                 baseInstance:0];
                    break;
                case Triangles:
                    if (!basicDrawMTL->triBuffer) {
                        // TODO: Figure out why this happens
                        // NSLog(@"BasicDrawableInstanceMTL: Bad basic drawable with no triangles.");
                        return;
                    }
                    // This actually draws the triangles (well, in a bit)
                    resources.addEntry(basicDrawMTL->triBuffer);
                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                          indexCount:basicDrawMTL->numTris*3
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:basicDrawMTL->triBuffer->buffer
                                   indexBufferOffset:basicDrawMTL->triBuffer->offset
                                       instanceCount:1
                                          baseVertex:0
                                        baseInstance:0];
                    break;
                default:
                    break;
            }
            break;
        case LocalStyle:
            // Model instancing
            switch (basicDraw->type) {
                case Lines:
                    [cmdEncode drawPrimitives:MTLPrimitiveTypeLine
                                  vertexStart:0
                                  vertexCount:basicDrawMTL->numPts
                                instanceCount:numInst
                                 baseInstance:0];
                    break;
                case Triangles:
                    resources.addEntry(basicDrawMTL->triBuffer);
                    [cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                          indexCount:basicDrawMTL->numTris*3
                                           indexType:MTLIndexTypeUInt16
                                         indexBuffer:basicDrawMTL->triBuffer->buffer
                                   indexBufferOffset:basicDrawMTL->triBuffer->offset
                                       instanceCount:numInst
                                          baseVertex:0
                                        baseInstance:0];
                    break;
                default:
                    break;
            }
            break;
        case GPUStyle:
            NSLog(@"GPU Style BasicDrawableInstance needs to be implemented");
            break;
    }
}
    
}