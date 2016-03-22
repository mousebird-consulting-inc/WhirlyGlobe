/*
 *  BaseInfo.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/6/15.
 *  Copyright 2011-2016 mousebird consulting
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

#import "BaseInfo.h"
#import "Drawable.h"
#import "BasicDrawable.h"
#import "BasicDrawableInstance.h"

using namespace WhirlyKit;
using namespace Eigen;

namespace WhirlyKit
{
BaseInfo::BaseInfo()
    : minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid), minVisBand(DrawVisibleInvalid), maxVisBand(DrawVisibleInvalid),
    minViewerDist(DrawVisibleInvalid), maxViewerDist(DrawVisibleInvalid), viewerCenter(DrawVisibleInvalid,DrawVisibleInvalid,DrawVisibleInvalid), fade(0.0), fadeIn(0.0), fadeOut(0.0), fadeOutTime(0.0), drawPriority(0), drawOffset(0.0),
    enable(true), startEnable(0.0), endEnable(0.0), programID(EmptyIdentity)
{
}

BaseInfo::BaseInfo(const Dictionary &dict)
{
    minVis = dict.getDouble("minVis",DrawVisibleInvalid);
    maxVis = dict.getDouble("maxVis",DrawVisibleInvalid);
    minVisBand = dict.getDouble("minVisBand",DrawVisibleInvalid);
    maxVisBand = dict.getDouble("maxVisBand",DrawVisibleInvalid);
    minViewerDist = dict.getDouble("minviewerdist",DrawVisibleInvalid);
    maxViewerDist = dict.getDouble("maxviewerdist",DrawVisibleInvalid);
    viewerCenter.x() = dict.getDouble("viewablecenterx",DrawVisibleInvalid);
    viewerCenter.y() = dict.getDouble("viewablecentery",DrawVisibleInvalid);
    viewerCenter.z() = dict.getDouble("viewablecenterz",DrawVisibleInvalid);
    fade = dict.getDouble("fade",0.0);
    fadeIn = fade;
    fadeOut = fade;
    fadeIn = dict.getDouble("fadein",fadeIn);
    fadeOut = dict.getDouble("fadeout",fadeOut);
    fadeOutTime = dict.getDouble("fadeouttime",0.0);
    drawPriority = dict.getInt("priority",0);
    drawPriority = dict.getInt("drawPriority",drawPriority);
    drawOffset = dict.getDouble("drawOffset",0.0);
    enable = dict.getBool("enable",true);
    startEnable = dict.getDouble("enablestart",0.0);
    endEnable = dict.getDouble("enableend",0.0);
    SimpleIdentity shaderID = dict.getInt("shader",EmptyIdentity);
    programID = dict.getInt("program",shaderID);
}

void BaseInfo::setupBasicDrawable(BasicDrawable *drawable) const
{
    drawable->setOnOff(enable);
    drawable->setEnableTimeRange(startEnable, endEnable);
    drawable->setDrawPriority(drawPriority);
    drawable->setVisibleRange(minVis,maxVis);
    drawable->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawable->setProgram(programID);
}

//void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstance *drawable)
//{
//    drawInst->setEnable(_enable);
//    drawInst->setEnableTimeRange(_startEnable, _endEnable);
//    drawInst->setDrawPriority(_drawPriority);
//    drawInst->setVisibleRange(_minVis,_maxVis);
//    drawInst->setViewerVisibility(_minViewerDist,_maxViewerDist,_viewerCenter);
//}
    
}
