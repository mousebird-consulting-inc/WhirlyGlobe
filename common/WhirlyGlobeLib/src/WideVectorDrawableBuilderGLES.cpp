/*  WideVectorDrawableBuilderGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/14/19.
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

#import <WhirlyGlobe.h>
#import <WideVectorDrawableBuilderGLES.h>
#import <WhirlyKitLog.h>

using namespace Eigen;

namespace WhirlyKit
{

// OpenGL version of the tweaker
void WideVectorTweakerGLES::tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
{
    auto basicDraw = dynamic_cast<BasicDrawableGLES *>(inDraw);
    if (!basicDraw)
    {
        wkLogLevel(Warn, "Invalid drawable passed to WideVectorTweakerGLES");
        return;
    }

    const double scale = std::max(frameInfo->sceneRenderer->framebufferWidth,frameInfo->sceneRenderer->framebufferHeight);
    const double screenSize = frameInfo->screenSizeInDisplayCoords.x();
    const double pixDispSize = std::min(frameInfo->screenSizeInDisplayCoords.x(),frameInfo->screenSizeInDisplayCoords.y()) / scale;
    const double texScale = scale / (screenSize * texRepeat);
    const float zoom = (opacityExp || colorExp || widthExp) ? getZoom(*inDraw,*frameInfo->scene,0.0f) : 0.0f;

    Vector4f c = colorExp ? colorExp->evaluateF(zoom,color) : color.asRGBAVecF();

    if (opacityExp)
    {
        c.w() = opacityExp->evaluate(zoom, 1.0f);
    }

    // Multiply the alpha through, otherwise you just get the max color
    c[0] *= c[3];  c[1] *= c[3];  c[2] *= c[3];
    const RGBAColor newC = RGBAColor::FromUnitFloats(&c[0]);
    basicDraw->setOverrideColor(newC);

    const float width = (widthExp ? widthExp->evaluate(zoom, lineWidth) : lineWidth) + 2 * edgeSize;
    basicDraw->setUniform(u_w2NameID, width);
    basicDraw->setUniform(u_Realw2NameID, (float)(pixDispSize * width));

    basicDraw->setUniform(u_EdgeNameID, edgeSize);
    basicDraw->setUniform(u_texScaleNameID, (float)texScale);
}

WideVectorDrawableBuilderGLES::WideVectorDrawableBuilderGLES(const std::string &name,const SceneRenderer *sceneRenderer,Scene *scene) :
    WideVectorDrawableBuilder(name,sceneRenderer,scene),
    drawableGotten(false),
    instanceGotten(false)
{
}
    
void WideVectorDrawableBuilderGLES::Init(unsigned int numVert,unsigned int numTri,unsigned int numCenterline,
                                         WideVecImplType implType,
                                         bool globeMode,
                                         const WideVectorInfo *vecInfo)
{
    WideVectorDrawableBuilder::Init(numVert,numTri,0,implType,globeMode,vecInfo);
}

// NOLINTNEXTLINE(google-default-arguments)
int WideVectorDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    return basicDrawable->addAttribute(dataType, nameID, slot, numThings);
}

DrawableTweakerRef WideVectorDrawableBuilderGLES::makeTweaker() const
{
    return std::make_shared<WideVectorTweakerGLES>();
}

BasicDrawableRef WideVectorDrawableBuilderGLES::getBasicDrawable()
{
    if (drawableGotten)
    {
        return basicDrawable->getDrawable();
    }

    drawableGotten = true;
    // non-const to allow copy elision on return
    auto theDraw = basicDrawable->getDrawable();

    setupTweaker(*theDraw);

    return theDraw;
}

BasicDrawableInstanceRef  WideVectorDrawableBuilderGLES::getInstanceDrawable()
{
    return nullptr;
}
    
static const char *vertexShaderTri = R"(
precision highp float;
uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform float u_w2;
uniform float u_real_w2;
uniform float u_texScale;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_texinfo;
attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;
attribute vec3 a_offset;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_color = a_color;
    //  Position along the line
    float t0 = a_c0 * u_real_w2;
    t0 = clamp(t0,-4.0,5.0);
    vec3 dir = normalize(a_p1 - a_position);
    vec3 realPosOffset = (a_p1 - a_position) * t0 +
                    dir * u_real_w2 * a_offset.y +
                    a_n0 * u_real_w2 +
                    a_n0 * u_real_w2 * a_offset.x;
    float texPos = ((a_texinfo.z - a_texinfo.y) * t0 + a_texinfo.y + a_texinfo.w * u_real_w2) * u_texScale;
    v_texCoord = vec2(a_texinfo.x, texPos);
    vec4 screenPos = u_mvpMatrix * vec4(a_position,1.0) + u_mvpMatrix * vec4(realPosOffset,0.0);
    screenPos /= screenPos.w;
    gl_Position = vec4(screenPos.xy,0,1.0);
}
)";

static const char *vertexGlobeShaderTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform float u_w2;
uniform float u_real_w2;
uniform float u_texScale;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_texinfo;
attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;
attribute vec3 a_offset;

varying vec2 v_texCoord;
varying vec4 v_color;
varying float      v_dot;

void main()
{
    v_color = a_color;
    //  Position along the line
    float t0 = a_c0 * u_real_w2;
    t0 = clamp(t0,-4.0,5.0);
    vec3 dir = normalize(a_p1 - a_position);
    vec3 realPosOffset = (a_p1 - a_position) * t0 +
                    dir * u_real_w2 * a_offset.y +
                    a_n0 * u_real_w2 +
                    a_n0 * u_real_w2 * a_offset.x;
    vec4 pt = u_mvMatrix * vec4(a_position,1.0);
    pt /= pt.w;
    vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);
    v_dot = dot(-pt.xyz,testNorm.xyz);
    float texPos = ((a_texinfo.z - a_texinfo.y) * t0 + a_texinfo.y + a_texinfo.w * u_real_w2) * u_texScale;
    v_texCoord = vec2(a_texinfo.x, texPos);
    vec4 screenPos = u_mvpMatrix * vec4(a_position,1.0) + u_mvpMatrix * vec4(realPosOffset,0.0);
    screenPos /= screenPos.w;
    gl_Position = vec4(screenPos.xy,0,1.0);
}
)";

static const char *fragmentShaderTriAlias = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform bool  u_hasTexture;
uniform float u_w2;
uniform float u_edge;
uniform float u_fade;

varying vec2      v_texCoord;
varying vec4      v_color;

void main()
{
    float patternVal = u_hasTexture ? texture2D(s_baseMap0, vec2(0.5,v_texCoord.y)).a : 1.0;
    float alpha = 1.0;
    float across = v_texCoord.x * u_w2;
    if (across < u_edge)
        alpha = across/u_edge;
    if (across > u_w2-u_edge)
        alpha = (u_w2-across)/u_edge;
    gl_FragColor = v_color * alpha * patternVal * u_fade;
}
)";

static const char *fragmentGlobeShaderTriAlias = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform bool  u_hasTexture;
uniform float u_w2;
uniform float u_edge;
uniform float u_fade;

varying vec2      v_texCoord;
varying float      v_dot;
varying vec4      v_color;

void main()
{
    float patternVal = u_hasTexture ? texture2D(s_baseMap0, vec2(0.5,v_texCoord.y)).a : 1.0;
    float alpha = 1.0;
    float across = v_texCoord.x * u_w2;
    if (across < u_edge)
        alpha = across/u_edge;
    if (across > u_w2-u_edge)
        alpha = (u_w2-across)/u_edge;
    gl_FragColor = (v_dot > 0.0 ? v_color * alpha * patternVal * u_fade : vec4(0.0,0.0,0.0,0.0));
}
)";

ProgramGLES *BuildWideVectorProgramGLES(const std::string &name, SceneRenderer *)
{
    auto shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTriAlias);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        
        shader->setUniform(u_lengthNameID, 10.f/1024);
        shader->setUniform(u_texScaleNameID, 1.f);
    }
    
    
    return shader;
}

ProgramGLES *BuildWideVectorGlobeProgramGLES(const std::string &name, SceneRenderer *)
{
    auto shader = new ProgramGLES(name,vertexGlobeShaderTri,fragmentGlobeShaderTriAlias);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        
        shader->setUniform(u_lengthNameID, 10.f/1024);
        shader->setUniform(u_texScaleNameID, 1.f);
    }

    return shader;
}
    
}
