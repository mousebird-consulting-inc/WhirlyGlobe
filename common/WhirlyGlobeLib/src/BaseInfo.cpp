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
    
// Really Android?  Really?
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

std::string BaseInfo::toString()
{
    std::string outStr =
    (std::string)"minVis = " + to_string(minVis) + ";" +
    " maxVis = " + to_string(maxVis) + ";" +
    " minVisBand = " + to_string(minVisBand) + ";" +
    " maxVisBand = " + to_string(maxVisBand) + ";" +
    " minViewerDist = " + to_string(minViewerDist) + ";" +
    " maxViewerDist = " + to_string(maxViewerDist) + ";" +
    " viewerCenter = (" + to_string(viewerCenter.x()) + "," + to_string(viewerCenter.y()) + "," + to_string(viewerCenter.z()) + ");" +
    " drawOffset = " + to_string(drawOffset) + ";" +
    " drawPriority = " + to_string(drawPriority) + ";" +
    " enable = " + (enable ? "yes" : "no") + ";" +
    " fade = " + to_string(fade) + ";" +
    " fadeIn = " + to_string(fadeIn) + ";" +
    " fadeOut = " + to_string(fadeOut) + ";" +
    " fadeOutTime = " + to_string(fadeOutTime) + ";" +
    " startEnable = " + to_string(startEnable) + ";" +
    " endEnable = " + to_string(endEnable) + ";" +
    " programID = " + to_string(programID) + ";";
    
    return outStr;
}

void BaseInfo::setupBasicDrawable(BasicDrawable *drawable) const
{
    drawable->setOnOff(enable);
    drawable->setEnableTimeRange(startEnable, endEnable);
    drawable->setDrawPriority(drawPriority);
    drawable->setVisibleRange(minVis,maxVis);
    drawable->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawable->setProgram(programID);
    drawable->setUniforms(uniforms);
}

void BaseInfo::setupBasicDrawableInstance(BasicDrawableInstance *drawInst)
{
    drawInst->setEnable(enable);
    drawInst->setEnableTimeRange(startEnable, endEnable);
    drawInst->setDrawPriority(drawPriority);
    drawInst->setVisibleRange(minVis,maxVis);
    drawInst->setViewerVisibility(minViewerDist,maxViewerDist,viewerCenter);
    drawInst->setUniforms(uniforms);
}
    
}
