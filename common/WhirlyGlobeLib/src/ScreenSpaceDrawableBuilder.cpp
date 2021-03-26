/*
 *  ScreenSpaceDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/24/14.
 *  Copyright 2011-2019 mousebird consulting.
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

#import "ScreenSpaceDrawableBuilder.h"
#import "Program.h"

namespace WhirlyKit
{
    
ScreenSpaceDrawableBuilder::ScreenSpaceDrawableBuilder()
: keepUpright(false), motion(false), rotation(false), offsetIndex(-1), rotIndex(-1), dirIndex(-1), startTime(0.0)
{
}

void ScreenSpaceDrawableBuilder::ScreenSpaceInit(bool hasMotion,bool hasRotation,bool buildAnyway)
{
    rotation = hasRotation;
    motion = hasMotion;
    BasicDrawableBuilder::Init();
    setupStandardAttributes();

    offsetIndex = addAttribute(BDFloat2Type, a_offsetNameID);
    if (hasRotation || buildAnyway)
        rotIndex = addAttribute(BDFloat3Type, a_rotNameID);
    if (hasMotion || buildAnyway)
        dirIndex = addAttribute(BDFloat3Type, a_dirNameID);
}
    
void ScreenSpaceDrawableBuilder::setKeepUpright(bool newVal)
{
    keepUpright = newVal;
}
    
void ScreenSpaceDrawableBuilder::setStartTime(TimeInterval inStartTime)
    { startTime = inStartTime; }

TimeInterval ScreenSpaceDrawableBuilder::getStartTime()
    { return startTime; }

void ScreenSpaceDrawableBuilder::addOffset(const Point2f &offset)
{
    addAttributeValue(offsetIndex, offset);
}

void ScreenSpaceDrawableBuilder::addOffset(const Point2d &offset)
{
    addAttributeValue(offsetIndex, Point2f(offset.x(),offset.y()));
}
    
void ScreenSpaceDrawableBuilder::addDir(const Point3d &dir)
{
    addAttributeValue(dirIndex, Point3f(dir.x(),dir.y(),dir.z()));
}

void ScreenSpaceDrawableBuilder::addDir(const Point3f &dir)
{
    addAttributeValue(dirIndex, dir);
}
    
void ScreenSpaceDrawableBuilder::addRot(const Point3d &rotDir)
{
    addRot(Point3f(rotDir.x(),rotDir.y(),rotDir.z()));
}

void ScreenSpaceDrawableBuilder::addRot(const Point3f &rotDir)
{
    addAttributeValue(rotIndex, rotDir);
}

void ScreenSpaceDrawableBuilder::setScaleExpression(FloatExpressionInfoRef inScaleExp)
{
    scaleExp = inScaleExp;
}
    
void ScreenSpaceDrawableBuilder::setupTweaker(const DrawableTweakerRef &inTweaker) const
{
    if (auto tweak = std::dynamic_pointer_cast<ScreenSpaceTweaker>(inTweaker))
    {
        tweak->startTime = startTime;
        tweak->keepUpright = keepUpright;
        tweak->activeRot = rotation;
        tweak->motion = motion;
    }
}

}
