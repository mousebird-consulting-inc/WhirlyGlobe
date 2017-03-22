/*
 *  BillboardDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/13.
 *  Copyright 2011-2015 mousebird consulting
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

#include "BillboardDrawable.h"
#include "OpenGLES2Program.h"

namespace WhirlyKit
{
    
BillboardDrawable::BillboardDrawable() : BasicDrawable("Billboard")
{
    offsetIndex = addAttribute(BDFloat3Type, "a_offset");
}
    
void BillboardDrawable::addOffset(const Point3f &offset)
{
    addAttributeValue(offsetIndex, offset);
}
    
void BillboardDrawable::addOffset(const Point3d &offset)
{
    addAttributeValue(offsetIndex, Point3f(offset.x(),offset.y(),offset.z()));
}
    
static const char *vertexShaderGroundTri =
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"uniform vec3 u_eyeVec;"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"attribute vec3 a_offset;                   \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord0;                 \n"
"   v_color = a_color;         \n"
"   vec3 axisX = cross(u_eyeVec,a_normal);"
"   vec3 axisZ = cross(axisX,a_normal);"
"   vec3 newPos = a_position + axisX * a_offset.x + a_normal * a_offset.y + axisZ * a_offset.z;"
"\n"
"   gl_Position = u_mvpMatrix * vec4(newPos,1.0);  \n"
"}                                           \n"
;
    
static const char *vertexShaderEyeTri =
"uniform mat4  u_mvMatrix;                   \n"
"uniform mat4  u_pMatrix;                   \n"
"uniform float u_fade;                        \n"
"uniform vec3 u_eyeVec;"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"attribute vec3 a_offset;                   \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord0;                 \n"
"   v_color = a_color;"
"   vec4 pos = u_mvMatrix * vec4(a_position,1.0);"
"   vec3 pos3 = (pos/pos.w).xyz;"
"   vec3 newPos = vec3(pos3.x + a_offset.x,pos3.y+a_offset.y,pos3.z+a_offset.z);"
"   gl_Position = u_pMatrix * vec4(newPos,1.0);"
//
//"   vec3 axisX = cross(u_eyeVec,normal);"
//"   vec3 axisZ = cross(axisX,normal);"
//"   vec3 newPos = a_position + axisX * a_offset.x + a_normal * a_offset.y + axisZ * a_offset.z;"
//"\n"
//"   gl_Position = u_mvpMatrix * vec4(newPos,1.0);  \n"
"}                                           \n"
;
    
static const char *fragmentShaderTri =
"precision mediump float;                            \n"
"\n"
"uniform sampler2D s_baseMap0;                        \n"
"uniform bool  u_hasTexture;                         \n"
"\n"
"varying vec2      v_texCoord;                       \n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
//"  vec4 baseColor = texture2D(s_baseMap0, v_texCoord); \n"
"  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0); \n"
"  if (baseColor.a < 0.1)                            \n"
"      discard;                                      \n"
"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;
    
WhirlyKit::OpenGLES2Program *BuildBillboardGroundProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kBillboardShaderName,vertexShaderGroundTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
        
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
            
        shader->setUniform("u_eyeVec", Point3f(0,0,1));
    }
        
        
    return shader;
}
    
WhirlyKit::OpenGLES2Program *BuildBillboardEyeProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kBillboardShaderName,vertexShaderEyeTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
        
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        
        shader->setUniform("u_eyeVec", Point3f(0,0,1));
    }
        
        
    return shader;
}
    
}
