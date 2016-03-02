/*
 *  WideVectorDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
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

#import "WideVectorDrawable.h"
#import "OpenGLES2Program.h"
#import "SceneRendererES.h"
#import "FlatMath.h"

using namespace Eigen;

namespace WhirlyKit
{
    
WideVectorDrawable::WideVectorDrawable() : BasicDrawable("WideVector"), texRepeat(1.0)
{
    lineWidth = 10.0/1024.0;
    offsetIndex = addAttribute(BDFloat3Type, "a_dir");
    lenIndex = addAttribute(BDFloatType, "a_maxlen");
}
    
void WideVectorDrawable::addDir(const Point3f &dir)
{
    addAttributeValue(offsetIndex, dir);
}
    
void WideVectorDrawable::addDir(const Point3d &dir)
{
    addAttributeValue(offsetIndex, Point3f(dir.x(),dir.y(),dir.z()));

//    // Note: Debugging
//    dirs.push_back(Point3d(dir.x(),dir.y(),dir.z()));
}

void WideVectorDrawable::addMaxLen(double len)
{
    addAttributeValue(lenIndex, len);
    
//    // Note: Debugging
//    lens.push_back(len);
}
    
void WideVectorDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo, Scene *scene)
{
//    double u_length = 0.0;
//    double u_scale = 1.0;
//    double u_pixDispSize = 0.0;
    
    if (frameInfo.program)
    {
        float scale = std::max(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight);
        float screenSize = frameInfo.screenSizeInDisplayCoords.x();
        float pixDispSize = std::min(frameInfo.screenSizeInDisplayCoords.x(),frameInfo.screenSizeInDisplayCoords.y()) / scale;
        frameInfo.program->setUniform("u_scale", 1.f/scale);
        frameInfo.program->setUniform("u_length", lineWidth/scale);
        frameInfo.program->setUniform("u_pixDispSize", pixDispSize);
        frameInfo.program->setUniform("u_lineWidth", lineWidth);
        float texScale = scale/(screenSize*texRepeat);
        frameInfo.program->setUniform("u_texScale", texScale);
        
        // Note: Debugging
//        u_length = lineWidth/scale;
//        u_scale = scale;
//        u_pixDispSize = pixDispSize;
    }
    
    BasicDrawable::draw(frameInfo,scene);
    
    // Note: Debugging
//    for (unsigned int ii=0;ii<dirs.size();ii++)
//    {
//        double len = lens[ii];
//        Point3d dir = dirs[ii];
//        if (u_pixDispSize * dir.norm() * lineWidth > len)
//        {
//            NSLog(@"Dropping one");
//        }
//    }
}

static const char *vertexShaderTri =
"uniform mat4  u_mvpMatrix;\n"
"uniform mat4  u_mvMatrix;\n"
"uniform mat4  u_pMatrix;\n"
"uniform float u_fade;\n"
"uniform float u_length;\n"
"uniform float u_texScale;\n"
"uniform float u_pixDispSize;\n"
"uniform float u_lineWidth;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec2 a_texCoord0;\n"
"attribute vec4 a_color;\n"
"attribute vec3 a_dir;\n"
"attribute float a_maxlen;\n"
"\n"
"varying vec2 v_texCoord;\n"
"varying vec4 v_color;\n"
"\n"
"void main()\n"
"{\n"
"   v_texCoord = vec2(a_texCoord0.x, a_texCoord0.y * u_texScale);\n"
"   v_color = a_color;\n"
"   vec4 vertPos = u_mvpMatrix * vec4(a_position,1.0);\n"
"   vertPos /= vertPos.w;\n"
"   vec2 screenDir = (u_mvpMatrix * vec4(a_dir,0.0)).xy;\n"
"   vec2 calcOff = screenDir * u_length;\n"
"   if (u_pixDispSize * length(a_dir) * u_lineWidth > a_maxlen && a_maxlen > 0.0)\n"
"      calcOff = vec2(0,0);\n"
"   gl_Position = vertPos + vec4(calcOff,0,0);\n"
"}\n"
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
"  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0); \n"
"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;

WhirlyKit::OpenGLES2Program *BuildWideVectorProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kWideVectorShaderName,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    // Set some reasonable defaults
    if (shader)
    {
        glUseProgram(shader->getProgram());
        
        shader->setUniform("u_length", 10.f/1024);
        shader->setUniform("u_texScale", 1.f);
    }
    
    
    return shader;
}
    
    
    
}
