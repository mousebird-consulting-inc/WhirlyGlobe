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
    p1_index = addAttribute(BDFloat3Type, "a_p1");
    t0_limit_index = addAttribute(BDFloatType, "a_t0_limit");
    n0_index = addAttribute(BDFloat3Type, "a_n0");
    c0_index = addAttribute(BDFloatType, "a_c0");
}
    
unsigned int WideVectorDrawable::addPoint(const Point3f &pt)
{
    locPts.push_back(pt);
    return BasicDrawable::addPoint(pt);
}
    
void WideVectorDrawable::add_p1(const Point3f &pt)
{
    NSLog(@"p1 = (%f,%f,%f)",pt.x(),pt.y(),pt.z());
    
    addAttributeValue(p1_index, pt);
    p1.push_back(pt);
}

void WideVectorDrawable::add_n0(const Point3f &dir)
{
    addAttributeValue(n0_index, dir);
    n0.push_back(dir);
}

void WideVectorDrawable::add_t0_limit(float minVal,float maxVal)
{
    addAttributeValue(t0_limit_index, Point2f(minVal,maxVal));
    t0_limits.push_back(Point2f(minVal,maxVal));
}

void WideVectorDrawable::add_c0(float val)
{
    addAttributeValue(c0_index, val);
    c0.push_back(val);
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
//        float pixDispSize = std::min(frameInfo.screenSizeInDisplayCoords.x(),frameInfo.screenSizeInDisplayCoords.y()) / scale;
//        frameInfo.program->setUniform("u_scale", 1.f/scale);
        frameInfo.program->setUniform("u_w2", lineWidth/(scale));
        frameInfo.program->setUniform("u_real_w2", pixDispSize * lineWidth);
//        frameInfo.program->setUniform("u_pixDispSize", pixDispSize);
//        frameInfo.program->setUniform("u_lineWidth", lineWidth);
        float texScale = scale/(screenSize*texRepeat);
        frameInfo.program->setUniform("u_texScale", texScale);
        
        // Note: Debugging
        // Redo the calculation for debugging
        NSLog(@"\n");
        for (unsigned int ii=0;ii<locPts.size();ii++)
        {
            float u_w2 = lineWidth/(2.f*scale);
            float u_real_w2 = pixDispSize * lineWidth;
            Point3f a_p0 = locPts[ii];
            Point3f a_p1 = p1[ii];
            Point2f a_t0_limit = t0_limits[ii];
            Point3f a_n0 = n0[ii];
            float a_c0 = c0[ii];
            
            Vector4f screen_p0 = frameInfo.mvpMat * Vector4f(a_p0.x(),a_p0.y(),a_p0.z(),1.0);
            screen_p0 /= screen_p0.w();
            Vector4f screen_p1 = frameInfo.mvpMat * Vector4f(a_p1.x(),a_p1.y(),a_p1.z(),1.0);
            screen_p1 /= screen_p1.w();
            Point2f loc_p0(screen_p0.x(),screen_p0.y());
            Point2f loc_p1(screen_p1.x(),screen_p1.y());
            
            Vector4f screen_n0 = frameInfo.mvpMat * Vector4f(a_n0.x(),a_n0.y(),a_n0.z(),0.0);
            Point2f loc_n0(screen_n0.x(),screen_n0.y());

            float t0 = a_c0 * u_real_w2;
            Vector2f calcOff = (loc_p1-loc_p0) * t0 + loc_n0 * u_w2;
            Point2f finalPos2f = loc_p0 + calcOff;
            
            NSLog(@"t0 = %f",t0);
            NSLog(@"finalPos = (%f,%f)",finalPos2f.x(),finalPos2f.y());
        }
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
"uniform float u_fade;\n"
"uniform float u_w2;\n"
"uniform float u_real_w2;\n"
"uniform float u_texScale;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec2 a_texCoord0;\n"
"attribute vec4 a_color;\n"
"attribute vec3 a_p1;\n"
"attribute vec2 a_t0_limit;\n"
"attribute vec3 a_n0;\n"
"attribute float a_c0;\n"
"\n"
"varying vec2 v_texCoord;\n"
"varying vec4 v_color;\n"
"\n"
"void main()\n"
"{\n"
"   v_texCoord = vec2(a_texCoord0.x, a_texCoord0.y * u_texScale);\n"
"   v_color = a_color;\n"
//  Position along the line
"   float t0 = a_c0 * u_real_w2;\n"
//"   t0 = clamp(t0,a_t0_limit.x,a_t0_limit.y);\n"
"   t0 = clamp(t0,0.0,1.0);\n"
"   vec3 realPos = (a_p1 - a_position) * t0 + a_n0 * u_real_w2 + a_position;\n"
"   vec4 screenPos = u_mvpMatrix * vec4(realPos,1.0);\n"
"   screenPos /= screenPos.w;\n"
"   gl_Position = vec4(screenPos.xy,0,1.0);\n"
"}\n"
;

static const char *fragmentShaderTri =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform bool  u_hasTexture;\n"
"\n"
"varying vec2      v_texCoord;\n"
"varying vec4      v_color;\n"
"\n"
"void main()\n"
"{\n"
"  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0);\n"
"  gl_FragColor = v_color * baseColor;\n"
"}\n"
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
