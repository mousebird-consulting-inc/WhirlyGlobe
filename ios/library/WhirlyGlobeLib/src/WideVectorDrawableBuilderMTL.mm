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

#import <MetalKit/MetalKit.h>
#import "WideVectorDrawableBuilderMTL.h"
#import "BasicDrawableMTL.h"
#import "BasicDrawableInstanceBuilderMTL.h"
#import "DefaultShadersMTL.h"
#import "ProgramMTL.h"
#import "RawData_NSData.h"

namespace WhirlyKit
{

WideVectorDrawableBuilderMTL::WideVectorDrawableBuilderMTL(const std::string &name,const SceneRenderer *sceneRenderer,Scene *scene)
: WideVectorDrawableBuilder(name, sceneRenderer, scene),
    drawableGotten(false), instanceGotten(false)
{
}
    
void WideVectorDrawableBuilderMTL::Init(unsigned int numVert, unsigned int numTri, unsigned int numCenterLine,
                                        WideVecImplType implType, bool globeMode)
{
    
    WideVectorDrawableBuilder::Init(numVert,numTri,numCenterLine,implType,globeMode);
    
    if (implType == WideVecImplBasic) {
        // Wire up the buffers
        // TODO: Merge these into a single data structure
        if (globeMode)
            ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[basicDrawable->basicDraw->normalEntry])->slot = WhirlyKitShader::WKSVertexNormalAttribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[basicDrawable->basicDraw->colorEntry])->slot = WhirlyKitShader::WKSVertexColorAttribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[p1_index])->slot = WhirlyKitShader::WKSVertexWideVecP1Attribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[tex_index])->slot = WhirlyKitShader::WKSVertexWideVecTexInfoAttribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[n0_index])->slot = WhirlyKitShader::WKSVertexWideVecN0Attribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[offset_index])->slot = WhirlyKitShader::WKSVertexWideVecOffsetAttribute;
        ((VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[c0_index])->slot = WhirlyKitShader::WKSVertexWideVecC0Attribute;
    }
}

int WideVectorDrawableBuilderMTL::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    return basicDrawable->addAttribute(dataType, nameID, slot, numThings);
}


WideVectorTweaker *WideVectorDrawableBuilderMTL::makeTweaker()
{
    return nullptr;
}

BasicDrawableRef WideVectorDrawableBuilderMTL::getBasicDrawable()
{
    if (drawableGotten)
        return basicDrawable->basicDraw;
    
    drawableGotten = true;
    VertexAttributeMTL *colorAttr = (VertexAttributeMTL *)basicDrawable->basicDraw->vertexAttributes[basicDrawable->basicDraw->colorEntry];
    colorAttr->setDefaultColor(color);

    return basicDrawable->basicDraw;
}

BasicDrawableInstanceRef WideVectorDrawableBuilderMTL::getInstanceDrawable()
{
    if (instanceGotten)
        return instDrawable->drawInst;

    instanceGotten = true;
    
    // Uniforms for regular wide vectors
    WhirlyKitShader::UniformWideVec uniWV;
    memset(&uniWV,0,sizeof(uniWV));
    uniWV.w2 = lineWidth/2.0;
    uniWV.offset = lineOffset;
    uniWV.edge = edgeSize;
    uniWV.texRepeat = texRepeat;
    uniWV.hasExp = widthExp || offsetExp || colorExp || opacityExp;

    BasicDrawable::UniformBlock uniBlock;
    uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&uniWV length:sizeof(uniWV)]));
    uniBlock.bufferID = WhirlyKitShader::WKSUniformWideVecEntry;
    instDrawable->drawInst->setUniBlock(uniBlock);
    
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
        instDrawable->drawInst->setUniBlock(uniBlock);
    }
    
    // Instances also go into their own buffer
    std::vector<WhirlyKitShader::VertexTriWideVecInstance> vecInsts(centerline.size());
    for (unsigned int ii=0;ii<centerline.size();ii++) {
        auto *outPtr = &vecInsts[ii];
        auto *inPtr = &centerline[ii];
        CopyIntoMtlFloat3(outPtr->center,inPtr->center);
        CopyIntoMtlFloat3(outPtr->up, inPtr->up);
        outPtr->len = inPtr->len;
        float color[4];
        inPtr->color.asUnitFloats(color);
        CopyIntoMtlFloat4(outPtr->color,color);
        outPtr->prev = inPtr->prev;
        outPtr->next = inPtr->next;
        outPtr->mask0 = inPtr->maskIDs[0];
        outPtr->mask1 = inPtr->maskIDs[1];
    }

    return instDrawable->drawInst;
}


}
