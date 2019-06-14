/*
 *  BillboardDrawableBuilderMTL.mm
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

#import "BillboardDrawableBuilderMTL.h"
#import "DefaultShadersMTL.h"

namespace WhirlyKit
{
    
BillboardTweakerMTL::BillboardTweakerMTL()
: groundMode(false)
{
}
    
void BillboardTweakerMTL::tweakForFrame(Drawable *inDraw,RendererFrameInfo *inFrameInfo)
{
    RendererFrameInfoMTL *frameInfo = (RendererFrameInfoMTL *)inFrameInfo;

    WhirlyKitShader::UniformBillboard uniBB;
    bzero(&uniBB,sizeof(uniBB));
    uniBB.groundMode = groundMode;
    CopyIntoMtlFloat3(uniBB.eyeVec, frameInfo->eyeVec);

    // Note: Can we not do this?
    [frameInfo->cmdEncode setCullMode:MTLCullModeNone];
    [frameInfo->cmdEncode setVertexBytes:&uniBB length:sizeof(uniBB) atIndex:WKSUniformDrawStateBillboardBuffer];
}

BillboardDrawableBuilderMTL::BillboardDrawableBuilderMTL(const std::string &name)
    : BasicDrawableBuilderMTL(name)
{
    Init();
}
    
void BillboardDrawableBuilderMTL::Init()
{
    BillboardDrawableBuilder::Init();
    
    // Wire up the buffers we use
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[offsetIndex])->bufferIndex = WKSVertexBillboardOffsetAttribute;
}
    
BasicDrawable *BillboardDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawable *theDraw = BasicDrawableBuilderMTL::getDrawable();
    BillboardTweakerMTL *tweak = new BillboardTweakerMTL();
    tweak->groundMode = groundMode;
    theDraw->addTweaker(DrawableTweakerRef(tweak));
    
    return theDraw;
}

    
}
