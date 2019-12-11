/*
 *  BasicDrawableBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "BasicDrawableBuilder.h"
#import "SceneRenderer.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableBuilder::BasicDrawableBuilder()
    : basicDraw(NULL)
{
}
    
BasicDrawableBuilder::BasicDrawableBuilder(const std::string &name)
    : name(name), basicDraw(NULL)
{
}
    
void BasicDrawableBuilder::setName(const std::string &name)
{
    basicDraw->name = name;
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
    basicDraw->drawPriority = 0;
    basicDraw->drawOffset = 0;
    basicDraw->type = Points;
    basicDraw->minVisible = basicDraw->maxVisible = DrawVisibleInvalid;
    basicDraw->minVisibleFadeBand = basicDraw->maxVisibleFadeBand = 0.0;
    basicDraw->minViewerDist = basicDraw->maxViewerDist = DrawVisibleInvalid;
    basicDraw->viewerCenter = Point3d(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid);
    
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
}
    
void BasicDrawableBuilder::setupStandardAttributes(int numReserve)
{
    //    setupTexCoordEntry(0,numReserve);
    
    basicDraw->colorEntry = addAttribute(BDChar4Type,a_colorNameID);
    basicDraw->vertexAttributes[basicDraw->colorEntry]->setDefaultColor(RGBAColor(255,255,255,255));
    basicDraw->vertexAttributes[basicDraw->colorEntry]->reserve(numReserve);
    
    basicDraw->normalEntry = addAttribute(BDFloat3Type,a_normalNameID);
    basicDraw->vertexAttributes[basicDraw->normalEntry]->setDefaultVector3f(Vector3f(1.0,1.0,1.0));
    basicDraw->vertexAttributes[basicDraw->normalEntry]->reserve(numReserve);
}
    
SimpleIdentity BasicDrawableBuilder::getDrawableID()
{
    if (basicDraw)
        return basicDraw->getId();
    return EmptyIdentity;
}
    
int BasicDrawableBuilder::getDrawablePriority()
{
   if (basicDraw)
       return basicDraw->getDrawPriority();
    return 0;
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
    
BasicDrawableBuilder::~BasicDrawableBuilder()
{
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

void BasicDrawableBuilder::setAlpha(bool onOff)
{
    basicDraw->isAlpha = onOff;
}

void BasicDrawableBuilder::setDrawOffset(float newOffset)
{
    basicDraw->drawOffset = newOffset;
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
    
float BasicDrawableBuilder::getLineWidth()
{
    return basicDraw->lineWidth;
}

void BasicDrawableBuilder::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    basicDraw->texInfo[which].texId = inId;
}

SimpleIdentity BasicDrawableBuilder::getTexId(unsigned int which)
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

void BasicDrawableBuilder::addTweaker(DrawableTweakerRef tweakRef)
{
    basicDraw->tweakers.insert(tweakRef);
}

void BasicDrawableBuilder::setType(GeometryType inType)
{
    basicDraw->type = inType;
}

void BasicDrawableBuilder::setColor(RGBAColor color)
{
    if (basicDraw->colorEntry >= 0)
        basicDraw->vertexAttributes[basicDraw->colorEntry]->setDefaultColor(color);
}

void BasicDrawableBuilder::setColor(unsigned char color[])
{
    setColor(RGBAColor(color[0],color[1],color[2],color[3]));
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
    
unsigned int BasicDrawableBuilder::getNumPoints()
{
    return points.size();
}

unsigned int BasicDrawableBuilder::getNumTris()
{
    return tris.size();
}

Point3d BasicDrawableBuilder::getPoint(int which)
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

bool BasicDrawableBuilder::compareVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    for (SingleVertexAttributeSet::iterator it = attrs.begin();
         it != attrs.end(); ++it)
    {
        int attrId = -1;
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
            if (basicDraw->vertexAttributes[ii]->nameID == it->nameID)
            {
                attrId = ii;
                break;
            }
        if (attrId == -1)
            return false;
        if (basicDraw->vertexAttributes[attrId]->getDataType() != it->type)
            return false;
    }
    
    return true;
}

void BasicDrawableBuilder::setVertexAttributes(const SingleVertexAttributeInfoSet &attrs)
{
    for (auto it = attrs.begin();
         it != attrs.end(); ++it)
        addAttribute(it->type,it->nameID);
}

void BasicDrawableBuilder::addVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    for (SingleVertexAttributeSet::iterator it = attrs.begin();
         it != attrs.end(); ++it)
    {
        int attrId = -1;
        for (unsigned int ii=0;ii<basicDraw->vertexAttributes.size();ii++)
            if (basicDraw->vertexAttributes[ii]->nameID == it->nameID)
            {
                attrId = ii;
                break;
            }
        
        if (attrId == -1)
            continue;
        
        switch (it->type)
        {
            case BDFloatType:
                addAttributeValue(attrId, it->data.floatVal);
                break;
            case BDFloat2Type:
            {
                Vector2f vec;
                vec.x() = it->data.vec2[0];
                vec.y() = it->data.vec2[1];
                addAttributeValue(attrId, vec);
            }
                break;
            case BDFloat3Type:
            {
                Vector3f vec;
                vec.x() = it->data.vec3[0];
                vec.y() = it->data.vec3[1];
                vec.z() = it->data.vec3[2];
                addAttributeValue(attrId, vec);
            }
                break;
            case BDFloat4Type:
            {
                Vector4f vec;
                vec.x() = it->data.vec4[0];
                vec.y() = it->data.vec4[1];
                vec.z() = it->data.vec4[2];
                vec.w() = it->data.vec4[3];
                addAttributeValue(attrId, vec);
            }
                break;
            case BDChar4Type:
            {
                RGBAColor color;
                color.r = it->data.color[0];
                color.g = it->data.color[1];
                color.b = it->data.color[2];
                color.a = it->data.color[3];
                addAttributeValue(attrId, color);
            }
                break;
            case BDIntType:
                addAttributeValue(attrId, it->data.intVal);
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

void BasicDrawableBuilder::addTriangle(BasicDrawable::Triangle tri)
{ tris.push_back(tri); }

void BasicDrawableBuilder::setUniforms(const SingleVertexAttributeSet &uniforms)
{
    basicDraw->uniforms = uniforms;
}

void BasicDrawableBuilder::applySubTexture(int which,SubTexture subTex,int startingAt)
{
    if (which == -1)
    {
        // Apply the mapping everywhere
        for (unsigned int ii=0;ii<basicDraw->texInfo.size();ii++)
            applySubTexture(ii, subTex, startingAt);
    } else {
        setupTexCoordEntry(which, 0);
        
        BasicDrawable::TexInfo &thisTexInfo = basicDraw->texInfo[which];
        thisTexInfo.texId = subTex.texId;
        std::vector<TexCoord> *texCoords = (std::vector<TexCoord> *)basicDraw->vertexAttributes[thisTexInfo.texCoordEntry]->data;
        
        for (unsigned int ii=startingAt;ii<texCoords->size();ii++)
        {
            Point2f tc = (*texCoords)[ii];
            (*texCoords)[ii] = subTex.processTexCoord(TexCoord(tc.x(),tc.y()));
        }
    }
}
    
}
