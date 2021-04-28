/*  BasicDrawableBuilderGLES.cpp
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

#import "BasicDrawableBuilderGLES.h"
#import <WhirlyKitLog.h>

using namespace Eigen;

namespace WhirlyKit
{

void BasicDrawableTweakerGLES::tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
{
    if (colorExp || opacityExp)
    if (auto program = dynamic_cast<const ProgramGLES*>(frameInfo->program))
    if (auto draw = dynamic_cast<BasicDrawable*>(inDraw))
    {
        const float zoom = getZoom(*inDraw,*frameInfo->scene,-1.0f);
        if (zoom >= 0)
        {
            auto c = colorExp ? colorExp->evaluate(zoom, color) : color;
            if (opacityExp)
            {
                const auto a = (uint8_t) (255.0f * opacityExp->evaluate(zoom, 1.0f));
                c = RGBAColor::FromInt((int)(((uint32_t)c.asInt() & 0x00FFFFFFU) | ((uint32_t)a << 24U)));
            }
            c.r *= c.a/255.0;  c.g *= c.a/255.0;  c.b *= c.a/255.0;
            draw->setOverrideColor(c);
            return;
        }
        else
        {
            wkLogLevel(Warn, "Failed to get zoom level for tweaker");
        }
    }
    // No tweaker should be set up if there's no work to do (no expressions)
    wkLogLevel(Warn, "Unexpected state for tweaker");
}

BasicDrawableBuilderGLES::BasicDrawableBuilderGLES(const std::string &name,Scene *scene,bool setupStandard)
    : BasicDrawableBuilder(name,scene), drawableGotten(false)
{
    basicDraw = std::make_shared<BasicDrawableGLES>(name);
    BasicDrawableBuilder::Init();
    if (setupStandard)
        setupStandardAttributes();  // NOLINT: derived virtual not called here
}

BasicDrawableBuilderGLES::~BasicDrawableBuilderGLES()
{
    if (!drawableGotten)
        basicDraw.reset();
}

// NOLINTNEXTLINE(google-default-arguments)
int BasicDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    VertexAttribute *attr = new VertexAttributeGLES(dataType,nameID);
    if (numThings > 0)
        attr->reserve(numThings);
    basicDraw->vertexAttributes.push_back(attr);

    return (int)(basicDraw->vertexAttributes.size()-1);
}

BasicDrawableRef BasicDrawableBuilderGLES::getDrawable()
{
    auto draw = std::dynamic_pointer_cast<BasicDrawableGLES>(basicDraw);

    if (draw && !drawableGotten) {
        draw->points = points;
        draw->tris = tris;
        draw->vertexSize = (int)draw->singleVertexSize();

        ((BasicDrawableBuilder*)this)->setupTweaker(*draw);

        drawableGotten = true;
    }

    return draw;
}

DrawableTweakerRef BasicDrawableBuilderGLES::makeTweaker() const
{
    if (colorExp || opacityExp)
    {
        return std::make_shared<BasicDrawableTweakerGLES>();
    }
    return {};
}

void BasicDrawableBuilderGLES::setupTweaker(const DrawableTweakerRef &inTweaker) const
{
    if (auto tweaker = std::dynamic_pointer_cast<BasicDrawableTweaker>(inTweaker))
    {
        tweaker->color = basicDraw->color;
        tweaker->colorExp = colorExp;
        tweaker->opacityExp = opacityExp;
    }
}

}
