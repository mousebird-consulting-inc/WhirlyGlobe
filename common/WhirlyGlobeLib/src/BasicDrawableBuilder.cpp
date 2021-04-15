/*  BasicDrawableBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "BasicDrawableBuilder.h"
#import "SceneRenderer.h"

using namespace Eigen;

namespace WhirlyKit
{

float BasicDrawableTweaker::getZoom(const Drawable &inDraw,const Scene &scene,float def) const
{
    const auto bd = dynamic_cast<const BasicDrawable*>(&inDraw);
    return (bd && bd->zoomSlot >= 0) ? scene.getZoomSlotValue(bd->zoomSlot) : def;
}

BasicDrawableBuilder::BasicDrawableBuilder() :
    scene(nullptr)
{
}
    
BasicDrawableBuilder::BasicDrawableBuilder(std::string name,Scene *scene) :
    name(std::move(name)),
    scene(scene)
{
}
    
void BasicDrawableBuilder::setName(std::string name)
{
    basicDraw->name = std::move(name);
}
    
void BasicDrawableBuilder::reserve(int numVert,int numTri)
{
    points.reserve(numVert);
    tris.reserve(numTri);
}
    
void BasicDrawableBuilder::Init()
{
    basicDraw->name = name;
    basicDraw->colorEntry = -1;
    basicDraw->normalEntry = -1;
    
    basicDraw->on = true;
    basicDraw->startEnable = 0.0;
    basicDraw->endEnable = 0.0;
    basicDraw->programId = EmptyIdentity;
    basicDraw->isAlpha = false;
    basicDraw->drawOrder = BaseInfo::DrawOrderTiles;
    basicDraw->drawPriority = 0;
    basicDraw->drawOffset = 0;
    basicDraw->type = Points;
    basicDraw->minVisible = basicDraw->maxVisible = DrawVisibleInvalid;
    basicDraw->minVisibleFadeBand = basicDraw->maxVisibleFadeBand = 0.0;
    basicDraw->minViewerDist = basicDraw->maxViewerDist = DrawVisibleInvalid;
    basicDraw->viewerCenter = Point3d(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid);
    basicDraw->zoomSlot = -1;
    basicDraw->minZoomVis = basicDraw->maxZoomVis = DrawVisibleInvalid;
    
    basicDraw->fadeDown = basicDraw->fadeUp = 0.0;
    basicDraw->color = RGBAColor(255,255,255,255);
    basicDraw->hasOverrideColor = false;
    basicDraw->lineWidth = 1.0;
    
    basicDraw->numTris = 0;
    basicDraw->numPoints = 0;
    
    basicDraw->requestZBuffer = false;
    basicDraw->writeZBuffer = true;
    basicDraw->renderTargetID = EmptyIdentity;
    
    basicDraw->clipCoords = false;
    
    basicDraw->hasMatrix = false;
    basicDraw->motion = false;
    basicDraw->extraFrames = 0;
    
    basicDraw->valuesChanged = true;
    basicDraw->texturesChanged = true;
    includeExp = false;
}
    
void BasicDrawableBuilder::setupStandardAttributes(int numReserve)
{
    //    setupTexCoordEntry(0,numReserve);
    
    basicDraw->colorEntry = findAttribute(a_colorNameID);
    if (basicDraw->colorEntry < 0)
        basicDraw->colorEntry = addAttribute(BDChar4Type,a_colorNameID);
    basicDraw->vertexAttributes[basicDraw->colorEntry]->setDefaultColor(RGBAColor(255,255,255,255));
    basicDraw->vertexAttributes[basicDraw->colorEntry]->reserve(numReserve);
    
    basicDraw->normalEntry = findAttribute(a_normalNameID);
    if (basicDraw->normalEntry < 0)
        basicDraw->normalEntry = addAttribute(BDFloat3Type,a_normalNameID);
    basicDraw->vertexAttributes[basicDraw->normalEntry]->setDefaultVector3f(Vector3f(1.0,1.0,1.0));
    basicDraw->vertexAttributes[basicDraw->normalEntry]->reserve(numReserve);
}
    
SimpleIdentity BasicDrawableBuilder::getDrawableID() const
{
    return basicDraw ? basicDraw->getId() : EmptyIdentity;
}
    
int BasicDrawableBuilder::getDrawablePriority() const
{
   return basicDraw ? basicDraw->getDrawPriority() : 0;
}
    
void BasicDrawableBuilder::setupTexCoordEntry(int which,int numReserve)
{
    if (which < basicDraw->texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)basicDraw->texInfo.size();ii<=which;ii++)
    {
        BasicDrawable::TexInfo newInfo;
        char attributeName[40];
        sprintf(attributeName,"a_texCoord%d",ii);
        newInfo.texCoordEntry = addAttribute(BDFloat2Type,StringIndexer::getStringID(attributeName));
        basicDraw->vertexAttributes[newInfo.texCoordEntry]->setDefaultVector2f(Vector2f(0.0,0.0));
        basicDraw->vertexAttributes[newInfo.texCoordEntry]->reserve(numReserve);
        basicDraw->texInfo.push_back(newInfo);
    }
}
    
void BasicDrawableBuilder::setOnOff(bool onOff)
{
    basicDraw->on = onOff;
}

void BasicDrawableBuilder::setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable)
{
    basicDraw->startEnable = inStartEnable;
    basicDraw->endEnable = inEndEnable;
}

void BasicDrawableBuilder::setFade(TimeInterval inFadeDown,TimeInterval inFadeUp)
{
    basicDraw->fadeUp = inFadeUp;  basicDraw->fadeDown = inFadeDown;
}

void BasicDrawableBuilder::setLocalMbr(Mbr mbr)
{
    basicDraw->localMbr = mbr;
}
    
const Mbr &BasicDrawableBuilder::getLocalMbr()
{
    return basicDraw->localMbr;
}

void BasicDrawableBuilder::setViewerVisibility(double inMinViewerDist,double inMaxViewerDist,const Point3d &inViewerCenter)
{
    basicDraw->minViewerDist = inMinViewerDist;
    basicDraw->maxViewerDist = inMaxViewerDist;
    basicDraw->viewerCenter = inViewerCenter;
}

void BasicDrawableBuilder::setVisibleRange(float minVis,float maxVis,float minVisBand,float maxVisBand)
{
    basicDraw->minVisible = minVis;  basicDraw->maxVisible = maxVis;  basicDraw->minVisibleFadeBand = minVisBand; basicDraw->maxVisibleFadeBand = maxVisBand;
}

void BasicDrawableBuilder::setZoomInfo(int zoomSlot,double minZoomVis,double maxZoomVis)
{
    basicDraw->zoomSlot = zoomSlot;
    basicDraw->minZoomVis = minZoomVis;
    basicDraw->maxZoomVis = maxZoomVis;
}

void BasicDrawableBuilder::setAlpha(bool onOff)
{
    basicDraw->isAlpha = onOff;
}

void BasicDrawableBuilder::setDrawOffset(float newOffset)
{
    basicDraw->drawOffset = newOffset;
}

int64_t BasicDrawableBuilder::getDrawOrder() const
{
    return basicDraw->drawOrder;
}

void BasicDrawableBuilder::setDrawOrder(int64_t newOrder)
{
    basicDraw->drawOrder = newOrder;
}

void BasicDrawableBuilder::setDrawPriority(unsigned int newPriority)
{
    basicDraw->drawPriority = newPriority;
}

void BasicDrawableBuilder::setMatrix(const Eigen::Matrix4d *inMat)
{
    basicDraw->mat = *inMat; basicDraw->hasMatrix = true;
}

void BasicDrawableBuilder::setRequestZBuffer(bool val)
{
    basicDraw->requestZBuffer = val;
}

void BasicDrawableBuilder::setWriteZBuffer(bool val)
{
    basicDraw->writeZBuffer = val;
}

void BasicDrawableBuilder::setRenderTarget(SimpleIdentity newRenderTarget)
{
    basicDraw->renderTargetID = newRenderTarget;
}

void BasicDrawableBuilder::setLineWidth(float inWidth)
{
    basicDraw->lineWidth = inWidth;
}
    
float BasicDrawableBuilder::getLineWidth() const
{
    return basicDraw->lineWidth;
}

void BasicDrawableBuilder::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    basicDraw->texInfo[which].texId = inId;
}

SimpleIdentity BasicDrawableBuilder::getTexId(unsigned int which) const
{
    if (!basicDraw)
        return EmptyIdentity;
    if (which >= basicDraw->texInfo.size())
        return EmptyIdentity;
    
    return basicDraw->texInfo[which].texId;
}

void BasicDrawableBuilder::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<texIDs.size();ii++)
    {
        setupTexCoordEntry(ii, 0);
        basicDraw->texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawableBuilder::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    basicDraw->setTexRelative(which, size, borderTexel, relLevel, relX, relY);
}

void BasicDrawableBuilder::setProgram(SimpleIdentity progId)
{
    basicDraw->setProgram(progId);
}

void BasicDrawableBuilder::setupTweaker(BasicDrawable &theDraw) const
{
    if (auto tweaker = makeTweaker())
    {
        setupTweaker(tweaker);
        theDraw.addTweaker(tweaker);
    }
}

void BasicDrawableBuilder::setupTweaker(const DrawableTweakerRef &inTweaker) const
{
    if (auto tweak = std::dynamic_pointer_cast<BasicDrawableTweaker>(inTweaker))
    {
        tweak->color = color;
        tweak->colorExp = colorExp;
        tweak->opacityExp = opacityExp;
    }
}

void BasicDrawableBuilder::addTweaker(const DrawableTweakerRef &tweakRef)
{
    basicDraw->tweakers.insert(tweakRef);
}

void BasicDrawableBuilder::setType(GeometryType inType)
{
    basicDraw->type = inType;
}

void BasicDrawableBuilder::setIncludeExp(bool newVal)
{
    includeExp = newVal;
}

void BasicDrawableBuilder::setColor(RGBAColor inColor)
{
    color = inColor;
    if (basicDraw->colorEntry >= 0)
    {
        basicDraw->vertexAttributes[basicDraw->colorEntry]->setDefaultColor(color);
    }
    basicDraw->color = color;
}

void BasicDrawableBuilder::setColor(const unsigned char color[])
{
    setColor(RGBAColor(color[0],color[1],color[2],color[3]));
}

void BasicDrawableBuilder::setColorExpression(const ColorExpressionInfoRef &inColorExp)
{
    colorExp = inColorExp;
}

void BasicDrawableBuilder::setOpacityExpression(const FloatExpressionInfoRef &inOpacityExp)
{
    opacityExp = inOpacityExp;
}

void BasicDrawableBuilder::setExtraFrames(int numFrames)
{
    basicDraw->extraFrames = numFrames;
}
    
void BasicDrawableBuilder::reserveNumPoints(int numPoints)
{
    points.reserve(points.size()+numPoints);
}

void BasicDrawableBuilder::reserveNumTris(int numTris)
{
    tris.reserve(tris.size()+numTris);
}

void BasicDrawableBuilder::reserveNumTexCoords(unsigned int which,int numCoords)
{
    setupTexCoordEntry(which, numCoords);
    basicDraw->vertexAttributes[basicDraw->texInfo[which].texCoordEntry]->reserve(numCoords);
}

void BasicDrawableBuilder::reserveNumNorms(int numNorms)
{
    basicDraw->vertexAttributes[basicDraw->normalEntry]->reserve(numNorms);
}

void BasicDrawableBuilder::reserveNumColors(int numColors)
{
    basicDraw->vertexAttributes[basicDraw->colorEntry]->reserve(numColors);
}

void BasicDrawableBuilder::setClipCoords(bool clipCoords)
{
    basicDraw->clipCoords = clipCoords;
}

unsigned int BasicDrawableBuilder::addPoint(const Point3f &pt)
{
    points.push_back(pt);
    return (unsigned int)(points.size()-1);
}

unsigned int BasicDrawableBuilder::addPoint(const Point3d &pt)
{
    points.push_back(Point3f(pt.x(),pt.y(),pt.z()));
    return (unsigned int)(points.size()-1);
}
    
unsigned int BasicDrawableBuilder::getNumPoints() const
{
    return points.size();
}

unsigned int BasicDrawableBuilder::getNumTris() const
{
    return tris.size();
}

Point3d BasicDrawableBuilder::getPoint(int which) const
{
    if (which >= points.size())
        return Point3d(0,0,0);
    const Point3f &pt = points[which];
    
    return Point3d(pt.x(),pt.y(),pt.z());
}

void BasicDrawableBuilder::addTexCoord(int which,TexCoord coord)
{
    if (which == -1)
    {
        // In this mode, add duplicate texture coords in each of the vertex attrs
        // Note: This could be optimized to a single set of vertex attrs for all the texture coords
        for (unsigned int ii=0;ii<basicDraw->texInfo.size();ii++)
            basicDraw->vertexAttributes[basicDraw->texInfo[ii].texCoordEntry]->addVector2f(coord);
    } else {
        setupTexCoordEntry(which, 0);
        basicDraw->vertexAttributes[basicDraw->texInfo[which].texCoordEntry]->addVector2f(coord);
    }
}

void BasicDrawableBuilder::addColor(RGBAColor color)
{
    if (basicDraw->colorEntry < 0)
        return;
    
    basicDraw->vertexAttributes[basicDraw->colorEntry]->addColor(color);
}

void BasicDrawableBuilder::addNormal(const Point3f &norm)
{
    if (basicDraw->normalEntry < 0)
        return;
    
    basicDraw->vertexAttributes[basicDraw->normalEntry]->addVector3f(norm);
}

void BasicDrawableBuilder::addNormal(const Point3d &norm)
{
    if (basicDraw->normalEntry < 0)
        return;
    
    basicDraw->vertexAttributes[basicDraw->normalEntry]->addVector3f(Point3f(norm.x(),norm.y(),norm.z()));
}

bool BasicDrawableBuilder::compareVertexAttributes(const SingleVertexAttributeSet &attrs) const
{
    for (const auto &attr : attrs)
    {
        unsigned attrId = -1;
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
        {
            if (basicDraw->vertexAttributes[ii]->nameID == attr.nameID)
            {
                attrId = ii;
                break;
            }
        }
        if (attrId == -1 || basicDraw->vertexAttributes[attrId]->getDataType() != attr.type)
        {
            return false;
        }
    }
    
    return true;
}

void BasicDrawableBuilder::setVertexAttribute(const SingleVertexAttributeInfo &attr)
{
    addAttribute(attr.type,attr.nameID,attr.slot);
}

void BasicDrawableBuilder::setVertexAttributes(const SingleVertexAttributeInfoSet &attrs)
{
    for (auto attr : attrs)
    {
        addAttribute(attr.type,attr.nameID,attr.slot);
    }
}

void BasicDrawableBuilder::addVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    for (const auto &attr : attrs)
    {
        int attrId = -1;
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
            if (basicDraw->vertexAttributes[ii]->nameID == attr.nameID)
            {
                attrId = ii;
                break;
            }
        
        if (attrId == -1)
            continue;
        
        switch (attr.type)
        {
            case BDFloatType:
                addAttributeValue(attrId, attr.data.floatVal);
                break;
            case BDFloat2Type:
            {
                addAttributeValue(attrId, Vector2f{attr.data.vec2[0], attr.data.vec2[1]});
            }
                break;
            case BDFloat3Type:
            {
                addAttributeValue(attrId, Vector3f{
                        attr.data.vec3[0],
                        attr.data.vec3[1],
                        attr.data.vec3[2]
                });
            }
                break;
            case BDFloat4Type:
            {
                addAttributeValue(attrId, Vector4f{
                        attr.data.vec4[0],
                        attr.data.vec4[1],
                        attr.data.vec4[2],
                        attr.data.vec4[3]
                });
            }
                break;
            case BDChar4Type:
            {
                addAttributeValue(attrId, RGBAColor{
                        attr.data.color[0],
                        attr.data.color[1],
                        attr.data.color[2],
                        attr.data.color[3]
                });
            }
                break;
            case BDIntType:
                addAttributeValue(attrId, attr.data.intVal);
                break;
            case BDInt64Type:
                addAttributeValue(attrId, attr.data.int64Val);
                break;
            case BDDataTypeMax:
                break;
        }
    }
}

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector2f &vec)
{ basicDraw->vertexAttributes[attrId]->addVector2f(vec); }

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector3f &vec)
{ basicDraw->vertexAttributes[attrId]->addVector3f(vec); }

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector4f &vec)
{ basicDraw->vertexAttributes[attrId]->addVector4f(vec); }

void BasicDrawableBuilder::addAttributeValue(int attrId,const RGBAColor &color)
{ basicDraw->vertexAttributes[attrId]->addColor(color); }

void BasicDrawableBuilder::addAttributeValue(int attrId,float val)
{ basicDraw->vertexAttributes[attrId]->addFloat(val); }

void BasicDrawableBuilder::addAttributeValue(int attrId,int val)
{
    basicDraw->vertexAttributes[attrId]->addInt(val);
}

void BasicDrawableBuilder::addAttributeValue(int attrId,int64_t val)
{ basicDraw->vertexAttributes[attrId]->addFloat(val); }

int BasicDrawableBuilder::findAttribute(int nameID)
{
    for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
        if (basicDraw->vertexAttributes[ii]->nameID == nameID)
            return ii;
    
    return -1;
}


void BasicDrawableBuilder::addTriangle(BasicDrawable::Triangle tri)
{ tris.push_back(tri); }

void BasicDrawableBuilder::setUniforms(const SingleVertexAttributeSet &uniforms)
{
    basicDraw->uniforms = uniforms;
}

void BasicDrawableBuilder::applySubTexture(int which,const SubTexture &subTex,int startingAt)
{
    if (which == -1)
    {
        // Apply the mapping everywhere
        for (unsigned int ii=0;ii<basicDraw->texInfo.size();ii++)
        {
            applySubTexture(ii, subTex, startingAt);
        }
    }
    else
    {
        setupTexCoordEntry(which, 0);
        
        BasicDrawable::TexInfo &thisTexInfo = basicDraw->texInfo[which];
        thisTexInfo.texId = subTex.texId;
        auto *texCoords = (std::vector<TexCoord> *)basicDraw->vertexAttributes[thisTexInfo.texCoordEntry]->data;
        
        for (unsigned int ii=startingAt;ii<texCoords->size();ii++)
        {
            Point2f tc = (*texCoords)[ii];
            (*texCoords)[ii] = subTex.processTexCoord(TexCoord(tc.x(),tc.y()));
        }
    }
}

}

#include <utility>
