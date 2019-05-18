/*
 *  ScreenSpaceDrawableBuilderMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "ScreenSpaceDrawableBuilder.h"
#import "BasicDrawableBuilderMTL.h"

namespace WhirlyKit
{
    
// Shader name
//#define kScreenSpaceShaderName "Screen Space Shader"
//#define kScreenSpaceShader2DName "Screen Space Shader 2D"
//#define kScreenSpaceShaderMotionName "Screen Space Shader Motion"
//#define kScreenSpaceShader2DMotionName "Screen Space Shader 2D Motion"

/// Construct and return the Screen Space shader program
Program *BuildScreenSpaceProgramMTL(const std::string &name,SceneRenderer *render);
Program *BuildScreenSpaceMotionProgramMTL(const std::string &name,SceneRenderer *render);
Program *BuildScreenSpace2DProgramMTL(const std::string &name,SceneRenderer *render);
Program *BuildScreenSpaceMotion2DProgramMTL(const std::string &name,SceneRenderer *render);

/** OpenGL version of ScreenSpaceDrawable Builder
 */
class ScreenSpaceDrawableBuilderMTL : public BasicDrawableBuilderMTL, public ScreenSpaceDrawableBuilder
{
public:
    ScreenSpaceDrawableBuilderMTL(const std::string &name);
    
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings = -1);
    
    /// Fill out and return the drawable
    virtual BasicDrawable *getDrawable();
};
    
}

