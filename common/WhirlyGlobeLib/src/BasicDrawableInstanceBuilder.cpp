/*
 *  BasicDrawableInstanceBuilder.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
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

#import "BasicDrawableInstanceBuilder.h"

namespace WhirlyKit
{

BasicDrawableInstanceBuilder::BasicDrawableInstanceBuilder(const std::string &name)
{
}
    
BasicDrawableInstanceBuilder::~BasicDrawableInstanceBuilder()
{
}
    
void BasicDrawableInstanceBuilder::Init()
{
    drawInst->programID = EmptyIdentity;
    drawInst->enable = true;
    drawInst->masterID = EmptyIdentity;
    drawInst->requestZBuffer = false;
    drawInst->writeZBuffer = true;
    drawInst->startEnable = 0.0;
    drawInst->endEnable = 0.0;
    drawInst->numInstances = 0;
    drawInst->minVis = DrawVisibleInvalid;
    drawInst->maxVis = DrawVisibleInvalid;
    drawInst->minViewerDist = DrawVisibleInvalid;
    drawInst->maxViewerDist = DrawVisibleInvalid;
    drawInst->viewerCenter = Point3d(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid);
    drawInst->startTime = 0;
    drawInst->moving = false;
    drawInst->instanceStyle = BasicDrawableInstance::LocalStyle;
    drawInst->renderTargetID = EmptyIdentity;
    drawInst->hasColor = false;
    drawInst->hasDrawPriority = false;
    drawInst->hasLineWidth = false;
    drawInst->instanceTexSource = EmptyIdentity;
    drawInst->instanceTexProg = EmptyIdentity;
}

void BasicDrawableInstanceBuilder::setMasterID(SimpleIdentity baseDrawID,BasicDrawableInstance::Style style)
{
    drawInst->masterID = baseDrawID;
    drawInst->instanceStyle = style;
}

void BasicDrawableInstanceBuilder::setOnOff(bool onOff)
{
    drawInst->enable = onOff;
}

void BasicDrawableInstanceBuilder::setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable)
{
    drawInst->startEnable = inStartEnable;
    drawInst->endEnable = inEndEnable;
}

void BasicDrawableInstanceBuilder::setViewerVisibility(double minViewerDist,double maxViewerDist,const Point3d &viewerCenter)
{
    drawInst->minViewerDist = minViewerDist;
    drawInst->maxViewerDist = maxViewerDist;
    drawInst->viewerCenter = viewerCenter;
}
void BasicDrawableInstanceBuilder::setVisibleRange(float minVis,float maxVis)
{
    drawInst->minVis = minVis;  drawInst->maxVis = maxVis;
}

void BasicDrawableInstanceBuilder::setDrawPriority(unsigned int newPriority)
{
    drawInst->hasDrawPriority = true;
    drawInst->drawPriority = newPriority;
}
    
void BasicDrawableInstanceBuilder::setColor(const RGBAColor &color)
{
    drawInst->hasColor = true;
    drawInst->setColor(color);
}

void BasicDrawableInstanceBuilder::setLineWidth(float lineWidth)
{
    drawInst->hasLineWidth = lineWidth;
    drawInst->setLineWidth(lineWidth);
}

void BasicDrawableInstanceBuilder::setRequestZBuffer(bool val)
{
    drawInst->requestZBuffer = val;
}

void BasicDrawableInstanceBuilder::setWriteZBuffer(bool val)
{
    drawInst->writeZBuffer = val;
}

void BasicDrawableInstanceBuilder::setRenderTarget(SimpleIdentity newRenderTarget)
{
    drawInst->renderTargetID = newRenderTarget;
}

void BasicDrawableInstanceBuilder::addTweaker(DrawableTweakerRef tweakRef)
{
    drawInst->tweakers.insert(tweakRef);
}

void BasicDrawableInstanceBuilder::setIsMoving(bool isMoving)
{
    drawInst->moving = isMoving;
}

void BasicDrawableInstanceBuilder::setStartTime(TimeInterval inStartTime)
{
    drawInst->startTime = inStartTime;
}

void BasicDrawableInstanceBuilder::addInstances(const std::vector<BasicDrawableInstance::SingleInstance> &insts)
{
    drawInst->instances.insert(drawInst->instances.end(), insts.begin(), insts.end());
}

void BasicDrawableInstanceBuilder::setInstanceTexSource(SimpleIdentity texID,SimpleIdentity srcProgID)
{
    drawInst->instanceTexSource = texID;
    drawInst->instanceTexProg = srcProgID;
}

void BasicDrawableInstanceBuilder::setUniforms(const SingleVertexAttributeSet &uniforms)
{
    drawInst->uniforms = uniforms;
}
    
void BasicDrawableInstanceBuilder::setUniBlock(const BasicDrawable::UniformBlock &uniBlock)
{
    drawInst->setUniBlock(uniBlock);
}

void BasicDrawableInstanceBuilder::setTexId(unsigned int which,SimpleIdentity inId)
{
    setupTexCoordEntry(which, 0);
    
    if (drawInst->texInfo.empty())
        return;
    
    drawInst->texInfo[which].texId = inId;
}

void BasicDrawableInstanceBuilder::setTexIDs(const std::vector<SimpleIdentity> &texIDs)
{
    for (unsigned int ii=0;ii<texIDs.size();ii++)
    {
        setupTexCoordEntry(ii, 0);
        drawInst->texInfo[ii].texId = texIDs[ii];
    }
}

void BasicDrawableInstanceBuilder::setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY)
{
    drawInst->setTexRelative(which, size, borderTexel, relLevel, relX, relY);
}

void BasicDrawableInstanceBuilder::setupTexCoordEntry(int which,int numReserve)
{
    if (which < drawInst->texInfo.size())
        return;
    
    for (unsigned int ii=(unsigned int)drawInst->texInfo.size();ii<=which;ii++)
    {
        BasicDrawableInstance::TexInfo newInfo;
        drawInst->texInfo.push_back(newInfo);
    }
}

void BasicDrawableInstanceBuilder::setProgram(SimpleIdentity progID)
{
    drawInst->setProgram(progID);
}
    
SimpleIdentity BasicDrawableInstanceBuilder::getDrawableID()
{
    if (drawInst)
        return drawInst->getId();
    return EmptyIdentity;
}

}
