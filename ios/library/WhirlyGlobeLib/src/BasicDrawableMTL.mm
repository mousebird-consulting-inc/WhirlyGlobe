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

namespace WhirlyKit
{
    
BasicDrawableMTL::BasicDrawableMTL(const std::string &name)
    : BasicDrawable(name), triBuffer(nil), setupForMTL(false), numPts(0), numTris(0)
{
}
 
// Note: Debugging
bool BasicDrawableMTL::isOn(RendererFrameInfo *frameInfo) const
{
    return true;
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
            vertAttrMTL->clear();
        }
    }
    
    // And put the triangles in their own
    // Note: Could use 2 bytes some of the time
    int bufferSize = 3*4*tris.size();
    numTris = tris.size();
    if (bufferSize > 0) {
        triBuffer = [setupInfo->mtlDevice newBufferWithBytes:&tris[0] length:bufferSize options:MTLStorageModeShared];
        tris.clear();
    }
    
    setupForMTL = true;
}

void BasicDrawableMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
    for (VertexAttribute *vertAttr : vertexAttributes) {
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)vertAttr;
        vertAttrMTL->buffer = nil;
    }
}

void BasicDrawableMTL::draw(RendererFrameInfo *inFrameInfo,Scene *inScene)
{
    // Note: Should cull these before this
    if (!triBuffer)
        return;
    
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    SceneMTL *scene = (SceneMTL *)inScene;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = [[MTLRenderPipelineDescriptor alloc] init];
    renderDesc.vertexFunction = program->vertFunc;
    renderDesc.fragmentFunction = program->fragFunc;
    
    // Make up a vertex descriptor
    MTLVertexDescriptor *vertDesc = [[MTLVertexDescriptor alloc] init];
    
    // Link up the vertex attributes with the buffers
    NSArray<MTLAttribute *> *vertAttrsMTL = renderDesc.vertexFunction.stageInputAttributes;
    int which = 0;
    for (MTLAttribute *vertAttrMTL : vertAttrsMTL) {
        // Find the matching attribute
        int nameID = StringIndexer::getStringID([vertAttrMTL.name cStringUsingEncoding:NSASCIIStringEncoding]);
        bool found = false;
        for (VertexAttribute *vertAttr : vertexAttributes) {
            if (vertAttr->nameID == nameID) {
                MTLVertexAttributeDescriptor *attrDesc = [[MTLVertexAttributeDescriptor alloc] init];
                VertexAttributeMTL *ourVertAttr = (VertexAttributeMTL *)vertAttr;
                
                // Describe the vertex attribute
                attrDesc.format = ourVertAttr->formatMTL();
                attrDesc.bufferIndex = which;
                attrDesc.offset = 0;

                // Add in the buffer
                MTLVertexBufferLayoutDescriptor *layoutDesc = [[MTLVertexBufferLayoutDescriptor alloc] init];
                if (ourVertAttr->buffer) {
                    // Normal case with one per vertex
                    layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
                    layoutDesc.stepRate = 1;
                    layoutDesc.stride = ourVertAttr->sizeMTL();

                    [frameInfo->cmdEncode setVertexBuffer:ourVertAttr->buffer offset:0 atIndex:which];
                } else {
                    // Provides just a default value for the whole thing
                    layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
                    layoutDesc.stepRate = 0;
                    layoutDesc.stride = ourVertAttr->sizeMTL();
                    
                    // Provide the default
                    switch (ourVertAttr->getDataType()) {
                        case BDFloatType:
                        {
                            attrDesc.format = MTLVertexFormatFloat;
                            layoutDesc.stride = 4;
                            float val = ourVertAttr->defaultData.floatVal;
                            [frameInfo->cmdEncode setVertexBytes:&val length:4 atIndex:which];
                        }
                            break;
                        case BDFloat2Type:
                        {
                            attrDesc.format = MTLVertexFormatFloat2;
                            layoutDesc.stride = 8;
                            auto val = ourVertAttr->defaultData.vec2;
                            [frameInfo->cmdEncode setVertexBytes:&val length:2*4 atIndex:which];
                        }
                            break;
                        case BDFloat3Type:
                        {
                            attrDesc.format = MTLVertexFormatFloat3;
                            layoutDesc.stride = 12;
                            auto val = ourVertAttr->defaultData.vec3;
                            [frameInfo->cmdEncode setVertexBytes:&val length:3*4 atIndex:which];
                        }
                            break;
                        case BDFloat4Type:
                        {
                            attrDesc.format = MTLVertexFormatFloat4;
                            layoutDesc.stride = 16;
                            auto val = ourVertAttr->defaultData.vec4;
                            [frameInfo->cmdEncode setVertexBytes:&val length:4*4 atIndex:which];
                        }
                            break;
                        case BDIntType:
                        {
                            attrDesc.format = MTLVertexFormatInt;
                            layoutDesc.stride = 4;
                            auto val = ourVertAttr->defaultData.intVal;
                            [frameInfo->cmdEncode setVertexBytes:&val length:4 atIndex:which];
                        }
                            break;
                        case BDChar4Type:
                        {
                            attrDesc.format = MTLVertexFormatUChar4;
                            layoutDesc.stride = 4;
                            auto val = ourVertAttr->defaultData.color;
                            [frameInfo->cmdEncode setVertexBytes:val length:4 atIndex:which];
                        }
                        default:
                            break;
                    }
                }
                vertDesc.attributes[which] = attrDesc;
                vertDesc.layouts[which] = layoutDesc;
                
                
                which++;
                found = true;
                break;
            }
        }
        
        // We don't have this one at all, so let's provide some sort of default anyway
        // This happens with texture coordinates
        if (!found) {
            MTLVertexAttributeDescriptor *attrDesc = [[MTLVertexAttributeDescriptor alloc] init];
            MTLVertexBufferLayoutDescriptor *layoutDesc = [[MTLVertexBufferLayoutDescriptor alloc] init];
            switch (vertAttrMTL.attributeType) {
                case MTLDataTypeFloat:
                {
                    attrDesc.format = MTLVertexFormatFloat;
                    layoutDesc.stride = 4;
                    float val = 0.0;
                    [frameInfo->cmdEncode setVertexBytes:&val length:4 atIndex:which];
                }
                    break;
                case MTLDataTypeFloat2:
                {
                    attrDesc.format = MTLVertexFormatFloat2;
                    layoutDesc.stride = 8;
                    float val[2] = {0.0,0.0};
                    [frameInfo->cmdEncode setVertexBytes:val length:2*4 atIndex:which];
                }
                    break;
                case MTLDataTypeFloat3:
                {
                    attrDesc.format = MTLVertexFormatFloat3;
                    layoutDesc.stride = 12;
                    float val[3] = {0.0,0.0,0.0};
                    [frameInfo->cmdEncode setVertexBytes:val length:3*4 atIndex:which];
                }
                   break;
                case MTLDataTypeFloat4:
                {
                    attrDesc.format = MTLVertexFormatFloat4;
                    layoutDesc.stride = 16;
                    float val[4] = {0.0,0.0,0.0};
                    [frameInfo->cmdEncode setVertexBytes:val length:4*4 atIndex:which];
                }
                    break;
                case MTLDataTypeInt:
                {
                    attrDesc.format = MTLVertexFormatInt;
                    layoutDesc.stride = 4;
                    int val = 0;
                    [frameInfo->cmdEncode setVertexBytes:&val length:4 atIndex:which];
                }
                    break;
                default:
                    break;
            }
            attrDesc.bufferIndex = which;
            attrDesc.offset = 0;
            vertDesc.attributes[which] = attrDesc;
            
            layoutDesc.stepFunction = MTLVertexStepFunctionConstant;
            layoutDesc.stepRate = 0;
            vertDesc.layouts[which] = layoutDesc;
            
            which++;
        }
    }
    
    renderDesc.vertexDescriptor = vertDesc;
    // TODO: Should be from the target
    renderDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    // Set up a render state
    // Note: Also querying the arguments and parsing those
    NSError *err = nil;
    MTLRenderPipelineReflection *reflInfo = nil;
    id<MTLRenderPipelineState> renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc options:MTLPipelineOptionArgumentInfo|MTLPipelineOptionBufferTypeInfo reflection:&reflInfo error:&err];
    
    [frameInfo->cmdEncode setRenderPipelineState:renderState];

    // Look for the vertex uniforms
    // Note: Put these in the ProgramMTL
    int texIndex = 0;
    NSArray<MTLArgument *> *vertArgs = reflInfo.vertexArguments;
    for (MTLArgument *vertArg : vertArgs) {
        if ([vertArg.name isEqualToString:@"uniforms"]) {
            [frameInfo->cmdEncode setVertexBuffer:frameInfo->uniformTriBuffer offset:0 atIndex:vertArg.index];
        } else if ([vertArg.name isEqualToString:@"lighting"]) {
            [frameInfo->cmdEncode setVertexBuffer:frameInfo->lightBuffer offset:0 atIndex:vertArg.index];
        } else if ([vertArg.name containsString:@"texIndirect"]) {
            WhirlyKitShader::TexIndirect texInd;
            texInd.offset[0] = 0.0;  texInd.offset[1] = 0.0;
            texInd.scale[0] = 1.0; texInd.scale[1] = 1.0;
            [frameInfo->cmdEncode setVertexBytes:&texInd length:sizeof(texInd) atIndex:vertArg.index];
        } else if (vertArg.type == MTLArgumentTypeTexture) {
            // TODO: Implement the attributes users can pass in
        }
    }
    
    // Same for the fragment shader
    NSArray<MTLArgument *> *fragArgs = reflInfo.fragmentArguments;
    texIndex = 0;
    for (MTLArgument *fragArg : fragArgs) {
        if ([fragArg.name isEqualToString:@"uniforms"]) {
            [frameInfo->cmdEncode setFragmentBuffer:frameInfo->uniformTriBuffer offset:0 atIndex:fragArg.index];
        } else {
            if (fragArg.type == MTLArgumentTypeTexture) {
                // Textures we just add as they come along
                SimpleIdentity texID = EmptyIdentity;
                if (texIndex < texInfo.size())
                    texID = texInfo[texIndex].texId;
                
                TextureBaseMTL *tex = NULL;
                if (texID != EmptyIdentity)
                    tex = dynamic_cast<TextureBaseMTL *>(scene->getTexture(texID));
                if (tex)
                    [frameInfo->cmdEncode setFragmentTexture:tex->getMTLID() atIndex:texIndex];
                
                texIndex++;
            }
        }
    }
    
    // This actually draws the triangles (well, in a bit)
    [frameInfo->cmdEncode drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numTris*3 indexType:MTLIndexTypeUInt16 indexBuffer:triBuffer indexBufferOffset:0];
}
    
}
