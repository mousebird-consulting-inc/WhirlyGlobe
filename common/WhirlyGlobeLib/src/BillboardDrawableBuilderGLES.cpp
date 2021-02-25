/*
 *  BillboardDrawableInstanceBuilderGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/15/19.
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

#import "BillboardDrawableBuilderGLES.h"

namespace WhirlyKit
{
    
BillboardDrawableBuilderGLES::BillboardDrawableBuilderGLES(const std::string &name,Scene *scene)
: BasicDrawableBuilderGLES(name,scene,true)
{
}
    
int BillboardDrawableBuilderGLES::addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int slot,int numThings)
{
    return BasicDrawableBuilderGLES::addAttribute(dataType, nameID, slot, numThings);
}

BasicDrawableRef BillboardDrawableBuilderGLES::getDrawable()
{
    return BasicDrawableBuilderGLES::getDrawable();
}

    
static const char *vertexShaderGroundTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform vec3 u_eyeVec;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec3 a_normal;
attribute vec3 a_offset;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color;
    vec3 axisX = cross(u_eyeVec,a_normal);
    vec3 axisZ = cross(axisX,a_normal);
    vec3 newPos = a_position + axisX * a_offset.x + a_normal * a_offset.y + axisZ * a_offset.z;
    
    gl_Position = u_mvpMatrix * vec4(newPos,1.0);
}
)";

static const char *vertexShaderEyeTri = R"(
precision highp float;

uniform mat4  u_mvMatrix;
uniform mat4  u_pMatrix;
uniform float u_fade;
uniform vec3 u_eyeVec;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec3 a_normal;
attribute vec3 a_offset;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    v_texCoord = a_texCoord0;
    v_color = a_color;
    vec4 pos = u_mvMatrix * vec4(a_position,1.0);
    vec3 pos3 = (pos/pos.w).xyz;
    vec3 newPos = vec3(pos3.x + a_offset.x,pos3.y+a_offset.y,pos3.z+a_offset.z);
    gl_Position = u_pMatrix * vec4(newPos,1.0);
    //
    //   vec3 axisX = cross(u_eyeVec,normal);
    //   vec3 axisZ = cross(axisX,normal);
    //   vec3 newPos = a_position + axisX * a_offset.x + a_normal * a_offset.y + axisZ * a_offset.z;
    //
    //   gl_Position = u_mvpMatrix * vec4(newPos,1.0);
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
    //  vec4 baseColor = texture2D(s_baseMap0, v_texCoord);
    vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0);
    if (baseColor.a < 0.1)
        discard;
    gl_FragColor = v_color * baseColor;
}
)";

ProgramGLES *BuildBillboardGroundProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderGroundTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        CheckGLError("BuildBillboardGroundProgram() glUseProgram");
        
        shader->setUniform(u_EyeVecNameID, Point3f(0,0,1));
    }
    
    
    return shader;
}

ProgramGLES *BuildBillboardEyeProgramGLES(const std::string &name,SceneRenderer *render)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderEyeTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        
        shader->setUniform(u_EyeVecNameID, Point3f(0,0,1));
    }
    
    
    return shader;
}
    
}

