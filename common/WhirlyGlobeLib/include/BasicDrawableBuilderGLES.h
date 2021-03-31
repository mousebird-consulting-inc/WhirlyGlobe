/*  BasicDrawableBuilderGLES.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/10/19.
 *  Copyright 2011-2021 mousebird consulting
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

#import "BasicDrawableBuilder.h"
#import "BasicDrawableGLES.h"

namespace WhirlyKit
{

/// GLES version modifies uniforms
struct BasicDrawableTweakerGLES : public BasicDrawableTweaker
{
    virtual void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo) override;
};

/** OpenGL version of BasicDrawable Builder.
  */
class BasicDrawableBuilderGLES : virtual public BasicDrawableBuilder
{
public:
    /// Construct empty
    BasicDrawableBuilderGLES(const std::string &name,Scene *scene,bool setupStandard=true);
    ~BasicDrawableBuilderGLES();
    
    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot = -1,int numThings = -1) override;

    /// Fill out and return the drawable
    virtual BasicDrawableRef getDrawable() override;

protected:
    virtual DrawableTweakerRef makeTweaker() const override;
    virtual void setupTweaker(const DrawableTweakerRef &inTweaker) const override;

    bool drawableGotten;
};
    
}
