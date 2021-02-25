/*
 *  ScreenSpaceDrawableBuilderMTL.mm
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

#import "ScreenSpaceDrawableBuilderMTL.h"
#import "DefaultShadersMTL.h"
#import "RawData_NSData.h"

namespace WhirlyKit
{
    
ScreenSpaceDrawableBuilderMTL::ScreenSpaceDrawableBuilderMTL(const std::string &name,Scene *scene)
    : BasicDrawableBuilderMTL(name,scene)
{
    this->scene = scene;
}

void ScreenSpaceDrawableBuilderMTL::Init(bool hasMotion,bool hasRotation,bool buildAnyway)
{
    basicDraw = std::make_shared<BasicDrawableMTL>("Screen Space");
    // Need the entries even if we don't bother to fill them in
    ScreenSpaceDrawableBuilder::Init(hasMotion,hasRotation,true);
    
    // Wire up the buffers
    // TODO: Merge these into a single data structure
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[offsetIndex])->slot = WhirlyKitShader::WKSVertexScreenSpaceOffsetAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[rotIndex])->slot = WhirlyKitShader::WKSVertexScreenSpaceRotAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[dirIndex])->slot = WhirlyKitShader::WKSVertexScreenSpaceDirAttribute;
}

ScreenSpaceTweaker *ScreenSpaceDrawableBuilderMTL::makeTweaker()
{
    return NULL;
}

BasicDrawableRef ScreenSpaceDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawableRef theDraw = BasicDrawableBuilderMTL::getDrawable();
    
    WhirlyKitShader::UniformScreenSpace uniSS;
    bzero(&uniSS,sizeof(uniSS));
    uniSS.keepUpright = keepUpright;
    if (motion) {
        theDraw->motion = true;
        uniSS.startTime = startTime - scene->getBaseTime();
    } else
        uniSS.startTime = 0.0;
    uniSS.activeRot = rotation;
    uniSS.hasMotion = motion;
    uniSS.hasExp = opacityExp || colorExp || scaleExp || includeExp;
    
    BasicDrawable::UniformBlock uniBlock;
    uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&uniSS length:sizeof(uniSS)]));
    uniBlock.bufferID = WhirlyKitShader::WKSUniformScreenSpaceEntry;
    theDraw->setUniBlock(uniBlock);

    // Expression uniforms, if present
    if (uniSS.hasExp) {
        WhirlyKitShader::UniformScreenSpaceExp ssExp;
        memset(&ssExp, 0, sizeof(ssExp));
        if (scaleExp)
            FloatExpressionToMtl(scaleExp,ssExp.scaleExp);
        if (opacityExp)
            FloatExpressionToMtl(opacityExp,ssExp.opacityExp);
        if (colorExp)
            ColorExpressionToMtl(colorExp,ssExp.colorExp);

        BasicDrawable::UniformBlock uniBlock;
        uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&ssExp length:sizeof(ssExp)]));
        uniBlock.bufferID = WhirlyKitShader::WKSUniformScreenSpaceEntryExp;
        basicDraw->setUniBlock(uniBlock);
    }
    
    return theDraw;
}
    
}
