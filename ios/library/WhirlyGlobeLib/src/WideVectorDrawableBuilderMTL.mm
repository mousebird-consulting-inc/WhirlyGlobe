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
: name(name), scene(scene)
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
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[offset_index])->slot = WhirlyKitShader::WKSVertexWideVecOffsetAttribute;
    ((VertexAttributeMTL *)basicDraw->vertexAttributes[c0_index])->slot = WhirlyKitShader::WKSVertexWideVecC0Attribute;
}

WideVectorTweaker *WideVectorDrawableBuilderMTL::makeTweaker()
{
    return NULL;
}

DrawableRef WideVectorDrawableBuilderMTL::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderMTL::getDrawable();
    
    BasicDrawableRef theDraw = BasicDrawableBuilderMTL::getDrawable();

    VertexAttributeMTL *colorAttr = (VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->colorEntry];
    colorAttr->setDefaultColor(color);

    // Uniforms for regular wide vectors
    WhirlyKitShader::UniformWideVec uniWV;
    memset(&uniWV,0,sizeof(uniWV));
    uniWV.w2 = lineWidth/2.0;
    uniWV.offset = lineOffset;
    uniWV.edge = edgeSize;
    uniWV.texRepeat = texRepeat;
    uniWV.hasExp = widthExp || offsetExp || colorExp || opacityExp || includeExp;

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
        if (offsetExp)
            FloatExpressionToMtl(offsetExp,wideVecExp.offsetExp);
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
