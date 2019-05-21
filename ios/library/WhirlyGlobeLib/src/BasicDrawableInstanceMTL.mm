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

namespace WhirlyKit
{
    
BasicDrawableInstanceMTL::BasicDrawableInstanceMTL(const std::string &name)
    : BasicDrawableInstance(name)
{
}

void BasicDrawableInstanceMTL::setupForRenderer(const RenderSetupInfo *setupInfo)
{
    // TODO: Implement
}

void BasicDrawableInstanceMTL::teardownForRenderer(const RenderSetupInfo *setupInfo)
{
    // TODO: Implement
}

void BasicDrawableInstanceMTL::draw(RendererFrameInfo *inFrameInfo,Scene *scene)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;
    ProgramMTL *program = (ProgramMTL *)frameInfo->program;
    SceneRendererMTL *sceneRender = (SceneRendererMTL *)frameInfo->sceneRenderer;
    id<MTLDevice> mtlDevice = sceneRender->setupInfo.mtlDevice;
    
    MTLRenderPipelineDescriptor *renderDesc = [[MTLRenderPipelineDescriptor alloc] init];
    renderDesc.vertexFunction = program->vertFunc;
    renderDesc.fragmentFunction = program->fragFunc;

    // TODO: Should be from the target
    renderDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    NSError *err = nil;
    id<MTLRenderPipelineState> renderState = [mtlDevice newRenderPipelineStateWithDescriptor:renderDesc error:&err];
    // Note: Vertex description
    [frameInfo->cmdEncode setRenderPipelineState:renderState];
    
    // TODO: And draw....
}
    
}
