/*
 *  BasicDrawableBuilderMTL.mm
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
#import "BasicDrawableBuilderMTL.h"
#import "DefaultShadersMTL.h"
#import "RawData_NSData.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableBuilderMTL::BasicDrawableBuilderMTL(const std::string &name,Scene *scene)
    : BasicDrawableBuilder(name,scene), drawableGotten(false)
{
    basicDraw = std::make_shared<BasicDrawableMTL>(name);
    BasicDrawableBuilder::Init();
    setupStandardAttributes();
}
    
void BasicDrawableBuilderMTL::setupStandardAttributes(int numReserve)
{
    basicDraw->colorEntry = addAttribute(BDChar4Type,a_colorNameID);
    VertexAttributeMTL *colorAttr = (VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->colorEntry];
    colorAttr->slot = WhirlyKitShader::WKSVertexColorAttribute;
    colorAttr->setDefaultColor(RGBAColor(255,255,255,255));
    colorAttr->reserve(numReserve);
    
    basicDraw->normalEntry = addAttribute(BDFloat3Type,a_normalNameID);
    VertexAttributeMTL *normalAttr = (VertexAttributeMTL *)basicDraw->vertexAttributes[basicDraw->normalEntry];
    normalAttr->slot = WhirlyKitShader::WKSVertexNormalAttribute;
    normalAttr->setDefaultVector3f(Vector3f(1.0,1.0,1.0));
    normalAttr->reserve(numReserve);
}
    
BasicDrawableBuilderMTL::~BasicDrawableBuilderMTL()
{
    if (!drawableGotten && basicDraw)
        basicDraw = NULL;
}
    
int BasicDrawableBuilderMTL::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    VertexAttribute *attr = new VertexAttributeMTL(dataType,nameID);
    attr->slot = slot;
    if (numThings > 0)
        attr->reserve(numThings);
    basicDraw->vertexAttributes.push_back(attr);
    
    return (unsigned int)(basicDraw->vertexAttributes.size()-1);
}
    
void BasicDrawableBuilderMTL::setupTexCoordEntry(int which,int numReserve)
{
    if (which < basicDraw->texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)basicDraw->texInfo.size();ii<=which;ii++)
    {
        BasicDrawable::TexInfo newInfo;
        char attributeName[40];
        sprintf(attributeName,"a_texCoord%d",ii);
        newInfo.texCoordEntry = addAttribute(BDFloat2Type,StringIndexer::getStringID(attributeName));
        VertexAttributeMTL *vertAttrMTL = (VertexAttributeMTL *)basicDraw->vertexAttributes[newInfo.texCoordEntry];
        vertAttrMTL->setDefaultVector2f(Vector2f(0.0,0.0));
        vertAttrMTL->reserve(numReserve);
        vertAttrMTL->slot = WhirlyKitShader::WKSVertexTextureBaseAttribute+ii;
        basicDraw->texInfo.push_back(newInfo);
    }
}

BasicDrawableRef BasicDrawableBuilderMTL::getDrawable()
{
    if (!basicDraw)
        return NULL;
    
    BasicDrawableMTLRef draw = std::dynamic_pointer_cast<BasicDrawableMTL>(basicDraw);
    
    if (!drawableGotten) {
        int ptsIndex = addAttribute(BDFloat3Type, a_PositionNameID);
        VertexAttributeMTL *ptsAttr = (VertexAttributeMTL *)basicDraw->vertexAttributes[ptsIndex];
        ptsAttr->slot = WhirlyKitShader::WKSVertexPositionAttribute;
        ptsAttr->reserve(points.size());
        for (auto pt : points)
            ptsAttr->addVector3f(pt);
        draw->tris = tris;
        
        // Expression uniforms, if we have those
        if (colorExp || opacityExp || includeExp) {
            WhirlyKitShader::UniformDrawStateExp vecExp;
            memset(&vecExp, 0, sizeof(vecExp));
            if (colorExp)
                ColorExpressionToMtl(colorExp, vecExp.colorExp);
            if (opacityExp)
                FloatExpressionToMtl(opacityExp, vecExp.opacityExp);
            
            BasicDrawable::UniformBlock uniBlock;
            uniBlock.blockData = RawDataRef(new RawNSDataReader([[NSData alloc] initWithBytes:&vecExp length:sizeof(vecExp)]));
            uniBlock.bufferID = WhirlyKitShader::WKSUniformVecEntryExp;
            basicDraw->setUniBlock(uniBlock);
        }
        
        drawableGotten = true;
    }
    
    return draw;
}
    
}
