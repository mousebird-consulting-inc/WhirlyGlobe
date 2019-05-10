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

#import "GLUtils.h"
#import "BasicDrawableInstance.h"
#import "SceneRenderer.h"
#import "TextureAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{

BasicDrawableInstance::BasicDrawableInstance(const std::string &name,SimpleIdentity masterID,Style style)
: Drawable(name)
{
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

SimpleIdentity BasicDrawableInstance::getProgram() const
{
    if (programID != EmptyIdentity)
        return programID;

    return basicDraw->getProgram();
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

GLenum BasicDrawableInstance::getType() const
{
    return basicDraw->getType();
}

bool BasicDrawableInstance::hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const
{
    return basicDraw->hasAlpha(frameInfo);
}

void BasicDrawableInstance::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    if (moving)
    {
        // Motion requires continuous rendering
        renderer->addContinuousRenderRequest(getId());
    }
    
    return basicDraw->updateRenderer(renderer);
}

const Eigen::Matrix4d *BasicDrawableInstance::getMatrix() const
{
    return basicDraw->getMatrix();
}

void BasicDrawableInstance::setUniforms(const SingleVertexAttributeSet &newUniforms)
{
    uniforms = newUniforms;
}

void BasicDrawableInstance::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    texInfo[which].texId = inId;
}

void BasicDrawableInstance::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<texIDs.size();ii++)
    {
        setupTexCoordEntry(ii, 0);
        texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawableInstance::setupTexCoordEntry(int which,int numReserve)
{
    if (which < texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)texInfo.size();ii<=which;ii++)
    {
        TexInfo newInfo;
        texInfo.push_back(newInfo);
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
