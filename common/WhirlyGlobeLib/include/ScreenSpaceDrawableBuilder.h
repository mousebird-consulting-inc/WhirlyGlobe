/*
 *  ScreenSpaceDrawable.h
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

#import "BasicDrawableBuilder.h"
#import "SceneRenderer.h"
#import "BaseInfo.h"

namespace WhirlyKit
{
    
// Modifies the uniform values of a given shader right before the
//  screenspace's Basic Drawables are rendered
class ScreenSpaceTweaker : public DrawableTweaker
{
public:
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) = 0;
    
    TimeInterval startTime;
    bool keepUpright;
    bool activeRot;
    bool motion;
};

/// Wrapper for building screen space drawables
class ScreenSpaceDrawableBuilder : virtual public BasicDrawableBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    // Construct with or without motion support
    ScreenSpaceDrawableBuilder();
    virtual void Init(bool hasMotion,bool hasRotation, bool buildAnyway = false);
    
    // If we've got a rotation, we set this to keep the image facing upright
    //  probably because it's text.
    void setKeepUpright(bool keepUpright);
    // Time we start counting from for motion
    void setStartTime(TimeInterval inStartTime);
    // Time we start counting from for motion
    TimeInterval getStartTime();
    
    // Each vertex has an offset on the screen
    void addOffset(const Point2f &offset);
    void addOffset(const Point2d &offset);

    // Add a direction to the attribute list (for animation)
    void addDir(const Point3f &dir);
    void addDir(const Point3d &dir);
    
    // Add a rotation vector to the attribute list
    void addRot(const Point3f &dir);
    void addRot(const Point3d &dir);
    
    // Apply a scale expression
    void setScaleExpression(FloatExpressionInfoRef scale);
        
    // Tweaker runs before we draw and we need different versions for the renderers
    virtual ScreenSpaceTweaker *makeTweaker() = 0;
    
    void setupTweaker(BasicDrawable *theDraw);
    
protected:
    bool motion,rotation;
    bool keepUpright;
    int offsetIndex;
    int dirIndex;
    int rotIndex;
    TimeInterval startTime;
    FloatExpressionInfoRef scaleExp;
    FloatExpressionInfoRef opacityExp;
    ColorExpressionInfoRef colorExp;
};
    
}
