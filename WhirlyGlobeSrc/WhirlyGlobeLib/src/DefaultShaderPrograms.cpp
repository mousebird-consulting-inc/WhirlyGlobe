/*
 *  DeafultShaderPrograms.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/29/13.
 *  Copyright 2011-2016 mousebird consulting
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

#import "DefaultShaderPrograms.h"
#import "BillboardDrawable.h"
#import "ScreenSpaceDrawable.h"
#import "ParticleSystemDrawable.h"
#import "WideVectorDrawable.h"
#import "GlobeScene.h"
#import "Drawable.h"

namespace WhirlyKit
{

static const char *vertexShaderTri =
"struct directional_light {"
"  vec3 direction;"
"  vec3 halfplane;"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float viewdepend;"
"};"
""
"struct material_properties {"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float specular_exponent;"
"};"
""
"uniform mat4  u_mvpMatrix;"
"uniform float u_fade;"
"uniform int u_numLights;"
"uniform directional_light light[8];"
"uniform material_properties material;"
""
"attribute vec3 a_position;"
"attribute vec2 a_texCoord0;"
"attribute vec4 a_color;"
"attribute vec3 a_normal;"
""
"varying vec2 v_texCoord;"
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_texCoord = a_texCoord0;"
"   v_color = vec4(0.0,0.0,0.0,0.0);"
"   if (u_numLights > 0)"
"   {"
"     vec4 ambient = vec4(0.0,0.0,0.0,0.0);"
"     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);"
"     for (int ii=0;ii<8;ii++)"
"     {"
"        if (ii>=u_numLights)"
"           break;"
"        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;"
"        float ndotl;"
//"        float ndoth;\n"
"        ndotl = max(0.0, dot(adjNorm, light[ii].direction));"
//"        ndotl = pow(ndotl,0.5);\n"
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n"
"        ambient += light[ii].ambient;"
"        diffuse += ndotl * light[ii].diffuse;"
"     }"
"     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;"
"   } else {"
"     v_color = a_color * u_fade;"
"   }"
""
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);"
"}"
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
//"  if (baseColor.a < 0.1)                            \n"
//"      discard;                                      \n"
"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;

static const char *fragmentShaderRampTri =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform sampler2D s_colorRamp;\n"
"uniform bool  u_hasTexture;\n"
"\n"
"varying vec2      v_texCoord;\n"
"varying vec4      v_color;\n"
"\n"
"void main()\n"
"{\n"
"  float index = texture2D(s_baseMap0, v_texCoord).a;\n"
"  vec4 baseColor = texture2D(s_colorRamp,vec2(0.5,index));\n"
"  gl_FragColor = v_color * baseColor;\n"
"}\n"
;

static const char *vertexShaderModelTri =
"struct directional_light {"
"  vec3 direction;"
"  vec3 halfplane;"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float viewdepend;"
"};"
""
"struct material_properties {"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float specular_exponent;"
"};"
""
"uniform mat4  u_mvpMatrix;"
"uniform float u_fade;"
"uniform float u_time;"
"uniform int u_numLights;"
"uniform directional_light light[8];"
"uniform material_properties material;"
""
"attribute vec3 a_position;"
"attribute vec2 a_texCoord0;"
"attribute vec4 a_color;"
"attribute vec4 a_instanceColor;"
"attribute float a_useInstanceColor;"
"attribute vec3 a_normal;"
"attribute mat4 a_singleMatrix;"
"attribute vec3 a_modelCenter;"
"attribute vec3 a_modelDir;"
""
"varying vec2 v_texCoord;"
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_texCoord = a_texCoord0;"
"   v_color = vec4(0.0,0.0,0.0,0.0);"
"   vec4 inColor = a_useInstanceColor > 0.0 ? a_instanceColor : a_color;"
"   if (u_numLights > 0)"
"   {"
"     vec4 ambient = vec4(0.0,0.0,0.0,0.0);"
"     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);"
"     for (int ii=0;ii<8;ii++)"
"     {"
"        if (ii>=u_numLights)"
"           break;"
"        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;"
"        float ndotl;"
//"        float ndoth;\n"
"        ndotl = max(0.0, dot(adjNorm, light[ii].direction));"
//"        ndotl = pow(ndotl,0.5);\n"
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n"
"        ambient += light[ii].ambient;"
"        diffuse += ndotl * light[ii].diffuse;"
"     }"
"     v_color = vec4(ambient.xyz * material.ambient.xyz * inColor.xyz + diffuse.xyz * inColor.xyz,inColor.a) * u_fade;"
"   } else {"
"     v_color = inColor * u_fade;"
"   }"
"   vec3 center = a_modelDir * u_time + a_modelCenter;"
"   vec3 vertPos = (a_singleMatrix *vec4(a_position,1.0)).xyz + center;"
""
"   gl_Position = u_mvpMatrix * vec4(vertPos,1.0);"
"}"
;

static const char *vertexShaderTriMultiTex =
"struct directional_light {\n"
"  vec3 direction;\n"
"  vec3 halfplane;\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float viewdepend;\n"
"};\n"
"\n"
"struct material_properties {\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float specular_exponent;\n"
"};\n"
"\n"
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"uniform int u_numLights;                      \n"
"uniform directional_light light[8];                     \n"
"uniform material_properties material;       \n"
"uniform float u_interp;"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec2 a_texCoord1;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying vec2 v_texCoord0;                    \n"
"varying vec2 v_texCoord1;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord0 = a_texCoord0;                 \n"
"   v_texCoord1 = a_texCoord1;                 \n"
"   v_color = vec4(0.0,0.0,0.0,0.0);         \n"
"   if (u_numLights > 0)                     \n"
"   {\n"
"     vec4 ambient = vec4(0.0,0.0,0.0,0.0);         \n"
"     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);         \n"
"     for (int ii=0;ii<8;ii++)                 \n"
"     {\n"
"        if (ii>=u_numLights)                  \n"
"           break;                             \n"
"        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;\n"
"        float ndotl;\n"
//"        float ndoth;\n"
"        ndotl = max(0.0, dot(adjNorm, light[ii].direction));\n"
//"        ndotl = pow(ndotl,0.5);\n"
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n"
"        ambient += light[ii].ambient;\n"
"        diffuse += ndotl * light[ii].diffuse;\n"
"     }\n"
"     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;\n"
"   } else {\n"
"     v_color = a_color * u_fade;\n"
"   }\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderTriMultiTex =
"precision mediump float;"
""
"uniform sampler2D s_baseMap0;"
"uniform sampler2D s_baseMap1;"
"uniform float u_interp;"
""
"varying vec2      v_texCoord0;"
"varying vec2      v_texCoord1;"
"varying vec4      v_color;"
""
"void main()"
"{"
"  vec4 baseColor0 = texture2D(s_baseMap0, v_texCoord0);"
"  vec4 baseColor1 = texture2D(s_baseMap1, v_texCoord1);"
"  gl_FragColor = v_color * mix(baseColor0,baseColor1,u_interp);"
"}"
;

static const char *fragmentShaderTriMultiTexRamp =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform sampler2D s_baseMap1;\n"
"uniform sampler2D s_colorRamp;\n"
"uniform float u_interp;\n"
"\n"
"varying vec2      v_texCoord0;\n"
"varying vec2      v_texCoord1;\n"
"varying vec4      v_color;\n"
"\n"
"void main()\n"
"{\n"
"  float baseVal0 = texture2D(s_baseMap0, v_texCoord0).a;\n"
"  float baseVal1 = texture2D(s_baseMap1, v_texCoord1).a;\n"
"  float index = mix(baseVal0,baseVal1,u_interp);\n"
"  gl_FragColor = v_color * texture2D(s_colorRamp,vec2(0.5,index));\n"
"}\n"
;

static const char *vertexShaderScreenTexTri =
"struct directional_light {"
"  vec3 direction;"
"  vec3 halfplane;"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float viewdepend;"
"};"
""
"struct material_properties {"
"  vec4 ambient;"
"  vec4 diffuse;"
"  vec4 specular;"
"  float specular_exponent;"
"};"
""
"uniform mat4  u_mvpMatrix;"
"uniform float u_fade;"
"uniform vec2  u_scale;"
"uniform vec2  u_texScale;"
"uniform vec2  u_screenOrigin;"
"uniform int u_numLights;"
"uniform directional_light light[8];"
"uniform material_properties material;"
""
"attribute vec3 a_position;"
"attribute vec2 a_texCoord0;"
"attribute vec4 a_color;"
"attribute vec3 a_normal;"
"attribute mat4 a_singleMatrix;"
""
"varying vec2 v_texCoord;"
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_texCoord = a_texCoord0;"
"   v_color = vec4(0.0,0.0,0.0,0.0);"
"   if (u_numLights > 0)"
"   {"
"     vec4 ambient = vec4(0.0,0.0,0.0,0.0);"
"     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);"
"     for (int ii=0;ii<8;ii++)"
"     {"
"        if (ii>=u_numLights)"
"           break;"
"        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;"
"        float ndotl;"
//"        float ndoth;\n"
"        ndotl = max(0.0, dot(adjNorm, light[ii].direction));"
//"        ndotl = pow(ndotl,0.5);\n"
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n"
"        ambient += light[ii].ambient;"
"        diffuse += ndotl * light[ii].diffuse;"
"     }"
"     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;"
"   } else {"
"     v_color = a_color * u_fade;"
"   }"
""
"   vec4 screenPt = (u_mvpMatrix * vec4(a_position,1.0));"
"   screenPt /= screenPt.w;"
"   v_texCoord = vec2((screenPt.x+u_screenOrigin.x)*u_scale.x*u_texScale.x,(screenPt.y+u_screenOrigin.y)*u_scale.y*u_texScale.y);"
""
"   gl_Position = u_mvpMatrix * (a_singleMatrix * vec4(a_position,1.0));"
"}"
;


static const char *vertexShaderTriNightDay =
"struct directional_light {\n"
"  vec3 direction;\n"
"  vec3 halfplane;\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float viewdepend;\n"
"};\n"
"\n"
"struct material_properties {\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float specular_exponent;\n"
"};\n"
"\n"
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"uniform int u_numLights;                      \n"
"uniform directional_light light[1];                     \n"
"uniform material_properties material;       \n"
"uniform float u_interp;"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec2 a_texCoord1;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying mediump vec2 v_texCoord0;                    \n"
"varying mediump vec2 v_texCoord1;                    \n"
"varying mediump vec4 v_color;\n"
"varying mediump vec3 v_adjNorm;\n"
"varying mediump vec3 v_lightDir;\n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord0 = a_texCoord0;                 \n"
"   v_texCoord1 = a_texCoord1;                 \n"
"   v_color = a_color;\n"
"   v_adjNorm = light[0].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;\n"
"   v_lightDir = (u_numLights > 0) ? light[0].direction : vec3(1,0,0);\n"
"   v_color = vec4(light[0].ambient.xyz * material.ambient.xyz * a_color.xyz + light[0].diffuse.xyz * a_color.xyz,a_color.a) * u_fade;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderTriNightDay =
"precision mediump float;"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform sampler2D s_baseMap1;\n"
"\n"
"varying vec2      v_texCoord0;\n"
"varying vec2      v_texCoord1;\n"
"varying vec4      v_color;\n"
"varying vec3      v_adjNorm;\n"
"varying vec3      v_lightDir;\n"
"\n"
"void main()\n"
"{\n"
"   float ndotl = max(0.0, dot(v_adjNorm, v_lightDir));\n"
"   ndotl = pow(ndotl,0.5);\n"
"\n"
// Note: Put the color back
"  vec4 baseColor0 = texture2D(s_baseMap0, v_texCoord0);\n"
"  vec4 baseColor1 = texture2D(s_baseMap1, v_texCoord1);\n"
"  gl_FragColor = mix(baseColor0,baseColor1,1.0-ndotl);\n"
"}\n"
;


static const char *vertexShaderLine =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_fade;"
""
"attribute vec3 a_position;"
"attribute vec4 a_color;"
"attribute vec3 a_normal;"
""
"varying vec4      v_color;"
"varying float      v_dot;"
""
"void main()"
"{"
"   vec4 pt = u_mvMatrix * vec4(a_position,1.0);"
"   pt /= pt.w;"
"   vec4 testNorm = u_mvNormalMatrix * vec4(a_normal,0.0);"
"   v_dot = dot(-pt.xyz,testNorm.xyz);"
"   v_color = a_color * u_fade;"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);"
"}"
;

static const char *fragmentShaderLine =
"precision mediump float;"
""
"varying vec4      v_color;"
"varying float      v_dot;"
""
"void main()"
"{"
"  gl_FragColor = (v_dot > 0.0 ? v_color : vec4(0.0,0.0,0.0,0.0));"
"}"
;

static const char *vertexShaderLineNoBack =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_fade;"
""
"attribute vec3 a_position;"
"attribute vec4 a_color;"
"attribute vec3 a_normal;"
""
"varying vec4      v_color;"
""
"void main()"
"{"
"   v_color = a_color * u_fade;"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);"
"}"
;

static const char *fragmentShaderLineNoBack =
"precision mediump float;"
""
"varying vec4      v_color;"
""
"void main()"
"{"
"  gl_FragColor = v_color;"
"}"
;

static const char *vertexShaderNoLightTri =
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord0;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord0;                 \n"
"   v_color = a_color * u_fade;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderNoLightTri =
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
//"  if (baseColor.a < 0.1)                            \n"
//"      discard;                                      \n"
    "  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;
    
void SetupDefaultShaders(Scene *scene)
{
    // Default triangle and line (point) shaders
    OpenGLES2Program *triShader = new OpenGLES2Program("Default triangle shader with lighting",vertexShaderTri,fragmentShaderTri);
    OpenGLES2Program *lineShader = new OpenGLES2Program("Default line shader with backface culling",vertexShaderLine,fragmentShaderLine);
    if (!triShader->isValid() || !lineShader->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Default triangle and line shaders didn't compile.  Nothing will work.\n");
        delete triShader;
        delete lineShader;
    } else {
        scene->addProgram(triShader);
        scene->addProgram(lineShader);
        scene->setSceneProgram(kToolkitDefaultTriangleProgram, triShader->getId());
        scene->setSceneProgram(kSceneDefaultTriShader, triShader->getId());
        scene->setSceneProgram(kToolkitDefaultLineProgram, lineShader->getId());
        scene->setSceneProgram(kSceneDefaultLineShader, lineShader->getId());
    }
    
    // Line shader that doesn't do backface culling
    OpenGLES2Program *lineNoBackShader = new OpenGLES2Program("Default line shader without backface culling",vertexShaderLineNoBack,fragmentShaderLineNoBack);
    if (!lineNoBackShader->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Line shader without backface checking didn't compile.\n");
        delete lineNoBackShader;
    } else {
        scene->addProgram(kToolkitDefaultLineNoBackfaceProgram, lineNoBackShader);
    }
    
    // Triangle shader that doesn't do lighting
    OpenGLES2Program *triShaderNoLight = new OpenGLES2Program("Default triangle shader without lighting",vertexShaderNoLightTri,fragmentShaderNoLightTri);
    if (!triShaderNoLight->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader without lighting didn't compile.\n");
        delete triShaderNoLight;
    } else {
        scene->addProgram(kToolkitDefaultTriangleNoLightingProgram, triShaderNoLight);
    }
    
    // Triangle shader the model instancing
    OpenGLES2Program *triShaderModel = new OpenGLES2Program("Triangle shader for models with lighting",vertexShaderModelTri,fragmentShaderTri);
    if (!triShaderModel->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader for model instancing and lighting didn't compile.");
        delete triShaderModel;
    } else {
        scene->addProgram(kToolkitDefaultTriangleModel, triShaderModel);
    }
    
    // Triangle shader that does screen space texture application
    OpenGLES2Program *triShaderScreenTex = new OpenGLES2Program("Triangle shader with screen texture and lighting",vertexShaderScreenTexTri,fragmentShaderTri);
    if (!triShaderScreenTex->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader with screen texture and lighting didn't compile.");
        delete triShaderScreenTex;
    } else {
        scene->addProgram(kToolkitDefaultTriangleScreenTex, triShaderScreenTex);
    }
    
    // Triangle shader that handles multiple textures
    OpenGLES2Program *triShaderMultiTex = new OpenGLES2Program("Triangle shader with multitex and lighting",vertexShaderTriMultiTex,fragmentShaderTriMultiTex);
    if (!triShaderMultiTex->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader with multi texture support didn't compile.\n");
        delete triShaderMultiTex;
    } else {
        scene->addProgram(kToolkitDefaultTriangleMultiTex, triShaderMultiTex);
    }
    
    // Triangle shader with ramp texture
    OpenGLES2Program *triShaderMultiTexRamp = new OpenGLES2Program("Triangle ramp shader with multitex and lighting",vertexShaderTriMultiTex,fragmentShaderTriMultiTexRamp);
    if (!triShaderMultiTexRamp->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle ramp shader with multi texture support didn't compile.\n");
        delete triShaderMultiTexRamp;
    } else {
        scene->addProgram(kToolkitDefaultTriangleMultiTexRamp, triShaderMultiTexRamp);
    }

    
    // Triangle shader that does night/day shading with multiple textures
    OpenGLES2Program *triShaderNightDay = new OpenGLES2Program("Triangle shader with multitex, lighting, and night/day support",vertexShaderTriNightDay,fragmentShaderTriNightDay);
    if (!triShaderNightDay->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader with night/day support didn't compile.");
        delete triShaderNightDay;
    } else {
        scene->addProgram(kToolkitDefaultTriangleNightDay, triShaderNightDay);
    }
    
#ifndef MAPLYMINIMAL
    // Billboard shader (ground)
    OpenGLES2Program *billShaderGround = BuildBillboardGroundProgram();
    if (!billShaderGround)
    {
        fprintf(stderr,"SetupDefaultShaders: Billboard ground shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultBillboardGroundProgram, billShaderGround);
    }
    
    // Billboard shader (eye)
    OpenGLES2Program *billShaderEye = BuildBillboardEyeProgram();
    if (!billShaderEye)
    {
        fprintf(stderr,"SetupDefaultShaders: Billboard eye shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultBillboardEyeProgram, billShaderEye);
    }
#endif
    
    // Widened vector shader
    OpenGLES2Program *wideVecShader = BuildWideVectorProgram();
    if (!wideVecShader)
    {
        fprintf(stderr,"SetupDefaultShaders: Wide Vector shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultWideVectorProgram, wideVecShader);
    }
    
    // Widened vector shader (globe version)
    OpenGLES2Program *wideVecGlobeShader = BuildWideVectorGlobeProgram();
    if (!wideVecGlobeShader)
    {
        fprintf(stderr,"SetupDefaultShaders: Wide Vector Globe shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultWideVectorGlobeProgram, wideVecGlobeShader);
    }    
    
    if (dynamic_cast<WhirlyGlobe::GlobeScene *>(scene))
    {
        // Screen space shader
        OpenGLES2Program *screenSpaceShader = BuildScreenSpaceProgram();
        if (!screenSpaceShader)
        {
            fprintf(stderr,"SetupDefaultShaders: Screen Space shader didn't compile.");
        } else {
            scene->addProgram(kToolkitDefaultScreenSpaceProgram, screenSpaceShader);
        }

        // Screen space shader w/ Motion
        OpenGLES2Program *screenSpaceMotionShader = BuildScreenSpaceMotionProgram();
        if (!screenSpaceMotionShader)
        {
            fprintf(stderr,"SetupDefaultShaders: Screen Space Motion shader didn't compile.");
        } else {
            scene->addProgram(kToolkitDefaultScreenSpaceMotionProgram, screenSpaceMotionShader);
        }
    } else {
        // Use the 2D versions, which don't do backface checking

        // Screen space shader
        OpenGLES2Program *screenSpaceShader = BuildScreenSpace2DProgram();
        if (!screenSpaceShader)
        {
            fprintf(stderr,"SetupDefaultShaders: Screen Space shader didn't compile.");
        } else {
            scene->addProgram(kToolkitDefaultScreenSpaceProgram, screenSpaceShader);
        }
        
        // Screen space shader w/ Motion
        OpenGLES2Program *screenSpaceMotionShader = BuildScreenSpaceMotion2DProgram();
        if (!screenSpaceMotionShader)
        {
            fprintf(stderr,"SetupDefaultShaders: Screen Space Motion shader didn't compile.");
        } else {
            scene->addProgram(kToolkitDefaultScreenSpaceMotionProgram, screenSpaceMotionShader);
        }
    }
    
#ifndef MAPLYMINIMAL
    // Particle System program
    OpenGLES2Program *particleSystemShader = BuildParticleSystemProgram();
    if (!particleSystemShader)
    {
        fprintf(stderr,"SetupDefaultShaders: Particle System Shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultParticleSystemProgram, particleSystemShader);
    }
#endif
}

}
