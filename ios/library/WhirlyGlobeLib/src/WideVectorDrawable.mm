/*
 *  WideVectorDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/29/14.
 *  Copyright 2011-2017 mousebird consulting
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
    
WideVectorDrawable::WideVectorDrawable(const std::string &inName,unsigned int numVert,unsigned int numTri,bool globeMode)
 : BasicDrawable(), texRepeat(1.0), edgeSize(1.0), realWidthSet(false), globeMode(globeMode)
{
    name = inName;
    basicDrawableInit();
    
    points.reserve(numVert);
    tris.reserve(numTri);
    lineWidth = 10.0/1024.0;
    if (globeMode)
        normalEntry = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_normal"),numVert);
    p1_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_p1"),numVert);
    tex_index = addAttribute(BDFloat4Type, StringIndexer::getStringID("a_texinfo"),numVert);
    n0_index = addAttribute(BDFloat3Type, StringIndexer::getStringID("a_n0"),numVert);
    c0_index = addAttribute(BDFloatType, StringIndexer::getStringID("a_c0"),numVert);
}
 
// Not.  Do not want standard attributes.
void WideVectorDrawable::setupStandardAttributes(int numReserve)
{
}
    
unsigned int WideVectorDrawable::addPoint(const Point3f &pt)
{
#ifdef WIDEVECDEBUG
    locPts.push_back(pt);
#endif
    return BasicDrawable::addPoint(pt);
}
    
void WideVectorDrawable::addNormal(const Point3f &norm)
{
    if (globeMode)
    {
        BasicDrawable::addNormal(norm);
    }
}

void WideVectorDrawable::addNormal(const Point3d &norm)
{
    if (globeMode)
    {
        BasicDrawable::addNormal(norm);
    }
}

void WideVectorDrawable::add_p1(const Point3f &pt)
{
    addAttributeValue(p1_index, pt);
#ifdef WIDEVECDEBUG
    p1.push_back(pt);
#endif
}

void WideVectorDrawable::add_texInfo(float texX,float texYmin,float texYmax,float texOffset)
{
    addAttributeValue(tex_index, Vector4f(texX,texYmin,texYmax,texOffset));
#ifdef WIDEVECDEBUG
#endif
}

void WideVectorDrawable::add_n0(const Point3f &dir)
{
    addAttributeValue(n0_index, dir);
#ifdef WIDEVECDEBUG
    n0.push_back(dir);
#endif
}

void WideVectorDrawable::add_c0(float val)
{
    addAttributeValue(c0_index, val);
#ifdef WIDEVECDEBUG
    c0.push_back(val);
#endif
}

void WideVectorDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo, Scene *scene)
{
    if (frameInfo.program)
    {
        float scale = std::max(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight);
        float screenSize = frameInfo.screenSizeInDisplayCoords.x();
        float pixDispSize = std::min(frameInfo.screenSizeInDisplayCoords.x(),frameInfo.screenSizeInDisplayCoords.y()) / scale;
        if (realWidthSet)
        {
            frameInfo.program->setUniform(u_w2NameID, (float)(realWidth / pixDispSize));
            frameInfo.program->setUniform(u_Realw2NameID, (float)realWidth);
            frameInfo.program->setUniform(u_EdgeNameID, edgeSize);
        } else {
            frameInfo.program->setUniform(u_w2NameID, lineWidth);
            frameInfo.program->setUniform(u_Realw2NameID, pixDispSize * lineWidth);
            frameInfo.program->setUniform(u_EdgeNameID, edgeSize);
        }
        float texScale = scale/(screenSize*texRepeat);
        frameInfo.program->setUniform(u_texScaleNameID, texScale);
        frameInfo.program->setUniform(u_colorNameID, Vector4f(color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0));
        
        // Note: This calculation is out of date with respect to the shader
        // Redo the calculation for debugging
//        NSLog(@"\n");
//        for (unsigned int ii=0;ii<locPts.size();ii++)
//        {
//            float u_w2 = lineWidth/(2.f*scale);
//            float u_real_w2 = pixDispSize * lineWidth;
//            Point3f a_p0 = locPts[ii];
//            Point3f a_p1 = p1[ii];
//            Point2f a_t0_limit = t0_limits[ii];
//            Point3f a_n0 = n0[ii];
//            float a_c0 = c0[ii];
//            
//            Vector4f screen_p0 = frameInfo.mvpMat * Vector4f(a_p0.x(),a_p0.y(),a_p0.z(),1.0);
//            screen_p0 /= screen_p0.w();
//            Vector4f screen_p1 = frameInfo.mvpMat * Vector4f(a_p1.x(),a_p1.y(),a_p1.z(),1.0);
//            screen_p1 /= screen_p1.w();
//            Point2f loc_p0(screen_p0.x(),screen_p0.y());
//            Point2f loc_p1(screen_p1.x(),screen_p1.y());
//            
//            Vector4f screen_n0 = frameInfo.mvpMat * Vector4f(a_n0.x(),a_n0.y(),a_n0.z(),0.0);
//            Point2f loc_n0(screen_n0.x(),screen_n0.y());
//
//            float t0 = a_c0 * u_real_w2;
//            Vector2f calcOff = (loc_p1-loc_p0) * t0 + loc_n0 * u_w2;
//            Point2f finalPos2f = loc_p0 + calcOff;
//            
//            NSLog(@"t0 = %f",t0);
//            NSLog(@"finalPos = (%f,%f)",finalPos2f.x(),finalPos2f.y());
//        }
    }
    
    BasicDrawable::draw(frameInfo,scene);
    
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
//"attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;

varying vec2 v_texCoord;
//"varying vec4 v_color;

void main()
{
//"   v_color = a_color;\n"
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
//"attribute vec4 a_color;
attribute vec3 a_p1;
attribute vec3 a_n0;
attribute float a_c0;

varying vec2 v_texCoord;
//"varying vec4 v_color;
varying float      v_dot;

void main()
{
//"   v_color = a_color;
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

WhirlyKit::OpenGLES2Program *BuildWideVectorProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kWideVectorShaderName,vertexShaderTri,fragmentShaderTriAlias);
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

WhirlyKit::OpenGLES2Program *BuildWideVectorGlobeProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kWideVectorGlobeShaderName,vertexGlobeShaderTri,fragmentGlobeShaderTriAlias);
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
