/*
 *  BasicDrawableInstance.mm
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

#import "Program.h"
#import "BasicDrawable.h"
#import "SceneRenderer.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableInstance::BasicDrawableInstance(const std::string &name)
: Drawable(name), numInstances(0), instanceTexSource(EmptyIdentity), instanceTexProg(EmptyIdentity), valuesChanged(true), texturesChanged(true)
{
}
    
BasicDrawableInstance::~BasicDrawableInstance()
{
}

BasicDrawableInstance::TexInfo::TexInfo(BasicDrawable::TexInfo &basicTexInfo)
: texId(basicTexInfo.texId), size(basicTexInfo.size), borderTexel(basicTexInfo.borderTexel),
relLevel(basicTexInfo.relLevel), relX(basicTexInfo.relX), relY(basicTexInfo.relY)
{
}

BasicDrawableRef BasicDrawableInstance::getMaster() const
{
    return basicDraw;
}

void BasicDrawableInstance::setMaster(BasicDrawableRef newMaster)
{
    basicDraw = newMaster;
}
    
Mbr BasicDrawableInstance::getLocalMbr() const
{
    return basicDraw->getLocalMbr();
}

int64_t BasicDrawableInstance::getDrawOrder() const
{
    return hasDrawOrder ? drawOrder : basicDraw->getDrawOrder();
}

unsigned int BasicDrawableInstance::getDrawPriority() const
{
    if (hasDrawPriority)
        return drawPriority;
    return basicDraw->getDrawPriority();
}
    
SimpleIdentity BasicDrawableInstance::getMasterID() const
{
    return masterID;
}

SimpleIdentity BasicDrawableInstance::getProgram() const
{
    if (programID != EmptyIdentity)
        return programID;

    return basicDraw->getProgram();
}
    
void BasicDrawableInstance::setProgram(SimpleIdentity progID)
{
    if (programID == progID)
        return;
    
    setValuesChanged();
    
    programID = progID;
}

bool BasicDrawableInstance::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable)
            return false;
        if (endEnable != 0.0 && endEnable < frameInfo->currentTime)
            return false;
    }

    if (!enable)
        return false;
    
    double visVal = frameInfo->theView->heightAboveSurface();
    
    // Height based check
    if (minVis != DrawVisibleInvalid && maxVis != DrawVisibleInvalid)
    {
        if (!((minVis <= visVal && visVal <= maxVis) ||
              (maxVis <= visVal && visVal <= minVis)))
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
    
    // Zoom based check.  We need to be in the current zoom range
    if (zoomSlot > -1 && zoomSlot <= MaplyMaxZoomSlots) {
        float zoom = frameInfo->scene->getZoomSlotValue(zoomSlot);
        if (zoom != MAXFLOAT) {
            if (minZoomVis != DrawVisibleInvalid && zoom < minZoomVis)
                return false;
            if (maxZoomVis != DrawVisibleInvalid && zoom >= maxZoomVis)
                return false;
        }
    }
    
    return true;
}
    
void BasicDrawableInstance::setRequestZBuffer(bool val)
{
    if (requestZBuffer == val)
        return;
    
    setValuesChanged();

    requestZBuffer = val;
}
    
bool BasicDrawableInstance::getRequestZBuffer() const
{
    return requestZBuffer;
}
    
void BasicDrawableInstance::setWriteZBuffer(bool val)
{
    if (writeZBuffer == val)
        return;
    
    setValuesChanged();

    writeZBuffer = val;
}
    
bool BasicDrawableInstance::getWriteZbuffer() const
{
    return writeZBuffer;
}

void BasicDrawableInstance::updateRenderer(WhirlyKit::SceneRenderer *renderer)
{
    if (moving)
    {
        // Motion requires continuous rendering
        renderer->addContinuousRenderRequest(getId());
    }
    
    return basicDraw->updateRenderer(renderer);
}
    
SimpleIdentity BasicDrawableInstance::getCalculationProgram() const
{
    return instanceTexProg;
}
    
void BasicDrawableInstance::setEnable(bool newEnable)
{
    enable = newEnable;
}

void BasicDrawableInstance::setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable)
{
    startEnable = inStartEnable;  endEnable = inEndEnable;
}

void BasicDrawableInstance::setVisibleRange(float inMinVis,float inMaxVis)
{
    minVis = inMinVis;   maxVis = inMaxVis;
}

/// Set the viewer based visibility
void BasicDrawableInstance::setViewerVisibility(double inMinViewerDist,double inMaxViewerDist,const Point3d &inViewerCenter)
{
    minViewerDist = inMinViewerDist; maxViewerDist = inMaxViewerDist; viewerCenter = inViewerCenter;
}

void BasicDrawableInstance::setZoomInfo(int inZoomSlot,double inMinZoomVis,double inMaxZoomVis)
{
    if (zoomSlot == inZoomSlot && minZoomVis == inMinZoomVis && maxZoomVis == inMaxZoomVis)
        return;
    
    setValuesChanged();
    
    zoomSlot = inZoomSlot;
    minZoomVis = inMinZoomVis;
    maxZoomVis = inMaxZoomVis;
}

/// Set the color
void BasicDrawableInstance::setColor(RGBAColor inColor)
{
    if (hasColor && color == inColor)
        return;
    
    setValuesChanged();

    hasColor = true; color = inColor;
}

/// Set the draw order
void BasicDrawableInstance::setDrawOrder(int64_t newOrder)
{
    if (hasDrawOrder && drawOrder == newOrder)
        return;
    
    setValuesChanged();

    hasDrawOrder = true;
    drawOrder = newOrder;
}

/// Set the draw priority
void BasicDrawableInstance::setDrawPriority(int newPriority)
{
    if (hasDrawPriority && drawPriority == newPriority)
        return;
    
    setValuesChanged();

    hasDrawPriority = true;  drawPriority = newPriority;
}

/// Set the line width
void BasicDrawableInstance::setLineWidth(float newLineWidth)
{
    if (hasLineWidth && lineWidth == newLineWidth)
        return;
    
    setValuesChanged();

    hasLineWidth = true;
    lineWidth = newLineWidth;
}
    
void BasicDrawableInstance::setStartTime(TimeInterval inStartTime)
{
    startTime = inStartTime;
}

void BasicDrawableInstance::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    uniforms = newUniforms;
    
    setValuesChanged();
}
    
void BasicDrawableInstance::setUniBlock(const BasicDrawable::UniformBlock &uniBlock)
{
    for (int ii=0;ii<uniBlocks.size();ii++)
        if (uniBlocks[ii].bufferID == uniBlock.bufferID) {
            uniBlocks[ii] = uniBlock;
            return;
        }
    
    uniBlocks.push_back(uniBlock);
}

const Eigen::Matrix4d *BasicDrawableInstance::getMatrix() const
{
    return basicDraw->getMatrix();
}
    
SimpleIdentity BasicDrawableInstance::getRenderTarget() const
{
    return renderTargetID;
}

void BasicDrawableInstance::setRenderTarget(SimpleIdentity newRenderTarget)
{
    if (renderTargetID == newRenderTarget)
        return;
    
    setValuesChanged();

    renderTargetID = newRenderTarget;
}

void BasicDrawableInstance::setTexId(unsigned int which,SimpleIdentity inId)
{
    bool changes = false;
    
    if (which < texInfo.size()) {
        texInfo[which].texId = inId;
        changes = true;
    } else {
        wkLogLevel(Error, "BasicDrawableInstance:setTexId() Tried to set texInfo entry that doesn't exist.");
    }
    
    if (changes)
        setTexturesChanged();
}

void BasicDrawableInstance::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    bool changes = false;
    
    for (unsigned int ii=0;ii<std::min(texInfo.size(),texIDs.size());ii++)
    {
        if (texInfo[ii].texId != texIDs[ii]) {
            texInfo[ii].texId = texIDs[ii];
            changes = true;
        }
    }

    if (changes)
        setTexturesChanged();
}

void BasicDrawableInstance::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    bool changes = false;
    
    if (which >= texInfo.size())
       return;

    TexInfo &ti = texInfo[which];
    if (ti.size != size || ti.borderTexel != borderTexel || ti.relLevel != relLevel || ti.relX != relX || ti.relY != relY) {
        ti.size = size;
        ti.borderTexel = borderTexel;
        ti.relLevel = relLevel;
        ti.relX = relX;
        ti.relY = relY;
        changes = true;
    }

    if (changes)
        setTexturesChanged();
}

void BasicDrawableInstance::setValuesChanged()
{
    valuesChanged = true;
    if (renderTargetCon)
        renderTargetCon->modified = true;
}
    
void BasicDrawableInstance::setTexturesChanged()
{
    texturesChanged = true;
    if (renderTargetCon)
        renderTargetCon->modified = true;
}

void BasicDrawableInstance::setInstanceData(int numInstances,RawDataRef data)
{
    this->numInstances = numInstances;
    this->instData = data;
}

}
