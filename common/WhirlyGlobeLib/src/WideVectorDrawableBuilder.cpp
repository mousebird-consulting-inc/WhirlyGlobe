/*
 *  WideVectorDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
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

#import "WideVectorDrawableBuilder.h"
#import "SceneRenderer.h"
#import "FlatMath.h"

using namespace Eigen;

namespace WhirlyKit
{
    
WideVectorDrawableBuilder::WideVectorDrawableBuilder()
    : texRepeat(1.0), edgeSize(1.0), realWidthSet(false), globeMode(true), color(255,255,255,255)
{
}
    
WideVectorDrawableBuilder::~WideVectorDrawableBuilder()
{
}
    
void WideVectorDrawableBuilder::Init(unsigned int numVert,unsigned int numTri,bool inGlobeMode)
{
    globeMode = inGlobeMode;
    
    BasicDrawableBuilder::Init();
    // Don't want standard attributes
    
    points.reserve(numVert);
    tris.reserve(numTri);
    
    lineWidth = 10.0/1024.0;
    if (globeMode)
        basicDraw->normalEntry = addAttribute(BDFloat3Type, a_normalNameID,numVert);
    basicDraw->colorEntry = addAttribute(BDChar4Type, a_colorNameID);
    p1_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_p1"),numVert);
    tex_index = addAttribute(BDFloat4Type, StringIndexer::getStringID("a_texinfo"),numVert);
    n0_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_n0"),numVert);
    c0_index = addAttribute(BDFloatType, StringIndexer::getStringID("a_c0"),numVert);
}
    
void WideVectorDrawableBuilder::setColor(RGBAColor inColor)
{
    color = inColor;
}
    
void WideVectorDrawableBuilder::setLineWidth(float inWidth)
{
    lineWidth = inWidth;
}
 
void WideVectorDrawableBuilder::setTexRepeat(float inTexRepeat)
    { texRepeat = inTexRepeat; }

void WideVectorDrawableBuilder::setEdgeSize(float inEdgeSize)
    { edgeSize = inEdgeSize; }

void WideVectorDrawableBuilder::setRealWorldWidth(double width)
    { realWidthSet = true;  realWidth = width; }

    
unsigned int WideVectorDrawableBuilder::addPoint(const Point3f &pt)
{
#ifdef WIDEVECDEBUG
    locPts.push_back(pt);
#endif
    return BasicDrawableBuilder::addPoint(pt);
}
    
void WideVectorDrawableBuilder::addNormal(const Point3f &norm)
{
    if (globeMode)
    {
        BasicDrawableBuilder::addNormal(norm);
    }
}

void WideVectorDrawableBuilder::addNormal(const Point3d &norm)
{
    if (globeMode)
    {
        BasicDrawableBuilder::addNormal(norm);
    }
}

void WideVectorDrawableBuilder::add_p1(const Point3f &pt)
{
    addAttributeValue(p1_index, pt);
#ifdef WIDEVECDEBUG
    p1.push_back(pt);
#endif
}

void WideVectorDrawableBuilder::add_texInfo(float texX,float texYmin,float texYmax,float texOffset)
{
    addAttributeValue(tex_index, Vector4f(texX,texYmin,texYmax,texOffset));
#ifdef WIDEVECDEBUG
#endif
}

void WideVectorDrawableBuilder::add_n0(const Point3f &dir)
{
    addAttributeValue(n0_index, dir);
#ifdef WIDEVECDEBUG
    n0.push_back(dir);
#endif
}

void WideVectorDrawableBuilder::add_c0(float val)
{
    addAttributeValue(c0_index, val);
#ifdef WIDEVECDEBUG
    c0.push_back(val);
#endif
}

void WideVectorDrawableBuilder::setWidthExpression(FloatExpressionInfoRef inWidthExp)
{
    widthExp = inWidthExp;
}

void WideVectorDrawableBuilder::setupTweaker(BasicDrawable *theDraw)
{
    WideVectorTweaker *tweak = makeTweaker();
    tweak->realWidthSet = false;
    tweak->realWidth = realWidth;
    tweak->edgeSize = edgeSize;
    tweak->lineWidth = lineWidth;
    tweak->texRepeat = texRepeat;
    tweak->color = color;
    theDraw->addTweaker(DrawableTweakerRef(tweak));
}    
    
}
