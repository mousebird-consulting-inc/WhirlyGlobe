/*
 *  WideVectorDrawableBuilderMTL.mm
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

#import "WideVectorDrawableBuilderMTL.h"
#import "DefaultShadersMTL.h"
#import "ProgramMTL.h"
#import "RawData_NSData.h"

namespace WhirlyKit
{

WideVectorDrawableBuilderMTL::WideVectorDrawableBuilderMTL(const std::string &name,Scene *scene)
: BasicDrawableBuilderMTL(name,scene)
{
}
    
void WideVectorDrawableBuilderMTL::Init(unsigned int numVert, unsigned int numTri, bool globeMode)
{
    basicDraw = std::make_shared<BasicDrawableMTL>("Wide Vector");
    WideVectorDrawableBuilder::Init(numVert,numTri,globeMode);
    
    // Wire up the buffers
    // TODO: Merge these into a single data structure
    if (globeMode)
        ((VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->normalEntry])->slot = WhirlyKitShader::WKSVertexNormalAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->colorEntry])->slot = WhirlyKitShader::WKSVertexColorAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[p1_index])->slot = WhirlyKitShader::WKSVertexWideVecP1Attribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[tex_index])->slot = WhirlyKitShader::WKSVertexWideVecTexInfoAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[n0_index])->slot = WhirlyKitShader::WKSVertexWideVecN0Attribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[c0_index])->slot = WhirlyKitShader::WKSVertexWideVecC0Attribute;
}

WideVectorTweaker *WideVectorDrawableBuilderMTL::makeTweaker()
{
    return NULL;
}

BasicDrawableRef WideVectorDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawableRef theDraw = BasicDrawableBuilderMTL::getDrawable();

    // Uniforms for regular wide vectors
    WhirlyKitShader::UniformWideVec uniWV;
    memset(&uniWV,0,sizeof(uniWV));
    uniWV.w2 = lineWidth/2.0;
    uniWV.edge = edgeSize;
    uniWV.texRepeat = texRepeat;
    uniWV.color[0] = color.r/255.0;
    uniWV.color[1] = color.g/255.0;
    uniWV.color[2] = color.b/255.0;
    uniWV.color[3] = color.a/255.0;
    uniWV.hasExp = widthExp || colorExp || opacityExp || includeExp;

    BasicDrawable::UniformBlock uniBlock;
    uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&uniWV length:sizeof(uniWV)]));
    uniBlock.bufferID = WhirlyKitShader::WKSUniformWideVecEntry;
    basicDraw->setUniBlock(uniBlock);
    
    // Expression uniforms, if we're using those
    if (uniWV.hasExp) {
        WhirlyKitShader::UniformWideVecExp wideVecExp;
        memset(&wideVecExp, 0, sizeof(wideVecExp));
        if (widthExp)
            FloatExpressionToMtl(widthExp,wideVecExp.widthExp);
        if (opacityExp)
            FloatExpressionToMtl(opacityExp,wideVecExp.opacityExp);
        if (colorExp)
            ColorExpressionToMtl(colorExp,wideVecExp.colorExp);
        
        BasicDrawable::UniformBlock uniBlock;
        uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&wideVecExp length:sizeof(wideVecExp)]));
        uniBlock.bufferID = WhirlyKitShader::WKSUniformWideVecEntryExp;
        basicDraw->setUniBlock(uniBlock);
    }

    return theDraw;
}

}
