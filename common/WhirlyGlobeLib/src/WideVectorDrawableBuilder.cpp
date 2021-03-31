/*  WideVectorDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "WideVectorDrawableBuilder.h"
#import "BasicDrawableInstanceBuilder.h"
#import "SceneRenderer.h"
#import "FlatMath.h"

using namespace Eigen;

namespace WhirlyKit
{
    
WideVectorDrawableBuilder::WideVectorDrawableBuilder(std::string name,
                                                     const SceneRenderer *sceneRenderer,
                                                     Scene *scene)
    : name(std::move(name)), renderer(sceneRenderer), scene(scene),
      implType(WideVecImplBasic), basicDrawable(nullptr), instDrawable(nullptr),
      lineWidth(1.0), lineOffset(0.0), globeMode(false),
      texRepeat(1.0), edgeSize(1.0),
      p1_index(-1), n0_index(-1), offset_index(-1), c0_index(-1), tex_index(-1)
{
}
    
WideVectorDrawableBuilder::~WideVectorDrawableBuilder()
{
}
    
void WideVectorDrawableBuilder::Init(unsigned int numVert,
                                     unsigned int numTri,
                                     unsigned int numCenterline,
                                     WideVecImplType implType,
                                     bool inGlobeMode,
                                     const WideVectorInfo *vecInfo)
{
    globeMode = inGlobeMode;
    this->implType = implType;
    
    basicDrawable = renderer->makeBasicDrawableBuilder(name);
    basicDrawable->Init();
    basicDrawable->setupStandardAttributes();
    basicDrawable->setType(Triangles);
    basicDrawable->points.reserve(numVert);
    basicDrawable->tris.reserve(numTri);
    
    if (implType == WideVecImplPerf) {
        instDrawable = renderer->makeBasicDrawableInstanceBuilder(name);
        // TODO: Reserve size for the instances
        basicDrawable->setOnOff(false);
        vecInfo->setupBasicDrawableInstance(instDrawable);
        instDrawable->setMasterID(basicDrawable->basicDraw->getId(), BasicDrawableInstance::LocalStyle);
        inst_index = addAttribute(BDIntType, StringIndexer::getStringID("a_instIndex"),numVert);
    } else {
        vecInfo->setupBasicDrawable(basicDrawable);
    }
          
    lineWidth = 10.0/1024.0;
    lineOffset = 0.0;
    if (implType == WideVecImplBasic) {
        p1_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_p1"),numVert);
        tex_index = addAttribute(BDFloat4Type, StringIndexer::getStringID("a_texinfo"),numVert);
        n0_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_n0"),numVert);
        offset_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_offset"),numVert);
        c0_index = addAttribute(BDFloatType, StringIndexer::getStringID("a_c0"),numVert);
    }
}
    
void WideVectorDrawableBuilder::setLineWidth(float inWidth)
{
    lineWidth = inWidth;
}

void WideVectorDrawableBuilder::setLineOffset(float inOffset)
{
    lineOffset = inOffset;
    lineOffsetSet = true;
}
 
void WideVectorDrawableBuilder::setTexRepeat(float inTexRepeat)
    { texRepeat = inTexRepeat; }

void WideVectorDrawableBuilder::setEdgeSize(float inEdgeSize)
    { edgeSize = inEdgeSize; }

unsigned int WideVectorDrawableBuilder::addPoint(const Point3f &pt)
{
#ifdef WIDEVECDEBUG
    locPts.push_back(pt);
#endif
    basicDrawable->addPoint(pt);
    
    return basicDrawable->getNumPoints()-1;
}

void WideVectorDrawableBuilder::addInstancePoint(const Point3f &pt,int vertIndex,int polyIndex)
{
    basicDrawable->addPoint(pt);
    basicDrawable->addAttributeValue(inst_index, polyIndex | (vertIndex << 16));
}
    
void WideVectorDrawableBuilder::addNormal(const Point3f &norm)
{
    basicDrawable->addNormal(norm);
}

void WideVectorDrawableBuilder::addNormal(const Point3d &norm)
{
    basicDrawable->addNormal(norm);
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

void WideVectorDrawableBuilder::add_offset(const Point3f &offset)
{
    addAttributeValue(offset_index, offset);
}

void WideVectorDrawableBuilder::add_c0(float val)
{
    addAttributeValue(c0_index, val);
#ifdef WIDEVECDEBUG
    c0.push_back(val);
#endif
}

void WideVectorDrawableBuilder::setColorExpression(ColorExpressionInfoRef colorExp)
{
    this->colorExp = std::move(colorExp);
}

void WideVectorDrawableBuilder::setOpacityExpression(FloatExpressionInfoRef opacityExp)
{
    this->opacityExp = std::move(opacityExp);
}

void WideVectorDrawableBuilder::setWidthExpression(FloatExpressionInfoRef inWidthExp)
{
    widthExp = std::move(inWidthExp);
}

void WideVectorDrawableBuilder::setOffsetExpression(FloatExpressionInfoRef inOffsetExp)
{
    offsetExp = std::move(inOffsetExp);
}

void WideVectorDrawableBuilder::setupTweaker(BasicDrawable &theDraw) const
{
    if (auto tweaker = makeTweaker())
    {
        setupTweaker(tweaker);
        theDraw.addTweaker(tweaker);
    }
}

void WideVectorDrawableBuilder::setupTweaker(const DrawableTweakerRef &inTweaker) const
{
    basicDrawable->setupTweaker(inTweaker);
    if (auto tweak = dynamic_cast<WideVectorTweaker*>(inTweaker.get()))
    {
        tweak->edgeSize = edgeSize;
        tweak->lineWidth = lineWidth;
        tweak->widthExp = widthExp;
        tweak->texRepeat = texRepeat;
        tweak->offset = lineOffset;
        tweak->offsetSet = lineOffsetSet;
        tweak->offsetExp = offsetExp;
        tweak->color = color;
    }
}

void WideVectorDrawableBuilder::addCenterLine(const Point3d &centerPt,
                                              const Point3d &up,
                                              double len,
                                              const RGBAColor &inColor,
                                              const std::vector<SimpleIdentity> &maskIDs,
                                              int prev,int next)
{
    CenterPoint pt;
    pt.center = Point3f(centerPt.x(),centerPt.y(),centerPt.z());
    pt.up = Point3f(up.x(),up.y(),up.z());
    pt.len = (float)len;
    pt.color = inColor;
    pt.maskIDs[0] = maskIDs.empty() ? 0 : (int)maskIDs[0];
    pt.maskIDs[1] = maskIDs.size() > 1 ? (int)maskIDs[1] : 0;
    pt.prev = prev;
    pt.next = next;
    centerline.push_back(pt);
}

/// Number of points added so far
unsigned int WideVectorDrawableBuilder::getNumPoints()
{
    return basicDrawable->getNumPoints();
}

/// Numer of triangles added so far
unsigned int WideVectorDrawableBuilder::getNumTris()
{
    return basicDrawable->getNumTris();
}

int WideVectorDrawableBuilder::getCenterLineCount()
{
    return centerline.size();
}

SimpleIdentity WideVectorDrawableBuilder::getBasicDrawableID()
{
    if (!basicDrawable)
        return EmptyIdentity;
    return basicDrawable->getDrawableID();
}

BasicDrawableRef WideVectorDrawableBuilder::getBasicDrawable()
{
    if (!basicDrawable)
        return nullptr;
    return basicDrawable->getDrawable();
}

SimpleIdentity WideVectorDrawableBuilder::getInstanceDrawableID()
{
    if (!instDrawable)
        return EmptyIdentity;
    return instDrawable->getDrawableID();
}

BasicDrawableInstanceRef WideVectorDrawableBuilder::getInstanceDrawable()
{
    if (!instDrawable)
        return nullptr;
    return instDrawable->getDrawable();
}

void WideVectorDrawableBuilder::setFade(TimeInterval inFadeDown,TimeInterval inFadeUp)
{
    if (instDrawable)
        instDrawable->setFade(inFadeDown, inFadeUp);
    else
        basicDrawable->setFade(inFadeDown, inFadeUp);
}

void WideVectorDrawableBuilder::setColor(RGBAColor inColor)
{
    color = inColor;
    if (instDrawable)
        instDrawable->setColor(inColor);
    else
        basicDrawable->setColor(inColor);
}


void WideVectorDrawableBuilder::setLocalMbr(const Mbr &)
{
//    instDrawable->setLocalMbr(mbr);
}

void WideVectorDrawableBuilder::setMatrix(const Eigen::Matrix4d *inMat)
{
    if (instDrawable) {
        // TODO: Add matrix offset to instance
//        instDrawable->setMatrix(inMat);
    } else
        basicDrawable->setMatrix(inMat);
}

void WideVectorDrawableBuilder::addVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    basicDrawable->addVertexAttributes(attrs);
}

/// Add a 2D vector to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector2f &vec)
{
    basicDrawable->addAttributeValue(attrId,vec);
}

/// Add a 3D vector to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector3f &vec)
{
    basicDrawable->addAttributeValue(attrId,vec);
}

/// Add a 4D vector to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector4f &vec)
{
    basicDrawable->addAttributeValue(attrId,vec);
}

/// Add a 4 component char array to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,const RGBAColor &theColor)
{
    basicDrawable->addAttributeValue(attrId,theColor);
}

/// Add a float to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,float theColor)
{
    basicDrawable->addAttributeValue(attrId,theColor);
}

/// Add an integer value to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,int val)
{
    basicDrawable->addAttributeValue(attrId,val);
}

/// Add an identity-type value to the given attribute array
void WideVectorDrawableBuilder::addAttributeValue(int attrId,int64_t val)
{
    basicDrawable->addAttributeValue(attrId,val);
}

/// Add a triangle.  Should point to the vertex IDs.
void WideVectorDrawableBuilder::addTriangle(BasicDrawable::Triangle tri)
{
    basicDrawable->addTriangle(tri);
}

void WideVectorDrawableBuilder::setTexId(unsigned int which,SimpleIdentity inId)
{
    if (instDrawable)
        instDrawable->setTexId(which, inId);
    else
        basicDrawable->setTexId(which, inId);
}

void WideVectorDrawableBuilder::setProgram(SimpleIdentity progId)
{
    if (instDrawable)
        instDrawable->setProgram(progId);
    else
        basicDrawable->setProgram(progId);
}

}

#include <utility>
