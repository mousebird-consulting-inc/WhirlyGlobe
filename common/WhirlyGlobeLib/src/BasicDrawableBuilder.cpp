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

void BasicDrawableBuilder::Init(SceneRenderer *sceneRender)
{
    basicDraw->colorEntry = -1;
    basicDraw->normalEntry = -1;
    
    basicDraw->on = true;
    basicDraw->hasOverrideColor = false;
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
    // Note: Set this as a default somewhere
    basicDraw->color = RGBAColor(255,255,255,255);
    basicDraw->lineWidth = 1.0;
    
    basicDraw->numTris = 0;
    basicDraw->numPoints = 0;
    
    basicDraw->requestZBuffer = false;
    basicDraw->writeZBuffer = true;
    basicDraw->renderTargetID = EmptyIdentity;
    
    basicDraw->clipCoords = false;
    
    basicDraw->hasMatrix = false;
}
    
BasicDrawableBuilder::BasicDrawableBuilder(const std::string &name,SceneRenderer *sceneRender)
{
    Init(sceneRender);
    basicDraw->name = name;
}

BasicDrawableBuilder::BasicDrawableBuilder(const std::string &name, SceneRenderer *sceneRender, unsigned int numVert,unsigned int numTri)
{
    Init(sceneRender);
    basicDraw->name = name;
    points.reserve(numVert);
    tris.reserve(numTri);
}

BasicDrawableBuilder::~BasicDrawableBuilder()
{
}

BasicDrawableBuilder::Triangle::Triangle()
{
}

BasicDrawableBuilder::Triangle::Triangle(unsigned short v0,unsigned short v1,unsigned short v2)
{
    verts[0] = v0;  verts[1] = v1;  verts[2] = v2;
    
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

/// Resulting drawable writes to the Z buffer
void BasicDrawableBuilder::setWriteZBuffer(bool val)
{
    basicDraw->writeZBuffer = val;
}

// If set, we'll render this data where directed
void BasicDrawableBuilder::setRenderTarget(SimpleIdentity newRenderTarget)
{
    basicDraw->renderTargetID = newRenderTarget;
}

/// Set the line width (if using lines)
void BasicDrawableBuilder::setLineWidth(float inWidth)
{
    basicDraw->lineWidth = inWidth;
}

void BasicDrawableBuilder::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    basicDraw->texInfo[which].texId = inId;
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


int BasicDrawableBuilder::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings = -1);

void BasicDrawableBuilder::reserveNumPoints(int numPoints);

void BasicDrawableBuilder::reserveNumTris(int numTris);

void BasicDrawableBuilder::reserveNumTexCoords(unsigned int which,int numCoords);

void BasicDrawableBuilder::reserveNumNorms(int numNorms);

void BasicDrawableBuilder::reserveNumColors(int numColors);

void BasicDrawableBuilder::setClipCoords(bool clipCoords);

unsigned int BasicDrawableBuilder::addPoint(const Point3f &pt);
unsigned int BasicDrawableBuilder::addPoint(const Point3d &pt);

Point3f BasicDrawableBuilder::getPoint(int which);

void BasicDrawableBuilder::addTexCoord(int which,TexCoord coord);

void BasicDrawableBuilder::addColor(RGBAColor color);

void BasicDrawableBuilder::addNormal(const Point3f &norm);
void BasicDrawableBuilder::addNormal(const Point3d &norm);

bool BasicDrawableBuilder::compareVertexAttributes(const SingleVertexAttributeSet &attrs);

void BasicDrawableBuilder::setVertexAttributes(const SingleVertexAttributeInfoSet &attrs);

void BasicDrawableBuilder::addVertexAttributes(const SingleVertexAttributeSet &attrs);

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector2f &vec);

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector3f &vec);

void BasicDrawableBuilder::addAttributeValue(int attrId,const Eigen::Vector4f &vec);

void BasicDrawableBuilder::addAttributeValue(int attrId,const RGBAColor &color);

void BasicDrawableBuilder::addAttributeValue(int attrId,float val);

void BasicDrawableBuilder::addTriangle(Triangle tri);

void BasicDrawableBuilder::addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center)
{
    {
        if (!points.empty())
        {
            Point3f &pt = points[which];
            
            // If there's a center, we have to offset everything first
            if (center)
            {
                Vector4d pt3d;
                if (hasMatrix)
                    pt3d = mat * Vector4d(pt.x(),pt.y(),pt.z(),1.0);
                else
                    pt3d = Vector4d(pt.x(),pt.y(),pt.z(),1.0);
                Point3f newPt(pt3d.x()-center->x(),pt3d.y()-center->y(),pt3d.z()-center->z());
                memcpy(basePtr+pointBuffer, &newPt.x(), 3*sizeof(GLfloat));
            } else {
                // Otherwise, copy it straight in
                memcpy(basePtr+pointBuffer, &pt.x(), 3*sizeof(GLfloat));
            }
        }
        
        for (VertexAttribute *attr : vertexAttributes)
        {
            if (attr->numElements() != 0)
                memcpy(basePtr+attr->buffer, attr->addressForElement(which), attr->size());
        }
    }
}

void BasicDrawableBuilder::setUniforms(const SingleVertexAttributeSet &uniforms);

void BasicDrawableBuilder::applySubTexture(int which,SubTexture subTex,int startingAt=0);

RawDataRef BasicDrawableBuilder::asData(bool dupStart,bool dupEnd)
{
    MutableRawDataRef retData;
    if (points.empty())
        return retData;
    
    // Verify that everything else (that has data) has the same amount)
    int numElements = (int)points.size();
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttribute *attr = vertexAttributes[ii];
        int theseElements = attr->numElements();
        if (theseElements != 0 && theseElements != numElements)
            return retData;
    }
    
    if (type == GL_TRIANGLE_STRIP || type == GL_POINTS || type == GL_LINES || type == GL_LINE_STRIP)
    {
        vertexSize = singleVertexSize();
        int numVerts = (int)(points.size() + (dupStart ? 2 : 0) + (dupEnd ? 2 : 0));
        
        retData = MutableRawDataRef(new MutableRawData(vertexSize*numVerts));
        unsigned char *basePtr = (unsigned char *)retData->getRawData();
        if (dupStart)
        {
            addPointToBuffer(basePtr, 0, NULL);
            basePtr += vertexSize;
            addPointToBuffer(basePtr, 0, NULL);
            basePtr += vertexSize;
        }
        for (unsigned int ii=0;ii<points.size();ii++,basePtr+=vertexSize)
            addPointToBuffer(basePtr, ii, NULL);
        if (dupEnd)
        {
            addPointToBuffer(basePtr, (int)(points.size()-1), NULL);
            basePtr += vertexSize;
            addPointToBuffer(basePtr, (int)(points.size()-1), NULL);
            basePtr += vertexSize;
        }
    }
    
    return retData;
}

void BasicDrawableBuilder::asVertexAndElementData(MutableRawDataRef retVertData,RawDataRef retElementData,int singleElementSize,const Point3d *center)
{
    if (type != GL_TRIANGLES)
        return;
    if (points.empty() || tris.empty())
        return;
    
    // Verify that everything else (that has data) has the same amount)
    int numElements = (int)points.size();
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttribute *attr = vertexAttributes[ii];
        int theseElements = attr->numElements();
        if (theseElements != 0 && theseElements != numElements)
            return;
    }
    
    // Build up the vertices
    vertexSize = singleVertexSize();
    int numVerts = (int)points.size();
    vertData = MutableRawDataRef(new MutableRawData(vertexSize * numVerts));
    unsigned char *basePtr = (unsigned char *)vertData->getRawData();
    for (unsigned int ii=0;ii<points.size();ii++,basePtr+=vertexSize)
        addPointToBuffer(basePtr, ii, center);
        
        // Build up the triangles
        int triSize = singleElementSize * 3;
        int numTris = (int)tris.size();
        int totSize = numTris*triSize;
        elementData = MutableRawDataRef(new MutableRawData(totSize));
        GLushort *elPtr = (GLushort *)elementData->getRawData();
        for (unsigned int ii=0;ii<tris.size();ii++,elPtr+=3)
        {
            Triangle &tri = tris[ii];
            for (unsigned int jj=0;jj<3;jj++)
            {
                unsigned short vertId = tri.verts[jj];
                elPtr[jj] = vertId;
            }
        }
}


}
