/*
 *  BillboardDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/13.
 *  Copyright 2011-2013 mousebird consulting
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
    
// Shader name for accessing within the scene
#define kBillboardShaderName "Billboard Shader"
    
/// Construct and return the Billboard shader program
OpenGLES2Program *BuildBillboardProgram();

/** The drawable class for rendering billboards.
    Billboards contain extra information per vertex and
    have a custom vertex shader.
  */
class BillboardDrawable : public BasicDrawable
{
public:
    BillboardDrawable();
    
    /// Each vertex has an offset in 3-space
    void addOffset(Point3f offset);
    
protected:
    int offsetIndex;
};

}
