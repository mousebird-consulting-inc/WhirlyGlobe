/*
 *  WideVectorDrawableBuilderGLES.cpp
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

#import "WideVectorDrawableBuilderGLES.h"

using namespace Eigen;

namespace WhirlyKit
{
    
// OpenGL version of the tweaker
void WideVectorTweakerGLES::tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
{
    if (frameInfo->program)
    {
        float scale = std::max(frameInfo->sceneRenderer->framebufferWidth,frameInfo->sceneRenderer->framebufferHeight);
        float screenSize = frameInfo->screenSizeInDisplayCoords.x();
        float pixDispSize = std::min(frameInfo->screenSizeInDisplayCoords.x(),frameInfo->screenSizeInDisplayCoords.y()) / scale;
        float texScale = scale/(screenSize*texRepeat);
        
        ProgramGLES *programGLES = (ProgramGLES *)frameInfo->program;

        if (realWidthSet)
        {
            programGLES->setUniform(u_w2NameID, (float)(realWidth / pixDispSize));
            programGLES->setUniform(u_Realw2NameID, (float)realWidth);
            programGLES->setUniform(u_EdgeNameID, edgeSize);
        } else {
            programGLES->setUniform(u_w2NameID, lineWidth);
            programGLES->setUniform(u_Realw2NameID, pixDispSize * lineWidth);
            programGLES->setUniform(u_EdgeNameID, edgeSize);
        }
        programGLES->setUniform(u_texScaleNameID, texScale);
        programGLES->setUniform(u_colorNameID, Vector4f(color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0));
    }
}

WideVectorDrawableBuilderGLES::WideVectorDrawableBuilderGLES(const std::string &name,Scene *scene)
: BasicDrawableBuilderGLES(name,scene,false)
{
}
    
void WideVectorDrawableBuilderGLES::Init(unsigned int numVert,unsigned int numTri,bool globeMode)
{
    basicDraw = std::make_shared<BasicDrawableGLES>("Wide Vector");
    WideVectorDrawableBuilder::Init(numVert,numTri,globeMode);
}
    
int WideVectorDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    return BasicDrawableBuilderGLES::addAttribute(dataType, nameID, slot, numThings);
}
    
WideVectorTweaker *WideVectorDrawableBuilderGLES::makeTweaker()
{
    return new WideVectorTweakerGLES();
}
    
BasicDrawableRef WideVectorDrawableBuilderGLES::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderGLES::getDrawable();
    
    const auto theDraw = BasicDrawableBuilderGLES::getDrawable();
    setupTweaker(theDraw.get());
        
    return theDraw;
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
uniform vec4 u_color;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_texinfo;
//attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;

varying vec2 v_texCoord;
//"varying vec4 v_color;

void main()
{
    //   v_color = a_color;\n"
    //  Position along the line
    float t0 = a_c0 * u_real_w2;
    t0 = clamp(t0,0.0,1.0);
    vec3 realPos = (a_p1 - a_position) * t0 + a_n0 * u_real_w2 + a_position;
    float texPos = ((a_texinfo.z - a_texinfo.y) * t0 + a_texinfo.y + a_texinfo.w * u_real_w2) * u_texScale;
    v_texCoord = vec2(a_texinfo.x, texPos);
    vec4 screenPos = u_mvpMatrix * vec4(realPos,1.0);
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
uniform vec4 u_color;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_texinfo;
//attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;

varying vec2 v_texCoord;
//varying vec4 v_color;
varying float      v_dot;

void main()
{
    //   v_color = a_color;
    //  Position along the line
    float t0 = a_c0 * u_real_w2;
    t0 = clamp(t0,0.0,1.0);
    vec3 realPos = (a_p1 - a_position) * t0 + a_n0 * u_real_w2 + a_position;
    vec4 pt = u_mvMatrix * vec4(a_position,1.0);
    pt /= pt.w;
    vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);
    v_dot = dot(-pt.xyz,testNorm.xyz);
    float texPos = ((a_texinfo.z - a_texinfo.y) * t0 + a_texinfo.y + a_texinfo.w * u_real_w2) * u_texScale;
    v_texCoord = vec2(a_texinfo.x, texPos);
    vec4 screenPos = u_mvpMatrix * vec4(realPos,1.0);
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
uniform vec4 u_color;
uniform float u_fade;

varying vec2      v_texCoord;

void main()
{
    float patternVal = u_hasTexture ? texture2D(s_baseMap0, vec2(0.5,v_texCoord.y)).a : 1.0;
    float alpha = 1.0;
    float across = v_texCoord.x * u_w2;
    if (across < u_edge)
        alpha = across/u_edge;
    if (across > u_w2-u_edge)
        alpha = (u_w2-across)/u_edge;
    gl_FragColor = u_color * alpha * patternVal * u_fade;
}
)";

static const char *fragmentGlobeShaderTriAlias = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform bool  u_hasTexture;
uniform float u_w2;
uniform float u_edge;
uniform vec4 u_color;
uniform float u_fade;

varying vec2      v_texCoord;
varying float      v_dot;
//"varying vec4      v_color;

void main()
{
    float patternVal = u_hasTexture ? texture2D(s_baseMap0, vec2(0.5,v_texCoord.y)).a : 1.0;
    float alpha = 1.0;
    float across = v_texCoord.x * u_w2;
    if (across < u_edge)
        alpha = across/u_edge;
    if (across > u_w2-u_edge)
        alpha = (u_w2-across)/u_edge;
    gl_FragColor = (v_dot > 0.0 ? u_color * alpha * patternVal * u_fade : vec4(0.0,0.0,0.0,0.0));
}
)";

ProgramGLES *BuildWideVectorProgramGLES(const std::string &name,SceneRenderer *renderer)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTriAlias);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
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

ProgramGLES *BuildWideVectorGlobeProgramGLES(const std::string &name,SceneRenderer *renderer)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexGlobeShaderTri,fragmentGlobeShaderTriAlias);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
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
