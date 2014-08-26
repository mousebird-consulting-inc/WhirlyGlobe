/*
 *  ScreenSpaceDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/24/14.
 *  Copyright 2011-2014 mousebird consulting. All rights reserved.
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

#import "Drawable.h"

namespace WhirlyKit
{
    
// Shader name
#define kScreenSpaceShaderName "Screen Space Shader"
    
/// Construct and return the Screen Space shader program
OpenGLES2Program *BuildScreenSpaceProgram();

/// Wrapper for building screen space drawables
class ScreenSpaceDrawable : public BasicDrawable
{
public:
    ScreenSpaceDrawable();
    
    // Set whether or not we use the rotation, rather than keeping things horizontal
    void setUseRotation(bool useRotation);
    // If we've got a rotation, we set this to keep the image facing upright
    //  probably because it's text.
    void setKeepUpright(bool keepUpright);
    
    // Each vertex has an offset on the screen
    void addOffset(const Point2f &offset);
    void addOffset(const Point2d &offset);
    
    /// We override draw so we can set our own values
    virtual void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);

protected:
    bool useRotation;
    bool keepUpright;
    int offsetIndex;
};
    
}
