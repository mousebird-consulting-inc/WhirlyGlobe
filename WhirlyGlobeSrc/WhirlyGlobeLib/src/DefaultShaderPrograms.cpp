/*
 *  DeafultShaderPrograms.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/29/13.
 *  Copyright 2011-2013 mousebird consulting
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
// Note: Porting
//#import "BillboardDrawable.h"
#import "ScreenSpaceDrawable.h"

namespace WhirlyKit
{

static const char *vertexShaderTri =
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
"\n"
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
// Note: Porting  Having some trouble with random colors showing up
"  gl_FragColor = vec4(1.0,1.0,1.0,1.0) * baseColor;"
//"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;

/** The vertex shader for screen space drawables
    a_position: Position in 3-space for the center
    a_offset:   Offset from the location
  */
//static const char *vertexShaderScreenSpace =
//"uniform mat4  u_mvMatrix;"
//"uniform mat4  u_mvpMatrix;"
//"uniform mat4  u_mvNormalMatrix;"
//"uniform float u_fade;"
//"attribute vec3 a_position;"
//"attribute vec2 a_offset;"
//"attribute vec3 a_normal;"
//"attribute vec2 a_texCoord0;"
//"attribute vec4 a_color;"
//""
//"varying vec2 v_texCoord0;"
//"varying vec4 v_color;"
//""
//"void main()"
//"{"
//"   v_texCoord0 = a_texCoord0;"
//"   v_color = a_color * u_fade;"
//"   vec4 testPt = u_mvMatrix * vec4(a_position,1.0);"
//"   vec4 testDir = u_mvNormalMatrix * vec4(a_normal,0.0);"
//"   float res = dot(vec3(-testPt.x/testPt.w,-testPt.y/testPt.w,-testPt.z/testPt.w),vec3(testDir.x,testDir.y,testDir.z));"
//"   gl_Position = (u_mvpMatrix * vec4(a_position,1.0) + vec4(a_offset,0.0,0.0)) * step(0.0,res);"
//"}"
//;

//static const char *fragmentShaderScreenSpace =
//"precision mediump float;"
//""
//"uniform sampler2D s_baseMap0;"
//""
//"varying vec2      v_texCoord0;"
//"varying vec4      v_color;"
//""
//"void main()"
//"{"
//"  vec4 baseColor0 = texture2D(s_baseMap0, v_texCoord0);"
//"  gl_FragColor = v_color * baseColor0;"
//"}"
//;

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
    
    // Triangle shader that handles multiple textures
    OpenGLES2Program *triShaderMultiTex = new OpenGLES2Program("Triangle shader with multitex and lighting",vertexShaderTriMultiTex,fragmentShaderTriMultiTex);
    if (!triShaderMultiTex->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader with multi texture support didn't compile.\n");
        delete triShaderMultiTex;
    } else {
        scene->addProgram(kToolkitDefaultTriangleMultiTex, triShaderMultiTex);
    }
    
    // Shader for screen space objects
    OpenGLES2Program *screenShader = new OpenGLES2Program("Triangle shader for screen space objects",vertexShaderScreenSpace,fragmentShaderScreenSpace);
    if (!screenShader->isValid())
    {
        fprintf(stderr,"SetupDefaultShaders: Triangle shader for screen space objects didn't compile.");
        delete screenShader;
    } else {
        scene->addProgram(kToolkitDefaultScreenSpaceProgram, screenShader);
    }

    
    // Note: Porting
//    OpenGLES2Program *billShader = BuildBillboardProgram();
//    if (!billShader)
//    {
//        fprintf(stderr,"SetupDefaultShaders: Billboard shader didn't compiled.");
//    } else {
//        scene->addProgram(kToolkitDefaultBillboardProgram, billShader);
//    }

    // Screen space shader
    OpenGLES2Program *screenSpaceShader = BuildScreenSpaceProgram();
    if (!screenSpaceShader)
    {
        NSLog(@"SetupDefaultShaders: Screen Space shader didn't compile.");
    } else {
        scene->addProgram(kToolkitDefaultScreenSpaceProgram, screenSpaceShader);
    }

}

}
