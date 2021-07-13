/*  ScreenSpaceDrawableBuilderGLES.cpp
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

#import "ScreenSpaceDrawableBuilderGLES.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
void ScreenSpaceTweakerGLES::tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
{
    if (auto programGLES = dynamic_cast<ProgramGLES*>(frameInfo->program))
    {
        const float zoom = (opacityExp || colorExp || scaleExp) ? getZoom(*inDraw,*frameInfo->scene,0.0f) : 0.0f;

        float scale = scaleExp ? scaleExp->evaluate(zoom, 1.0) : 1.0;

        const Point2f fbSize = frameInfo->sceneRenderer->getFramebufferSize();
        programGLES->setUniform(u_ScaleNameID, Point2f(2.f * scale / fbSize.x(), 2.f * scale / (float) fbSize.y()));
        programGLES->setUniform(u_uprightNameID, keepUpright);
        programGLES->setUniform(u_activerotNameID, (activeRot ? 1 : 0));
    }
    if (auto draw = dynamic_cast<BasicDrawable*>(inDraw))
    {
        if (draw->hasMotion())
        {
            //programGLES->setUniform(u_TimeNameID, (float) (frameInfo->currentTime - startTime));
            draw->setUniform(u_TimeNameID, (float) (frameInfo->currentTime - startTime));
        }
    }
}

ScreenSpaceDrawableBuilderGLES::ScreenSpaceDrawableBuilderGLES(const std::string &name,Scene *scene)
    : BasicDrawableBuilderGLES(name,scene,true)
{
}
    
int ScreenSpaceDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    return BasicDrawableBuilderGLES::addAttribute(dataType, nameID, slot, numThings);
}
    
DrawableTweakerRef ScreenSpaceDrawableBuilderGLES::makeTweaker() const
{
    return std::make_shared<ScreenSpaceTweakerGLES>();
}

void ScreenSpaceDrawableBuilderGLES::setupTweaker(BasicDrawable &draw) const
{
    // Diamond inheritance, this method only exists to eliminate ambiguity
    BasicDrawableBuilder::setupTweaker(draw);
}

void ScreenSpaceDrawableBuilderGLES::setupTweaker(const DrawableTweakerRef &inTweaker) const
{
    // Need to set up both parts of the tweaker
    BasicDrawableBuilderGLES::setupTweaker(inTweaker);
    ScreenSpaceDrawableBuilder::setupTweaker(inTweaker);
    ScreenSpaceTweakerGLESRef theTweaker = std::dynamic_pointer_cast<ScreenSpaceTweakerGLES>(inTweaker);
    if (theTweaker && scaleExp) {
        theTweaker->scaleExp = scaleExp;
    }
}

BasicDrawableRef ScreenSpaceDrawableBuilderGLES::getDrawable()
{
    if (drawableGotten)
        return BasicDrawableBuilderGLES::getDrawable();
    
    auto theDraw = BasicDrawableBuilderGLES::getDrawable();
    theDraw->motion = motion;

    setupTweaker(*theDraw);
    
    return theDraw;
}
    
static const char *vertexShaderTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform vec2  u_scale;
uniform bool  u_activerot;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec2 a_offset;
attribute vec3 a_rot;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color * u_fade;
    
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(a_position,1.0);
    pt /= pt.w;
    // Make sure the object is facing the user
    vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);
    float dot_res = dot(-pt.xyz,testNorm.xyz);
    // Project the point all the way to screen space
    vec4 screenPt = (u_mvpMatrix * vec4(a_position,1.0));
    screenPt /= screenPt.w;
    // Project the rotation into display space and drop the Z
    vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);
    vec2 rotY = normalize(projRot.xy);
    vec2 rotX = vec2(rotY.y,-rotY.x);
    vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);
    gl_Position = (dot_res > 0.0 && pt.z <= 0.0) ? vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0) : vec4(0.0,0.0,0.0,0.0);
}
)";

static const char *vertexShaderTri2d = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform vec2  u_scale;
uniform bool  u_activerot;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec2 a_offset;
attribute vec3 a_rot;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color * u_fade;
    
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(a_position,1.0);
    pt /= pt.w;
    // Project the point all the way to screen space
    vec4 screenPt = (u_mvpMatrix * vec4(a_position,1.0));
    screenPt /= screenPt.w;
    // Project the rotation into display space and drop the Z
    vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);
    vec2 rotY = normalize(projRot.xy);
    vec2 rotX = vec2(rotY.y,-rotY.x);
    vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);
    gl_Position = vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0);
}
)";


static const char *vertexShaderMotionTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform vec2  u_scale;
uniform float u_time;
uniform bool  u_activerot;

attribute vec3 a_position;
attribute vec3 a_dir;
attribute vec3 a_normal;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec2 a_offset;
attribute vec3 a_rot;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color * u_fade;
    
    // Position can be modified over time
    vec3 thePos = a_position + u_time * a_dir;
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(thePos,1.0);
    pt /= pt.w;
    // Make sure the object is facing the user
    vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);
    float dot_res = dot(-pt.xyz,testNorm.xyz);
    // Project the point all the way to screen space
    vec4 screenPt = (u_mvpMatrix * vec4(thePos,1.0));
    screenPt /= screenPt.w;
    // Project the rotation into display space and drop the Z
    vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);
    vec2 rotY = normalize(projRot.xy);
    vec2 rotX = vec2(rotY.y,-rotY.x);
    vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);
    gl_Position = (dot_res > 0.0 && pt.z <= 0.0) ? vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0) : vec4(0.0,0.0,0.0,0.0);
}
)";

static const char *vertexShader2dMotionTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform mat4  u_mvMatrix;
uniform mat4  u_mvNormalMatrix;
uniform float u_fade;
uniform vec2  u_scale;
uniform float u_time;
uniform bool  u_activerot;

attribute vec3 a_position;
attribute vec3 a_dir;
attribute vec3 a_normal;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec2 a_offset;
attribute vec3 a_rot;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color * u_fade;
    
    // Position can be modified over time
    vec3 thePos = a_position + u_time * a_dir;
    // Convert from model space into display space
    vec4 pt = u_mvMatrix * vec4(thePos,1.0);
    pt /= pt.w;
    // Project the point all the way to screen space
    vec4 screenPt = (u_mvpMatrix * vec4(thePos,1.0));
    screenPt /= screenPt.w;
    // Project the rotation into display space and drop the Z
    vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);
    vec2 rotY = normalize(projRot.xy);
    vec2 rotX = vec2(rotY.y,-rotY.x);
    vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);
    gl_Position = vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0);
}
)";

static const char *fragmentShaderTri = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform bool  u_hasTexture;

varying vec2      v_texCoord;
varying vec4      v_color;

void main()
{
    vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0);
    gl_FragColor = v_color * baseColor;
}
)";

ProgramGLES *BuildScreenSpaceProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

ProgramGLES *BuildScreenSpace2DProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri2d,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

ProgramGLES *BuildScreenSpaceMotionProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderMotionTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

ProgramGLES *BuildScreenSpaceMotion2DProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShader2dMotionTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = nullptr;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

}
