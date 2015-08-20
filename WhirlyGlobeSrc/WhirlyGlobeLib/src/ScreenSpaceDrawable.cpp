/*
 *  ScreenSpaceDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/24/14.
 *  Copyright 2011-2015 mousebird consulting.
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

#import "ScreenSpaceDrawable.h"
#import "OpenGLES2Program.h"
#import "SceneRendererES.h"
#import "FlatMath.h"

namespace WhirlyKit
{

ScreenSpaceDrawable::ScreenSpaceDrawable(bool hasMotion,bool hasRotation) : BasicDrawable("ScreenSpace"), keepUpright(false), motion(hasMotion), rotation(hasRotation), offsetIndex(-1), rotIndex(-1), dirIndex(-1)
{
    offsetIndex = addAttribute(BDFloat2Type, "a_offset");
    if (hasRotation)
        rotIndex = addAttribute(BDFloat3Type, "a_rot");
    if (hasMotion)
        dirIndex = addAttribute(BDFloat3Type, "a_dir");
}
    
void ScreenSpaceDrawable::setKeepUpright(bool newVal)
{
    keepUpright = newVal;
}

void ScreenSpaceDrawable::addOffset(const Point2f &offset)
{
    addAttributeValue(offsetIndex, offset);
}

void ScreenSpaceDrawable::addOffset(const Point2d &offset)
{
    addAttributeValue(offsetIndex, Point2f(offset.x(),offset.y()));
}
    
void ScreenSpaceDrawable::addDir(const Point3d &dir)
{
    addAttributeValue(dirIndex, Point3f(dir.x(),dir.y(),dir.z()));
}

void ScreenSpaceDrawable::addDir(const Point3f &dir)
{
    addAttributeValue(dirIndex, dir);
}
    
void ScreenSpaceDrawable::addRot(const Point3d &rotDir)
{
    addRot(Point3f(rotDir.x(),rotDir.y(),rotDir.z()));
}

void ScreenSpaceDrawable::addRot(const Point3f &rotDir)
    {
    addAttributeValue(rotIndex, rotDir);
}

void ScreenSpaceDrawable::updateRenderer(WhirlyKit::SceneRendererES *renderer)
{
    renderer->setRenderUntil(fadeUp);
    renderer->setRenderUntil(fadeDown);

    if (motion)
    {
        // Motion requires continuous rendering
        renderer->addContinuousRenderRequest(getId());
    }
}

void ScreenSpaceDrawable::draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene)
{
    if (frameInfo->program)
    {
        frameInfo->program->setUniform("u_scale", (Point2f)(frameInfo->sceneRenderer->getFramebufferSize()/2.f));
        frameInfo->program->setUniform("u_upright", keepUpright);
        if (motion)
            frameInfo->program->setUniform("u_time", (float)(frameInfo->currentTime - startTime));
        frameInfo->program->setUniform("u_activerot", (rotIndex >= 0 ? 1 : 0));
    }

    BasicDrawable::draw(frameInfo,scene);
}

static const char *vertexShaderTri =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_fade;"
"uniform vec2  u_scale;"
"uniform bool  u_activerot;"
""
"attribute vec3 a_position;"
"attribute vec3 a_normal;"
"attribute vec2 a_texCoord0;"
"attribute vec4 a_color;"
"attribute vec2 a_offset;"
"attribute vec3 a_rot;"
""
"varying vec2 v_texCoord;"
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_texCoord = a_texCoord0;"
"   v_color = a_color * u_fade;"
""
// Convert from model space into display space
"   vec4 pt = u_mvMatrix * vec4(a_position,1.0);"
"   pt /= pt.w;"
// Make sure the object is facing the user
"   vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);"
"   float dot_res = dot(-pt.xyz,testNorm.xyz);"
// Project the point all the way to screen space
"   vec4 screenPt = (u_mvpMatrix * vec4(a_position,1.0));"
"   screenPt /= screenPt.w;"
// Project the rotation into display space and drop the Z
"   vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);"
"   vec2 rotY = normalize(projRot.xy);"
"   vec2 rotX = vec2(rotY.y,-rotY.x);"
"   vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);"
"   gl_Position = (dot_res > 0.0 && pt.z <= 0.0) ? vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0) : vec4(0.0,0.0,0.0,0.0);"
"}"
;
    
static const char *vertexShaderMotionTri =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_fade;"
"uniform vec2  u_scale;"
"uniform float u_time;"
"uniform bool  u_activerot;"
""
"attribute vec3 a_position;"
"attribute vec3 a_dir;"
"attribute vec3 a_normal;"
"attribute vec2 a_texCoord0;"
"attribute vec4 a_color;"
"attribute vec2 a_offset;"
"attribute vec3 a_rot;"
""
"varying vec2 v_texCoord;"
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_texCoord = a_texCoord0;"
"   v_color = a_color * u_fade;"
""
// Position can be modified over time
"   vec3 thePos = a_position + u_time * a_dir;"
// Convert from model space into display space
"   vec4 pt = u_mvMatrix * vec4(thePos,1.0);"
"   pt /= pt.w;"
// Make sure the object is facing the user
"   vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);"
"   float dot_res = dot(-pt.xyz,testNorm.xyz);"
// Project the point all the way to screen space
"   vec4 screenPt = (u_mvpMatrix * vec4(thePos,1.0));"
"   screenPt /= screenPt.w;"
// Project the rotation into display space and drop the Z
"   vec4 projRot = u_mvNormalMatrix * vec4(a_rot,0.0);"
"   vec2 rotY = normalize(projRot.xy);"
"   vec2 rotX = vec2(rotY.y,-rotY.x);"
"   vec2 screenOffset = (u_activerot ? a_offset.x*rotX + a_offset.y*rotY : a_offset);"
"   gl_Position = (dot_res > 0.0 && pt.z <= 0.0) ? vec4(screenPt.xy + vec2(screenOffset.x*u_scale.x,screenOffset.y*u_scale.y),0.0,1.0) : vec4(0.0,0.0,0.0,0.0);"
"}"
;

static const char *fragmentShaderTri =
"precision lowp float;"
""
"uniform sampler2D s_baseMap0;"
""
"varying vec2      v_texCoord;"
"varying vec4      v_color;"
""
"void main()"
"{"
"  vec4 baseColor = texture2D(s_baseMap0, v_texCoord);"
"  gl_FragColor = v_color * baseColor;"
"}"
;

WhirlyKit::OpenGLES2Program *BuildScreenSpaceProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kScreenSpaceShaderName,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}
    
WhirlyKit::OpenGLES2Program *BuildScreenSpaceMotionProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kScreenSpaceShaderMotionName,vertexShaderMotionTri,fragmentShaderTri);
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
