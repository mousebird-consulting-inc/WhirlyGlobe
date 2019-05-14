/*
 *  ScreenSpaceDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/24/14.
 *  Copyright 2011-2019 mousebird consulting.
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
#import "ProgramGLES.h"

namespace WhirlyKit
{
    
// Modifies the uniform values of a given shader right before the
//  screenspace's Basic Drawables are rendered
class ScreenSpaceTweaker : public DrawableTweaker
{
public:
    void tweakForFrame(Drawable *inDraw,RendererFrameInfo *frameInfo)
    {
        BasicDrawable *draw = (BasicDrawable *)inDraw;
        if (frameInfo->program)
        {
            Point2f fbSize = frameInfo->sceneRenderer->getFramebufferSize();
            frameInfo->program->setUniform(u_ScaleNameID, Point2f(2.f/fbSize.x(),2.f/(float)fbSize.y()));
            frameInfo->program->setUniform(u_uprightNameID, keepUpright);
            if (draw->hasMotion())
                frameInfo->program->setUniform(u_TimeNameID, (float)(frameInfo->currentTime - startTime));
            frameInfo->program->setUniform(u_activerotNameID, (rotIndex >= 0 ? 1 : 0));
        }
    }
    
    TimeInterval startTime;
    bool keepUpright;
    int rotIndex;
};

ScreenSpaceDrawableBuilder::ScreenSpaceDrawableBuilder()
: keepUpright(false), motion(false), rotation(false), offsetIndex(-1), rotIndex(-1), dirIndex(-1)
{
}

void ScreenSpaceDrawableBuilder::setup(bool hasMotion,bool hasRotation)
{
    Init();
    setupStandardAttributes();
    
    offsetIndex = addAttribute(BDFloat2Type, a_offsetNameID);
    if (hasRotation)
        rotIndex = addAttribute(BDFloat3Type, a_rotNameID);
    if (hasMotion)
        dirIndex = addAttribute(BDFloat3Type, a_dirNameID);
}
    
void ScreenSpaceDrawableBuilder::setKeepUpright(bool newVal)
{
    keepUpright = newVal;
}
    
void ScreenSpaceDrawableBuilder::setStartTime(TimeInterval inStartTime)
    { startTime = inStartTime; }

TimeInterval ScreenSpaceDrawableBuilder::getStartTime()
    { return startTime; }

void ScreenSpaceDrawableBuilder::addOffset(const Point2f &offset)
{
    addAttributeValue(offsetIndex, offset);
}

void ScreenSpaceDrawableBuilder::addOffset(const Point2d &offset)
{
    addAttributeValue(offsetIndex, Point2f(offset.x(),offset.y()));
}
    
void ScreenSpaceDrawableBuilder::addDir(const Point3d &dir)
{
    addAttributeValue(dirIndex, Point3f(dir.x(),dir.y(),dir.z()));
}

void ScreenSpaceDrawableBuilder::addDir(const Point3f &dir)
{
    addAttributeValue(dirIndex, dir);
}
    
void ScreenSpaceDrawableBuilder::addRot(const Point3d &rotDir)
{
    addRot(Point3f(rotDir.x(),rotDir.y(),rotDir.z()));
}

void ScreenSpaceDrawableBuilder::addRot(const Point3f &rotDir)
{
    addAttributeValue(rotIndex, rotDir);
}
    
void ScreenSpaceDrawableBuilder::setupTweaker(BasicDrawable *theDraw)
{
    ScreenSpaceTweaker *tweak = new ScreenSpaceTweaker();
    tweak->startTime = startTime;
    tweak->keepUpright = keepUpright;
    tweak->rotIndex = rotIndex;
    theDraw->addTweaker(DrawableTweakerRef(tweak));
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

Program *BuildScreenSpaceProgram(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

Program *BuildScreenSpace2DProgram(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri2d,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

Program *BuildScreenSpaceMotionProgram(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderMotionTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}

Program *BuildScreenSpaceMotion2DProgram(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShader2dMotionTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}
    
}
