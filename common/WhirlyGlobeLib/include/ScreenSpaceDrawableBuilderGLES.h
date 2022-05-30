/*  ScreenSpaceDrawableBuilderGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "ScreenSpaceDrawableBuilder.h"
#import "BasicDrawableBuilderGLES.h"

namespace WhirlyKit
{

/// Construct and return the Screen Space shader program
ProgramGLES *BuildScreenSpaceProgramGLES(const std::string &name,SceneRenderer *render);
ProgramGLES *BuildScreenSpaceMotionProgramGLES(const std::string &name,SceneRenderer *render);

/// The OpenGL version sets uniforms
struct ScreenSpaceTweakerGLES : public ScreenSpaceTweaker
{
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) override;

    FloatExpressionInfoRef scaleExp;
};
typedef std::shared_ptr<ScreenSpaceTweakerGLES> ScreenSpaceTweakerGLESRef;
    
/** OpenGL version of ScreenSpaceDrawable Builder
 */
struct ScreenSpaceDrawableBuilderGLES : virtual public BasicDrawableBuilderGLES, virtual public ScreenSpaceDrawableBuilder
{
    ScreenSpaceDrawableBuilderGLES(const std::string &name,Scene *scene);
    
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot = -1,int numThings = -1) override;

    /// Fill out and return the drawable
    virtual BasicDrawableRef getDrawable() override;

    virtual DrawableTweakerRef makeTweaker() const override;

    virtual void setupTweaker(BasicDrawable &draw) const override;
    virtual void setupTweaker(const DrawableTweakerRef &inTweaker) const override;
};

}
