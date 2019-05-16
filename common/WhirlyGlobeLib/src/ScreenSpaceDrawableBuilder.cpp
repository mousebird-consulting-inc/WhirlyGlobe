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
#import "ProgramGLES.h"

namespace WhirlyKit
{
    
// Modifies the uniform values of a given shader right before the
//  screenspace's Basic Drawables are rendered
class ScreenSpaceTweaker : public DrawableTweaker
{
public:
    void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
    {
        BasicDrawable *draw = (BasicDrawable *)inDraw;
        if (frameInfo->program)
        {
            Point2f fbSize = frameInfo->sceneRenderer->getFramebufferSize();
            frameInfo->program->setUniform(u_ScaleNameID, Point2f(2.f/fbSize.x(),2.f/(float)fbSize.y()));
            frameInfo->program->setUniform(u_uprightNameID, keepUpright);
            if (draw->hasMotion())
                frameInfo->program->setUniform(u_TimeNameID, (float)(frameInfo->currentTime - startTime));
            frameInfo->program->setUniform(u_activerotNameID, (rotIndex >= 0 ? 1 : 0));
        }
    }
    
    TimeInterval startTime;
    bool keepUpright;
    int rotIndex;
};

ScreenSpaceDrawableBuilder::ScreenSpaceDrawableBuilder()
: keepUpright(false), motion(false), rotation(false), offsetIndex(-1), rotIndex(-1), dirIndex(-1)
{
}

void ScreenSpaceDrawableBuilder::Init(bool hasMotion,bool hasRotation)
{
    BasicDrawableBuilder::Init();
    setupStandardAttributes();

    offsetIndex = addAttribute(BDFloat2Type, a_offsetNameID);
    if (hasRotation)
        rotIndex = addAttribute(BDFloat3Type, a_rotNameID);
    if (hasMotion)
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
    
void ScreenSpaceDrawableBuilder::setupTweaker(BasicDrawable *theDraw)
{
    ScreenSpaceTweaker *tweak = new ScreenSpaceTweaker();
    tweak->startTime = startTime;
    tweak->keepUpright = keepUpright;
    tweak->rotIndex = rotIndex;
    theDraw->addTweaker(DrawableTweakerRef(tweak));
}
    
}
