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
: Drawable(name)
{
}
    
BasicDrawableInstance::~BasicDrawableInstance()
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
    programID = progID;
}

bool BasicDrawableInstance::isOn(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    if (startEnable != endEnable)
    {
        if (frameInfo->currentTime < startEnable ||
            endEnable < frameInfo->currentTime)
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
    
    return true;
}

bool BasicDrawableInstance::hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    return basicDraw->hasAlpha(frameInfo);
}
    
void BasicDrawableInstance::setRequestZBuffer(bool val)
{
    requestZBuffer = val;
}
    
bool BasicDrawableInstance::getRequestZBuffer() const
{
    return requestZBuffer;
}
    
void BasicDrawableInstance::setWriteZBuffer(bool val)
{
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
    return EmptyIdentity;
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

/// Set the color
void BasicDrawableInstance::setColor(RGBAColor inColor)
{
    hasColor = true; color = inColor;
}

/// Set the draw priority
void BasicDrawableInstance::setDrawPriority(int newPriority)
{
    hasDrawPriority = true;  drawPriority = newPriority;
}

/// Set the line width
void BasicDrawableInstance::setLineWidth(int newLineWidth)
{
    hasLineWidth = true;  lineWidth = newLineWidth;
}
    
void BasicDrawableInstance::setStartTime(TimeInterval inStartTime)
{
    startTime = inStartTime;
}

void BasicDrawableInstance::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    uniforms = newUniforms;
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
    renderTargetID = newRenderTarget;
}

void BasicDrawableInstance::setTexId(unsigned int which,SimpleIdentity inId)
{
    if (which < texInfo.size())
        texInfo[which].texId = inId;
    else {
        wkLogLevel(Error, "BasicDrawableInstance:setTexId() Tried to set texInfo entry that doesn't exist.");
    }
}

void BasicDrawableInstance::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<std::min(texInfo.size(),texIDs.size());ii++)
    {
        texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawableInstance::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    if (which >= texInfo.size())
        return;
    
    TexInfo &ti = texInfo[which];
    ti.size = size;
    ti.borderTexel = borderTexel;
    ti.relLevel = relLevel;
    ti.relX = relX;
    ti.relY = relY;
}    
    
}
