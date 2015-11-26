/*
 *  BasicDrawable.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import "GLUtils.h"
#import "Drawable.h"
#import "BasicDrawable.h"
#import "SceneRendererES.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawable::BasicDrawable(const std::string &name)
: Drawable(name)
{
    on = true;
    startEnable = 0.0;
    endEnable = 0.0;
    programId = EmptyIdentity;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
    type = 0;
    minVisible = maxVisible = DrawVisibleInvalid;
    minVisibleFadeBand = maxVisibleFadeBand = 0.0;
    minViewerDist = maxViewerDist = DrawVisibleInvalid;
    viewerCenter = Point3d(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid);
    
    fadeDown = fadeUp = 0.0;
    color.r = color.g = color.b = color.a = 255;
    lineWidth = 1.0;
    
    numTris = 0;
    numPoints = 0;
    
    pointBuffer = triBuffer = 0;
    sharedBuffer = 0;
    vertexSize = 0;
    vertArrayObj = 0;
    sharedBufferIsExternal = false;
    requestZBuffer = false;
    writeZBuffer = true;
    
    hasMatrix = false;
    
    setupStandardAttributes();
}

BasicDrawable::BasicDrawable(const std::string &name,unsigned int numVert,unsigned int numTri)
: Drawable(name)
{
    on = true;
    startEnable = 0.0;
    endEnable = 0.0;
    programId = EmptyIdentity;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
    points.reserve(numVert);
    setupStandardAttributes(numVert);
    tris.reserve(numTri);
    fadeDown = fadeUp = 0.0;
    color.r = color.g = color.b = color.a = 255;
    lineWidth = 1.0;
    drawPriority = 0;
    minVisible = maxVisible = DrawVisibleInvalid;
    minVisibleFadeBand = maxVisibleFadeBand = 0.0;
    minViewerDist = maxViewerDist = DrawVisibleInvalid;
    viewerCenter = Point3d(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid);
    requestZBuffer = false;
    writeZBuffer = true;
    
    numTris = 0;
    numPoints = 0;
    
    pointBuffer = triBuffer = 0;
    sharedBuffer = 0;
    vertexSize = 0;
    vertArrayObj = 0;
    sharedBufferIsExternal = false;
    
    hasMatrix = false;
}

BasicDrawable::~BasicDrawable()
{
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        delete vertexAttributes[ii];
    vertexAttributes.clear();
}

void BasicDrawable::setupTexCoordEntry(int which,int numReserve)
{
    if (which < texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)texInfo.size();ii<=which;ii++)
    {
        TexInfo newInfo;
        char attributeName[40];
        sprintf(attributeName,"a_texCoord%d",ii);
        newInfo.texCoordEntry = addAttribute(BDFloat2Type,attributeName);
        vertexAttributes[newInfo.texCoordEntry]->setDefaultVector2f(Vector2f(0.0,0.0));
        vertexAttributes[newInfo.texCoordEntry]->reserve(numReserve);
        texInfo.push_back(newInfo);
    }
}

void BasicDrawable::setupStandardAttributes(int numReserve)
{
    setupTexCoordEntry(0,numReserve);
    
    colorEntry = addAttribute(BDChar4Type,"a_color");
    vertexAttributes[colorEntry]->setDefaultColor(RGBAColor(255,255,255,255));
    vertexAttributes[colorEntry]->reserve(numReserve);
    
    normalEntry = addAttribute(BDFloat3Type,"a_normal");
    vertexAttributes[normalEntry]->setDefaultVector3f(Vector3f(1.0,1.0,1.0));
    vertexAttributes[normalEntry]->reserve(numReserve);
}

SimpleIdentity BasicDrawable::getProgram() const
{
    return programId;
}

void BasicDrawable::setProgram(SimpleIdentity progId)
{
    programId = progId;
}

unsigned int BasicDrawable::getDrawPriority() const
{
    return drawPriority;
}

bool BasicDrawable::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable ||
            endEnable < frameInfo->currentTime)
            return false;
    }
    
    if (!on)
        return false;
    
    double visVal = frameInfo->theView->heightAboveSurface();

    // Height based check
    if (minVisible != DrawVisibleInvalid && maxVisible != DrawVisibleInvalid)
    {
        if (!((minVisible <= visVal && visVal <= maxVisible) ||
                (maxVisible <= visVal && visVal <= minVisible)))
            return false;
    }
    
    // Viewer based check
    if (minViewerDist != DrawVisibleInvalid && maxViewerDist != DrawVisibleInvalid &&
        viewerCenter.x() != DrawVisibleInvalid)
    {
        double dist2 = (viewerCenter - frameInfo->eyePos).squaredNorm();
        if (!(minViewerDist*minViewerDist < dist2 && dist2 <= maxViewerDist*maxViewerDist))
            return false;
    }
    
    return true;    
}

void BasicDrawable::setOnOff(bool onOff)
{
    on = onOff;
}

bool BasicDrawable::hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (isAlpha)
        return true;
    
    // We don't need to get tricky unless we're z buffering this data
    if (!requestZBuffer)
        return false;
    
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < fadeDown)
            return false;
        else
            if (frameInfo->currentTime > fadeUp)
                return false;
            else
                return true;
    } else
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < fadeUp)
                return false;
            else
                if (frameInfo->currentTime > fadeDown)
                    return false;
                else
                    return true;
        }
    
    // Note: Need to move this elsewhere
    WhirlyGlobe::GlobeView *globeView = dynamic_cast<WhirlyGlobe::GlobeView *>(frameInfo->theView);
    if ((minVisibleFadeBand != 0.0 || maxVisibleFadeBand != 0.0) && globeView)
    {
        float height = globeView->heightAboveSurface();
        if (height > minVisible && height < minVisible + minVisibleFadeBand)
        {
            return true;
        } else if (height > maxVisible - maxVisibleFadeBand && height < maxVisible)
        {
            return true;
        }
    }
    
    return false;
}

void BasicDrawable::setAlpha(bool onOff)
{
    isAlpha = onOff;
}

Mbr BasicDrawable::getLocalMbr() const
{
    return localMbr;
}


void BasicDrawable::setLocalMbr(Mbr mbr)
{
    localMbr = mbr;
}

void BasicDrawable::setDrawPriority(unsigned int newPriority)
{
    drawPriority = newPriority;
}

unsigned int BasicDrawable::getDrawPriority()
{
    return drawPriority;
}

void BasicDrawable::setDrawOffset(float newOffset)
{
    drawOffset = newOffset;
}

float BasicDrawable::getDrawOffset()
{
    return drawOffset;
}

void BasicDrawable::setType(GLenum inType)
{
    type = inType;
}

GLenum BasicDrawable::getType() const
{
    return type;
}

void BasicDrawable::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    texInfo[which].texId = inId;
}

void BasicDrawable::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<texIDs.size();ii++)
    {
        setupTexCoordEntry(ii, 0);
        texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawable::setColor(RGBAColor inColor)
{
    color = inColor;
    vertexAttributes[colorEntry]->setDefaultColor(color);
}

/// Set the color as an array.
void BasicDrawable::setColor(unsigned char inColor[])
{
    color.r = inColor[0];  color.g = inColor[1];  color.b = inColor[2];  color.a = inColor[3];
    vertexAttributes[colorEntry]->setDefaultColor(color);
}

RGBAColor BasicDrawable::getColor() const
{
    return color;
}

void BasicDrawable::setVisibleRange(float minVis,float maxVis,float minVisBand,float maxVisBand)
{ minVisible = minVis;  maxVisible = maxVis;  minVisibleFadeBand = minVisBand; maxVisibleFadeBand = maxVisBand; }

void BasicDrawable::getVisibleRange(float &minVis,float &maxVis)
{ minVis = minVisible;  maxVis = maxVisible; }

void BasicDrawable::getVisibleRange(float &minVis,float &maxVis,float &minVisBand,float &maxVisBand)
{ minVis = minVisible; maxVis = maxVisible;  minVisBand = minVisibleFadeBand; maxVisBand = maxVisibleFadeBand; }

void BasicDrawable::setViewerVisibility(double inMinViewerDist,double inMaxViewerDist,const Point3d &inViewerCenter)
{
    minViewerDist = inMinViewerDist;
    maxViewerDist = inMaxViewerDist;
    viewerCenter = inViewerCenter;
}

void BasicDrawable::getViewerVisibility(double &outMinViewerDist,double &outMaxViewerDist,Point3d &outViewerCenter)
{
    outMinViewerDist = minViewerDist;
    outMaxViewerDist = maxViewerDist;
    outViewerCenter = viewerCenter;
}
void BasicDrawable::setFade(TimeInterval inFadeDown,TimeInterval inFadeUp)
{ fadeUp = inFadeUp;  fadeDown = inFadeDown; }

void BasicDrawable::setLineWidth(float inWidth)
{ lineWidth = inWidth; }

float BasicDrawable::getLineWidth()
{ return lineWidth; }

void BasicDrawable::setRequestZBuffer(bool val)
{ requestZBuffer = val; }

bool BasicDrawable::getRequestZBuffer() const
{ return requestZBuffer; }

void BasicDrawable::setWriteZBuffer(bool val)
{ writeZBuffer = val; }

bool BasicDrawable::getWriteZbuffer() const
{ if (type == GL_LINES || type == GL_LINE_LOOP || type == GL_POINTS) return false;  return writeZBuffer; }

unsigned int BasicDrawable::addPoint(const Point3f &pt)
{
    points.push_back(pt);
    return (unsigned int)(points.size()-1);
}

unsigned int BasicDrawable::addPoint(const Point3d &pt)
{
    points.push_back(Point3f(pt.x(),pt.y(),pt.z()));
    return (unsigned int)(points.size()-1);
}

Point3f BasicDrawable::getPoint(int which)
{
    if (which >= points.size())
        return Point3f(0,0,0);
    return points[which];
}

void BasicDrawable::addTexCoord(int which,TexCoord coord)
{
    if (which == -1)
    {
        // In this mode, add duplicate texture coords in each of the vertex attrs
        // Note: This could be optimized to a single set of vertex attrs for all the texture coords
        for (unsigned int ii=0;ii<texInfo.size();ii++)
            vertexAttributes[texInfo[ii].texCoordEntry]->addVector2f(coord);
    } else {
        setupTexCoordEntry(which, 0);
        vertexAttributes[texInfo[which].texCoordEntry]->addVector2f(coord);
    }
}

void BasicDrawable::addColor(RGBAColor color)
{ vertexAttributes[colorEntry]->addColor(color); }

void BasicDrawable::addNormal(const Point3f &norm)
{ vertexAttributes[normalEntry]->addVector3f(norm); }

void BasicDrawable::addNormal(const Point3d &norm)
{ vertexAttributes[normalEntry]->addVector3f(Point3f(norm.x(),norm.y(),norm.z())); }

void BasicDrawable::addAttributeValue(int attrId,const Eigen::Vector2f &vec)
{ vertexAttributes[attrId]->addVector2f(vec); }

void BasicDrawable::addAttributeValue(int attrId,const Eigen::Vector3f &vec)
{ vertexAttributes[attrId]->addVector3f(vec); }

void BasicDrawable::addAttributeValue(int attrId,const Eigen::Vector4f &vec)
{ vertexAttributes[attrId]->addVector4f(vec); }

void BasicDrawable::addAttributeValue(int attrId,const  RGBAColor &color)
{ vertexAttributes[attrId]->addColor(color); }

void BasicDrawable::addAttributeValue(int attrId,float val)
{ vertexAttributes[attrId]->addFloat(val); }

bool BasicDrawable::compareVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    for (SingleVertexAttributeSet::iterator it = attrs.begin();
         it != attrs.end(); ++it)
    {
        int attrId = -1;
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            if (vertexAttributes[ii]->name == it->name)
            {
                attrId = ii;
                break;
            }
        if (attrId == -1)
            return false;
        if (vertexAttributes[attrId]->getDataType() != it->type)
            return false;
    }
    
    return true;
}

void BasicDrawable::setVertexAttributes(const SingleVertexAttributeInfoSet &attrs)
{
    for (auto it = attrs.begin();
         it != attrs.end(); ++it)
        addAttribute(it->type,it->name);
}

void BasicDrawable::addVertexAttributes(const SingleVertexAttributeSet &attrs)
{
    for (SingleVertexAttributeSet::iterator it = attrs.begin();
         it != attrs.end(); ++it)
    {
        int attrId = -1;
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            if (vertexAttributes[ii]->name == it->name)
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
        }
    }
}

void BasicDrawable::addTriangle(Triangle tri)
{ tris.push_back(tri); }

SimpleIdentity BasicDrawable::getTexId(unsigned int which)
{
    SimpleIdentity texId = EmptyIdentity;
    if (which < texInfo.size())
        texId = texInfo[which].texId;
    
    return texId;
}

// If we're fading in or out, update the rendering window
void BasicDrawable::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    renderer->setRenderUntil(fadeUp);
    renderer->setRenderUntil(fadeDown);
    
    // Let's also pull the default shaders out if need be
    if (programId == EmptyIdentity)
    {
        if (type == GL_LINE_LOOP || type == GL_LINES)
            programId = renderer->getScene()->getProgramIDBySceneName(kSceneDefaultLineShader);
        else
            programId = renderer->getScene()->getProgramIDBySceneName(kSceneDefaultTriShader);
    }
}

// Move the texture coordinates around and apply a new texture
void BasicDrawable::applySubTexture(int which,SubTexture subTex,int startingAt)
{
    if (which == -1)
    {
        // Apply the mapping everywhere
        for (unsigned int ii=0;ii<texInfo.size();ii++)
            applySubTexture(ii, subTex, startingAt);
    } else {
        setupTexCoordEntry(which, 0);
        
        TexInfo &thisTexInfo = texInfo[which];
        thisTexInfo.texId = subTex.texId;
        std::vector<TexCoord> *texCoords = (std::vector<TexCoord> *)vertexAttributes[thisTexInfo.texCoordEntry]->data;
        
        for (unsigned int ii=startingAt;ii<texCoords->size();ii++)
        {
            Point2f tc = (*texCoords)[ii];
            (*texCoords)[ii] = subTex.processTexCoord(TexCoord(tc.x(),tc.y()));
        }
    }
}

int BasicDrawable::addAttribute(BDAttributeDataType dataType,const std::string &name)
{
    VertexAttribute *attr = new VertexAttribute(dataType,name);
    vertexAttributes.push_back(attr);
    
    return (unsigned int)(vertexAttributes.size()-1);
}

unsigned int BasicDrawable::getNumPoints() const
{ return (unsigned int)points.size(); }

unsigned int BasicDrawable::getNumTris() const
{ return (unsigned int)tris.size(); }

void BasicDrawable::reserveNumPoints(int numPoints)
{ points.reserve(points.size()+numPoints); }

void BasicDrawable::reserveNumTris(int numTris)
{ tris.reserve(tris.size()+numTris); }

void BasicDrawable::reserveNumTexCoords(unsigned int which,int numCoords)
{
    setupTexCoordEntry(which, numCoords);
    vertexAttributes[texInfo[which].texCoordEntry]->reserve(numCoords);
}

void BasicDrawable::reserveNumNorms(int numNorms)
{ vertexAttributes[normalEntry]->reserve(numNorms); }

void BasicDrawable::reserveNumColors(int numColors)
{
    vertexAttributes[colorEntry]->reserve(numColors);
}

void BasicDrawable::setMatrix(const Eigen::Matrix4d *inMat)
{ mat = *inMat; hasMatrix = true; }

/// Return the active transform matrix, if we have one
const Eigen::Matrix4d *BasicDrawable::getMatrix() const
{ if (hasMatrix) return &mat;  return NULL; }

// Size of a single vertex in an interleaved buffer
// Note: We're resetting the buffers for no good reason
GLuint BasicDrawable::singleVertexSize()
{
    GLuint singleVertSize = 0;
    
    // Always have points
    if (!points.empty())
    {
        pointBuffer = singleVertSize;
        singleVertSize += 3*sizeof(GLfloat);
    }
    
    // Now for the rest of the buffers
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttribute *attr = vertexAttributes[ii];
        if (attr->numElements() != 0)
        {
            attr->buffer = singleVertSize;
            singleVertSize += attr->size();
        }
    }
    
    return singleVertSize;
}

// Adds the basic vertex data to an interleaved vertex buffer
void BasicDrawable::addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center)
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
    
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        VertexAttribute *attr = vertexAttributes[ii];
        if (attr->numElements() != 0)
            memcpy(basePtr+attr->buffer, attr->addressForElement(which), attr->size());
    }
}

void BasicDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    setupGL(setupInfo,memManager,0,0);
}

// Create VBOs and such
void BasicDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,GLuint externalSharedBuf,GLuint externalSharedBufOffset)
{
    // If we're already setup, don't do it twice
    if (pointBuffer || sharedBuffer)
        return;
    
    // Offset the geometry upward by minZres units along the normals
    // Only do this once, obviously
    // Note: Probably replace this with a shader program at some point
    if (drawOffset != 0 && (points.size() == vertexAttributes[normalEntry]->numElements()))
    {
        float scale = setupInfo->minZres*drawOffset;
        Point3fVector &norms = *(Point3fVector *)vertexAttributes[normalEntry]->data;
        
        for (unsigned int ii=0;ii<points.size();ii++)
        {
            Vector3f pt = points[ii];
            points[ii] = norms[ii] * scale + pt;
        }
    }
    
    pointBuffer = triBuffer = 0;
    sharedBuffer = 0;
    
    // We'll set up a single buffer for everything.
    // The other buffer pointers are now strides
    // Size of a single vertex entry
    vertexSize = singleVertexSize();
    int numVerts = (int)points.size();
    
    // We're handed an external buffer, so just use it
    int bufferSize = 0;
    if (externalSharedBuf)
    {
        sharedBuffer = externalSharedBuf;
        sharedBufferOffset = externalSharedBufOffset;
        sharedBufferIsExternal = true;
    } else {
        // Set up the buffer
        int bufferSize = vertexSize*numVerts;
        if (!tris.empty())
        {
            bufferSize += tris.size()*sizeof(Triangle);
        }
        sharedBuffer = memManager->getBufferID(bufferSize,GL_STATIC_DRAW);
        sharedBufferOffset = 0;
        sharedBufferIsExternal = false;
    }
    
    // Now copy in the data
    // Note: OpenGL ES 3.0 has mapped buffer support, but it's different
    glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
    if (hasMapBufferSupport)
    {
        // Note: Porting
        //        void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
        //        unsigned char *basePtr = (unsigned char *)glMem + sharedBufferOffset;
        //        for (unsigned int ii=0;ii<numVerts;ii++,basePtr+=vertexSize)
        //            addPointToBuffer(basePtr,ii,NULL);
        //
        //        // And copy in the element buffer
        //        if (tris.size())
        //        {
        //            triBuffer = vertexSize*numVerts;
        //            unsigned char *basePtr = (unsigned char *)glMem + triBuffer + sharedBufferOffset;
        //            for (unsigned int ii=0;ii<tris.size();ii++,basePtr+=sizeof(Triangle))
        //                memcpy(basePtr, &tris[ii], sizeof(Triangle));
        //        }
        //        glUnmapBufferOES(GL_ARRAY_BUFFER);
    } else {
        bufferSize = numVerts*vertexSize+tris.size()*sizeof(Triangle);
        
        // Gotta do this the hard way
        unsigned char *glMem = (unsigned char *)malloc(bufferSize);
        unsigned char *basePtr = glMem;
        for (unsigned int ii=0;ii<numVerts;ii++,basePtr+=vertexSize)
            addPointToBuffer(basePtr, ii,NULL);
        
        // Now the element buffer
        triBuffer = numVerts*vertexSize;
        for (unsigned int ii=0;ii<tris.size();ii++,basePtr+=sizeof(Triangle))
            memcpy(basePtr, &tris[ii], sizeof(Triangle));
        
        glBufferData(GL_ARRAY_BUFFER, bufferSize, glMem, GL_STATIC_DRAW);
        free(glMem);
    }
    
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Clear out the arrays, since we won't need them again
    numPoints = points.size();
    points.clear();
    numTris = tris.size();
    tris.clear();
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        vertexAttributes[ii]->clear();
    
    usingBuffers = true;
}

// Instead of copying data to an OpenGL buffer, we'll just put it in an NSData
RawDataRef BasicDrawable::asData(bool dupStart,bool dupEnd)
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
        int numVerts = (int)points.size() + (dupStart ? 2 : 0) + (dupEnd ? 2 : 0);
        
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

void BasicDrawable::asVertexAndElementData(MutableRawDataRef &vertData,MutableRawDataRef &elementData,int singleElementSize,const Point3d *center)
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

const std::vector<VertexAttribute *> &BasicDrawable::getVertexAttributes()
{
    return vertexAttributes;
}

// Note: Closed for repairs
#if 0
// Combined vertex used in triangle stripping
struct TmpVert
{
    TmpVert() { }
    TmpVert(BasicDrawable *draw,int which)
    {
        vert = draw->getPoint(which);
        texCoord = draw->getTexCoord(which);
        norm = draw->getNormal(which);
        color = draw->getColor(which);
    }
    bool operator == (const TmpVert &that)
    {
        return vert.x() == that.vert.x() && vert.y() == that.vert.y() && vert.z() == that.vert.z() &&
        texCoord.x() == that.texCoord.x() && texCoord.y() && that.texCoord.y() &&
        norm.x() == that.norm.x() && norm.y() == that.norm.y() && norm.z() == that.norm.z() &&
        color.r == that.color.r && color.g == that.color.g && color.b == that.color.b && color.a == that.color.a;
    }
    Point3f vert;
    Point2f texCoord;
    Point3f norm;
    RGBAColor color;
};

class TriStripBuilder
{
public:
    TriStripBuilder(BasicDrawable *draw) : draw(draw), flip(false) { }
    
    void addTriangle(TmpVert verts[3])
    {
        if (lastVerts.empty())
        {
            addVert(verts[0]);
            addVert(verts[1]);
            addVert(verts[2]);
            lastVerts.push_back(verts[1]);
            lastVerts.push_back(verts[2]);
        } else {
            // See if this triangle shares two vertices with the last one
            int matchOffset = 3;
            if (lastVerts.size() == 2)
            {
                // Try rotating the vertices until we find a match
                for (matchOffset=0;matchOffset<3;matchOffset++)
                    if (verts[matchOffset] == lastVerts[(flip ? 1 : 0)] &&
                        verts[(matchOffset+1)%3] == lastVerts[(flip ? 0 : 1)])
                        break;
            }
            
            // Found one that worked
            if (matchOffset != 3)
            {
                addVert(verts[(matchOffset+2)%3]);
                lastVerts[0] = lastVerts[1];
                lastVerts[1] = verts[(matchOffset+2)%3];
            } else {
                // Otherwise, repeat the last vertex and the first vertex of the new triangle
                addVert(lastVerts[1]);
                lastVerts.clear();
                if (flip)
                {
                    addVert(verts[2]);
                    addVert(verts[2]);
                    addVert(verts[1]);
                    addVert(verts[0]);
                    lastVerts.push_back(verts[1]);
                    lastVerts.push_back(verts[0]);
                } else {
                    addVert(verts[0]);
                    addVert(verts[0]);
                    addVert(verts[1]);
                    addVert(verts[2]);
                    lastVerts.push_back(verts[1]);
                    lastVerts.push_back(verts[2]);
                }
            }
        }
        
        flip = !flip;
    }
    
    void addVert(TmpVert vert)
    {
        draw->addPoint(vert.vert);
        draw->addTexCoord(TexCoord(vert.texCoord.x(),vert.texCoord.y()));
        draw->addNormal(vert.norm);
        draw->addColor(vert.color);
    }
    
    BasicDrawable *draw;
    bool flip;
    std::vector<TmpVert> lastVerts;
};

void BasicDrawable::convertToTriStrip()
{
    if (type != GL_TRIANGLES)
        return;
    
    if (points.empty() || tris.empty() ||
        points.size() != texCoords.size() || points.size() != norms.size())
        return;
    
    // Set up a temporary drawable to capture the data
    BasicDrawable tmpDraw("tmp");
    tmpDraw.setType(GL_TRIANGLE_STRIP);
    
    TriStripBuilder stripBuilder(&tmpDraw);
    
    // Add the triangles one by one and let the
    for (unsigned int ii=0;ii<tris.size();ii++)
    {
        Triangle &tri = tris[ii];
        TmpVert verts[3];
        for (unsigned int jj=0;jj<3;jj++)
            verts[jj] = TmpVert(this,tri.verts[jj]);
        stripBuilder.addTriangle(verts);
    }
    
    setType(GL_TRIANGLE_STRIP);
    numPoints = 0;
    numTris = 0;
    points = tmpDraw.points;
    colors = tmpDraw.colors;
    texCoords = tmpDraw.texCoords;
    norms = tmpDraw.norms;
    tris.clear();
}
#endif

// Tear down the VBOs we set up
void BasicDrawable::teardownGL(OpenGLMemManager *memManager)
{
    if (vertArrayObj)
        glDeleteVertexArrays(1,&vertArrayObj);
    vertArrayObj = 0;
    
    if (sharedBuffer && !sharedBufferIsExternal)
    {
        memManager->removeBufferID(sharedBuffer);
        sharedBuffer = 0;
    } else {
        if (pointBuffer)
            memManager->removeBufferID(pointBuffer);
        if (triBuffer)
            memManager->removeBufferID(triBuffer);
    }
    pointBuffer = 0;
    triBuffer = 0;
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        vertexAttributes[ii]->buffer = 0;
}

void BasicDrawable::draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene)
{
    drawOGL2(frameInfo,scene);
}

// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)((uintptr_t)(base)) + (off))


// Called once to set up a Vertex Array Object
GLuint BasicDrawable::setupVAO(OpenGLES2Program *prog)
{
    GLuint theVertArrayObj;
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
    
    glGenVertexArrays(1, &theVertArrayObj);
    glBindVertexArray(theVertArrayObj);
    
    // We're using a single buffer for all of our vertex attributes
    if (sharedBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER,sharedBuffer);
        CheckGLError("BasicDrawable::drawVBO2() shared glBindBuffer");
    }
    
    // Vertex array
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(sharedBufferOffset,0));
        glEnableVertexAttribArray ( vertAttr->index );
    }
    
    // All the rest of the attributes
    const OpenGLESAttribute *progAttrs[vertexAttributes.size()];
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    {
        progAttrs[ii] = NULL;
        VertexAttribute *attr = vertexAttributes[ii];
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attr->name);
        if (thisAttr && (attr->buffer != 0 || attr->numElements() != 0))
        {
            glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), vertexSize, CALCBUFOFF(sharedBufferOffset,attr->buffer));
            glEnableVertexAttribArray(thisAttr->index);
            progAttrs[ii] = thisAttr;
        }
    }
    
    // Bind the element array
    bool boundElements = false;
    if (type == GL_TRIANGLES && triBuffer)
    {
        boundElements = true;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
        CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
    }
    
    glBindVertexArray(0);
    
    // Let a subclass set up their own VAO state
    setupAdditionalVAO(prog,theVertArrayObj);
    
    // Now tear down all that state
    if (vertAttr)
        glDisableVertexAttribArray(vertAttr->index);
    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        if (progAttrs[ii])
            glDisableVertexAttribArray(progAttrs[ii]->index);
    if (boundElements)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (sharedBuffer)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return theVertArrayObj;
}

// Draw Vertex Buffer Objects, OpenGL 2.0
void BasicDrawable::drawOGL2(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene)
{
    OpenGLES2Program *prog = frameInfo->program;
    
    // Figure out if we're fading in or out
    float fade = 1.0;
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo->currentTime < fadeDown)
            fade = 0.0;
        else
            if (frameInfo->currentTime > fadeUp)
                fade = 1.0;
            else
                fade = (frameInfo->currentTime - fadeDown)/(fadeUp - fadeDown);
    } else {
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo->currentTime < fadeUp)
                fade = 1.0;
            else
                if (frameInfo->currentTime > fadeDown)
                    fade = 0.0;
                else
                    fade = 1.0-(frameInfo->currentTime - fadeUp)/(fadeDown - fadeUp);
        }
    }
    // Deal with the range based fade
    if (frameInfo->heightAboveSurface > 0.0)
    {
        float factor = 1.0;
        if (minVisibleFadeBand != 0.0)
        {
            float a = (frameInfo->heightAboveSurface - minVisible)/minVisibleFadeBand;
            if (a >= 0.0 && a < 1.0)
                factor = a;
        }
        if (maxVisibleFadeBand != 0.0)
        {
            float b = (maxVisible - frameInfo->heightAboveSurface)/maxVisibleFadeBand;
            if (b >= 0.0 && b < 1.0)
                factor = b;
        }
        
        fade = fade * factor;
    }
    
    // GL Texture IDs
    bool anyTextures = false;
    std::vector<GLuint> glTexIDs;
    for (unsigned int ii=0;ii<texInfo.size();ii++)
    {
        const TexInfo &thisTexInfo = texInfo[ii];
        GLuint glTexID = EmptyIdentity;
        if (thisTexInfo.texId != EmptyIdentity)
        {
            glTexID = scene->getGLTexture(thisTexInfo.texId);
            anyTextures = true;
        }
        glTexIDs.push_back(glTexID);
    }
    
    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo->mvpMat);
    prog->setUniform("u_mvMatrix", frameInfo->viewAndModelMat);
    prog->setUniform("u_mvNormalMatrix", frameInfo->viewModelNormalMat);
    prog->setUniform("u_mvpNormalMatrix", frameInfo->mvpNormalMat);
    prog->setUniform("u_pMatrix", frameInfo->projMat);
    
    // Fill the a_singleMatrix attribute with default values
    const OpenGLESAttribute *matAttr = prog->findAttribute("a_singleMatrix");
    if (matAttr)
    {
        glVertexAttrib4f(matAttr->index,1.0,0.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+1,0.0,1.0,0.0,0.0);
        glVertexAttrib4f(matAttr->index+2,0.0,0.0,1.0,0.0);
        glVertexAttrib4f(matAttr->index+3,0.0,0.0,0.0,1.0);
    }
    
    // Fade is always mixed in
    prog->setUniform("u_fade", fade);
    
    // Let the shaders know if we even have a texture
    prog->setUniform("u_hasTexture", anyTextures);
    
    // If this is present, the drawable wants to do something based where the viewer is looking
    prog->setUniform("u_eyeVec", frameInfo->fullEyeVec);
    
    // The program itself may have some textures to bind
    bool hasTexture[WhirlyKitMaxTextures];
    int progTexBound = prog->bindTextures();
    for (unsigned int ii=0;ii<progTexBound;ii++)
        hasTexture[ii] = true;
    
    // Zero or more textures in the drawable
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures-progTexBound;ii++)
    {
        GLuint glTexID = ii < glTexIDs.size() ? glTexIDs[ii] : 0;
        char baseMapName[40];
        sprintf(baseMapName,"s_baseMap%d",ii);
        const OpenGLESUniform *texUni = prog->findUniform(baseMapName);
        hasTexture[ii+progTexBound] = glTexID != 0 && texUni;
        if (hasTexture[ii+progTexBound])
        {
            frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii+progTexBound);
            glBindTexture(GL_TEXTURE_2D, glTexID);
            CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
            prog->setUniform(baseMapName, (int)ii+progTexBound);
            CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
        }
    }
    
    // If necessary, set up the VAO (once)
    bool boundElements = false;
    bool usedLocalVertices = false;
    const OpenGLESAttribute *vertAttr = NULL;
    const OpenGLESAttribute *progAttrs[vertexAttributes.size()];
    if (hasVertexArraySupport)
    {
        if (vertArrayObj == 0 && sharedBuffer != 0)
            vertArrayObj = setupVAO(prog);
        
        // Figure out what we're using
        vertAttr = prog->findAttribute("a_position");
        
        // Vertex array
        if (vertAttr && !(sharedBuffer || pointBuffer))
        {
            usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
            glEnableVertexAttribArray ( vertAttr->index );
            CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
        }
        
        // Other vertex attributes
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            VertexAttribute *attr = vertexAttributes[ii];
            const OpenGLESAttribute *progAttr = prog->findAttribute(attr->name);
            progAttrs[ii] = NULL;
            if (progAttr)
            {
                // The data hasn't been downloaded, so hook it up directly here
                if (attr->buffer == 0)
                {
                    // We have a data array for it, so hand that over
                    if (attr->numElements() != 0)
                    {
                        glVertexAttribPointer(progAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), 0, attr->addressForElement(0));
                        CheckGLError("BasicDrawable::drawVBO2() glVertexAttribPointer");
                        glEnableVertexAttribArray ( progAttr->index );
                        CheckGLError("BasicDrawable::drawVBO2() glEnableVertexAttribArray");
                        
                        progAttrs[ii] = progAttr;
                    } else {
                        // The program is expecting it, so we need a default
                        // Note: Could be doing this in the VAO
                        attr->glSetDefault(progAttr->index);
                        CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                    }
                }
            }
        }
    } else {
        // Note: Porting.  Move this into a shared function
        vertAttr = prog->findAttribute("a_position");
        
        // We're using a single buffer for all of our vertex attributes
        if (sharedBuffer)
        {
            glBindBuffer(GL_ARRAY_BUFFER,sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO2() shared glBindBuffer");
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(sharedBufferOffset,0));
        } else {
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
        }
        usedLocalVertices = true;
        glEnableVertexAttribArray ( vertAttr->index );
        
        // All the rest of the attributes
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
        {
            progAttrs[ii] = NULL;
            VertexAttribute *attr = vertexAttributes[ii];
            const OpenGLESAttribute *thisAttr = prog->findAttribute(attr->name);
            if (thisAttr)
            {
                if (attr->buffer != 0 || attr->numElements() != 0)
                {
                    if (attr->buffer)
                        glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), vertexSize, CALCBUFOFF(sharedBufferOffset,attr->buffer));
                    else
                        glVertexAttribPointer(thisAttr->index, attr->glEntryComponents(), attr->glType(), attr->glNormalize(), 0, attr->addressForElement(0));
                    glEnableVertexAttribArray(thisAttr->index);
                    progAttrs[ii] = thisAttr;
                } else {
                    // The program is expecting it, so we need a default
                    attr->glSetDefault(thisAttr->index);
                    CheckGLError("BasicDrawable::drawVBO2() glSetDefault");
                }
            }
        }
        
        // Bind the element array
        if (type == GL_TRIANGLES && sharedBuffer)
        {
            boundElements = true;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
        }
    }
    
    // Let a subclass bind anything additional
    bindAdditionalRenderObjects(frameInfo,scene);
    
    // If we're using a vertex array object, bind it and draw
    if (vertArrayObj)
    {
        glBindVertexArray(vertArrayObj);
        switch (type)
        {
            case GL_TRIANGLES:
                glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(sharedBufferOffset,triBuffer));
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                break;
            case GL_POINTS:
            case GL_LINES:
            case GL_LINE_STRIP:
            case GL_LINE_LOOP:
                frameInfo->stateOpt->setLineWidth(lineWidth);
                glDrawArrays(type, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
            case GL_TRIANGLE_STRIP:
                glDrawArrays(type, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
        }
        glBindVertexArray(0);
    } else {
        // Draw without a VAO
        switch (type)
        {
            case GL_TRIANGLES:
            {
                if (triBuffer)
                {
                    if (!boundElements)
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
                    CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
                    glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, (void *)((uintptr_t)triBuffer));
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                } else {
                    if (!boundElements)
                        glDrawElements(GL_TRIANGLES, (GLsizei)tris.size()*3, GL_UNSIGNED_SHORT, &tris[0]);
                    else
                        glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, 0);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                }
            }
                break;
            case GL_POINTS:
            case GL_LINES:
            case GL_LINE_STRIP:
            case GL_LINE_LOOP:
                frameInfo->stateOpt->setLineWidth(lineWidth);
                CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
                glDrawArrays(type, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
            case GL_TRIANGLE_STRIP:
                glDrawArrays(type, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
        }
    }
    
    // Unbind any texture
    for (unsigned int ii=0;ii<WhirlyKitMaxTextures;ii++)
        if (hasTexture[ii])
        {
            frameInfo->stateOpt->setActiveTexture(GL_TEXTURE0+ii);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    
    // Note: Porting.  Is this redundant?
    // Tear down the various arrays, if we stood them up
    if (usedLocalVertices)
        glDisableVertexAttribArray(vertAttr->index);
    //    for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
    //        if (progAttrs[ii])
    //            glDisableVertexAttribArray(progAttrs[ii]->index);
    
    if (!hasVertexArraySupport)
    {
        // Now tear down all that state
        if (vertAttr)
            glDisableVertexAttribArray(vertAttr->index);
        for (unsigned int ii=0;ii<vertexAttributes.size();ii++)
            if (progAttrs[ii])
                glDisableVertexAttribArray(progAttrs[ii]->index);
        if (boundElements)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        if (sharedBuffer)
            glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    // Let a subclass clean up any remaining state
    postDrawCallback(frameInfo,scene);
}

BasicDrawableTexTweaker::BasicDrawableTexTweaker(const std::vector<SimpleIdentity> &texIDs,TimeInterval startTime,double period)
: texIDs(texIDs), startTime(startTime), period(period)
{
}

void BasicDrawableTexTweaker::tweakForFrame(Drawable *draw,RendererFrameInfo *frame)
{
    BasicDrawable *basicDraw = (BasicDrawable *)draw;
    
    double t = fmod(frame->currentTime-startTime,period)/period;
    int base = floor(t * texIDs.size());
    int next = (base+1)%texIDs.size();
    
    basicDraw->setTexId(0, texIDs[base]);
    basicDraw->setTexId(1, texIDs[next]);
    
    // This forces a redraw every frame
    // Note: There has to be a better way
    frame->scene->addChangeRequest(NULL);
}

BasicDrawableScreenTexTweaker::BasicDrawableScreenTexTweaker(const Point3d &centerPt,const Point2d &texScale)
    : centerPt(centerPt), texScale(texScale)
{
}
    
void BasicDrawableScreenTexTweaker::tweakForFrame(Drawable *draw,WhirlyKit::RendererFrameInfo *frameInfo)
{
    if (frameInfo->program)
    {
        Vector4f screenPt = frameInfo->mvpMat * Vector4f(centerPt.x(),centerPt.y(),centerPt.z(),1.0);
        screenPt /= screenPt.w();
        
        Point2f u_scale = frameInfo->sceneRenderer->getFramebufferSize() / 2.f;
        Point2f newScreenPt(fmod(-screenPt.x()*texScale.x()*u_scale.x(),1.0),fmod(-screenPt.y()*texScale.y()*u_scale.y(),1.0));
        newScreenPt.x() /= texScale.x()*u_scale.x();
        newScreenPt.y() /= texScale.y()*u_scale.y();
        
        frameInfo->program->setUniform("u_screenOrigin", Point2f(newScreenPt.x(),newScreenPt.y()));
        frameInfo->program->setUniform("u_scale", u_scale);
        frameInfo->program->setUniform("u_texScale", Point2f(texScale.x(),texScale.y()));
    }
}

ColorChangeRequest::ColorChangeRequest(SimpleIdentity drawId,RGBAColor inColor)
: DrawableChangeRequest(drawId)
{
    color[0] = inColor.r;
    color[1] = inColor.g;
    color[2] = inColor.b;
    color[3] = inColor.a;
}

void ColorChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
    {
        basicDrawable->setColor(color);
    } else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setColor(RGBAColor(color[0],color[1],color[2],color[3]));
    }
}

OnOffChangeRequest::OnOffChangeRequest(SimpleIdentity drawId,bool OnOff)
: DrawableChangeRequest(drawId), newOnOff(OnOff)
{
    
}

void OnOffChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setOnOff(newOnOff);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setEnable(newOnOff);
    }
}

VisibilityChangeRequest::VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis)
: DrawableChangeRequest(drawId), minVis(minVis), maxVis(maxVis)
{
}

void VisibilityChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setVisibleRange(minVis,maxVis);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        basicDrawInst->setVisibleRange(minVis, maxVis);
    }
}

FadeChangeRequest::FadeChangeRequest(SimpleIdentity drawId,TimeInterval fadeUp,TimeInterval fadeDown)
: DrawableChangeRequest(drawId), fadeUp(fadeUp), fadeDown(fadeDown)
{
    
}

void FadeChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    // Fade it out, then remove it
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
    {
        basicDrawable->setFade(fadeDown, fadeUp);
    }
    
    // And let the renderer know
    renderer->setRenderUntil(fadeDown);
    renderer->setRenderUntil(fadeUp);
}

DrawTexChangeRequest::DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId)
: DrawableChangeRequest(drawId), which(which), newTexId(newTexId)
{
}

void DrawTexChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setTexId(which,newTexId);
}

DrawTexturesChangeRequest::DrawTexturesChangeRequest(SimpleIdentity drawId,const std::vector<SimpleIdentity> &newTexIDs)
: DrawableChangeRequest(drawId), newTexIDs(newTexIDs)
{
}

void DrawTexturesChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setTexIDs(newTexIDs);
}

TransformChangeRequest::TransformChangeRequest(SimpleIdentity drawId,const Matrix4d *newMat)
: DrawableChangeRequest(drawId), newMat(*newMat)
{
}

void TransformChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDraw = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDraw.get())
        basicDraw->setMatrix(&newMat);
}

DrawPriorityChangeRequest::DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority)
: DrawableChangeRequest(drawId), drawPriority(drawPriority)
{
}

void DrawPriorityChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setDrawPriority(drawPriority);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setDrawPriority(drawPriority);
    }
}

LineWidthChangeRequest::LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth)
: DrawableChangeRequest(drawId), lineWidth(lineWidth)
{
}

void LineWidthChangeRequest::execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = std::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDrawable)
        basicDrawable->setLineWidth(lineWidth);
    else {
        BasicDrawableInstanceRef basicDrawInst = std::dynamic_pointer_cast<BasicDrawableInstance>(draw);
        if (basicDrawInst)
            basicDrawInst->setLineWidth(lineWidth);
    }
}
    
}
