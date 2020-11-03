/*
 *  ScreenSpaceDrawableBuilderGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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
#import "BasicDrawableBuilderGLES.h"

namespace WhirlyKit
{
    
// Shader name
//#define kScreenSpaceShaderName "Screen Space Shader"
//#define kScreenSpaceShader2DName "Screen Space Shader 2D"
//#define kScreenSpaceShaderMotionName "Screen Space Shader Motion"
//#define kScreenSpaceShader2DMotionName "Screen Space Shader 2D Motion"

/// Construct and return the Screen Space shader program
ProgramGLES *BuildScreenSpaceProgramGLES(const std::string &name,SceneRenderer *render);
ProgramGLES *BuildScreenSpaceMotionProgramGLES(const std::string &name,SceneRenderer *render);
ProgramGLES *BuildScreenSpace2DProgramGLES(const std::string &name,SceneRenderer *render);
ProgramGLES *BuildScreenSpaceMotion2DProgramGLES(const std::string &name,SceneRenderer *render);
    
/// The OpenGL version sets uniforms
class ScreenSpaceTweakerGLES : public ScreenSpaceTweaker
{
public:
    void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo);
};
    
/** OpenGL version of ScreenSpaceDrawable Builder
 */
class ScreenSpaceDrawableBuilderGLES : virtual public BasicDrawableBuilderGLES, virtual public ScreenSpaceDrawableBuilder
{
public:
    ScreenSpaceDrawableBuilderGLES(const std::string &name,Scene *scene);
    
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot = -1,int numThings = -1);

    virtual ScreenSpaceTweaker *makeTweaker();
    
    /// Fill out and return the drawable
    virtual BasicDrawableRef getDrawable();
};
    
}
