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
#import "RawData_NSData.h"

namespace WhirlyKit
{
    
BillboardDrawableBuilderMTL::BillboardDrawableBuilderMTL(const std::string &name,Scene *scene)
    : BasicDrawableBuilderMTL(name,scene)
{
    Init();
}
    
void BillboardDrawableBuilderMTL::Init()
{
    basicDraw = std::make_shared<BasicDrawableMTL>("Billboard");
    BillboardDrawableBuilder::Init();
    
    // Wire up the buffers we use
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[offsetIndex])->slot = WhirlyKitShader::WKSVertexBillboardOffsetAttribute;
}
    
BasicDrawableRef BillboardDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawableRef theDraw = BasicDrawableBuilderMTL::getDrawable();

    WhirlyKitShader::UniformBillboard uniBB;
    bzero(&uniBB,sizeof(uniBB));
    uniBB.groundMode = groundMode;
    BasicDrawable::UniformBlock uniBlock;
    uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&uniBB length:sizeof(uniBB)]));
    uniBlock.bufferID = WhirlyKitShader::WKSUniformBillboardEntry;
    basicDraw->setUniBlock(uniBlock);
    
    return theDraw;
}

    
}
