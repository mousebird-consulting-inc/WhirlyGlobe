/*
 *  WideVectorDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
 *  Copyright 2011-2014 mousebird consulting
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
#define kWideVectorShaderName "Wide Vector Shader"
    
/// Construct and return the Billboard shader program
OpenGLES2Program *BuildWideVectorProgram();

/** This drawable adds convenience functions for
  */
class WideVectorDrawable : public BasicDrawable
{
public:
    WideVectorDrawable();
    
    /// Each vertex has an offset in 3-space
    void addDir(const Point3f &dir);
    void addDir(const Point3d &dir);
    
    /// Set the width we'll we'll use
    void setWidth(float inWidth) { width = inWidth; }
    void setTexRepeat(float inTexRepeat) { texRepeat = inTexRepeat; }
    
    /// We override draw so we can set our own values
    virtual void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    
protected:
    float width,texRepeat;
    int offsetIndex;
};
    
}
