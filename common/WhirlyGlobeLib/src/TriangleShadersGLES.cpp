/*
 *  TriangleShaders.cpp
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/21/18.
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

#import "TriangleShadersGLES.h"
#import "ProgramGLES.h"

namespace WhirlyKit
{
    
static const char *vertexShaderTri = R"(
precision highp float;

struct directional_light {
  vec3 direction;
  vec3 halfplane;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float viewdepend;
};

struct material_properties {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_exponent;
};

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform int u_numLights;
uniform directional_light light[8];
uniform material_properties material;

uniform vec2 u_texOffset0;
uniform vec2 u_texScale0;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec3 a_normal;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
   if (u_texScale0.x != 0.0)
     v_texCoord = vec2(a_texCoord0.x*u_texScale0.x,a_texCoord0.y*u_texScale0.y) + u_texOffset0;
   else
     v_texCoord = a_texCoord0;
   v_color = vec4(0.0,0.0,0.0,0.0);
   if (u_numLights > 0)
   {
     vec4 ambient = vec4(0.0,0.0,0.0,0.0);
     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);
     for (int ii=0;ii<8;ii++)
     {
        if (ii>=u_numLights)
           break;
        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;
        float ndotl;
//"        float ndoth;
        ndotl = max(0.0, dot(adjNorm, light[ii].direction));
//"        ndotl = pow(ndotl,0.5);
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));
        ambient += light[ii].ambient;
        diffuse += ndotl * light[ii].diffuse;
     }
     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;
   } else {
     v_color = a_color * u_fade;
   }

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
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
//"  vec4 baseColor = texture2D(s_baseMap0, v_texCoord);"
  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0);
//"  if (baseColor.a < 0.1)
//"      discard;
  gl_FragColor = v_color * baseColor;
}
)";
    
// Triangle shader with lighting
ProgramGLES *BuildDefaultTriShaderLightingGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

static const char *vertexShaderNoLightTri = R"(
precision highp float;
    
uniform mat4  u_mvpMatrix;
uniform float u_fade;
attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec3 a_normal;

uniform vec2 u_texOffset0;
uniform vec2 u_texScale0;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
    if (u_texScale0.x != 0.0)
        v_texCoord = vec2(a_texCoord0.x*u_texScale0.x,a_texCoord0.y*u_texScale0.y) + u_texOffset0;
    else
        v_texCoord = a_texCoord0;
   v_color = a_color * u_fade;

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
}
)";

static const char *fragmentShaderNoLightTri = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform bool  u_hasTexture;

varying vec2      v_texCoord;
varying vec4      v_color;

void main()
{
//  vec4 baseColor = texture2D(s_baseMap0, v_texCoord);
  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap0, v_texCoord) : vec4(1.0,1.0,1.0,1.0);
//  if (baseColor.a < 0.1)
//      discard;
  gl_FragColor = v_color * baseColor;
}
)";
    
// Triangle shader without lighting
ProgramGLES *BuildDefaultTriShaderNoLightingGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderNoLightTri,fragmentShaderNoLightTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}
    
static const char *vertexShaderModelTri = R"(
precision highp float;
    
struct directional_light {
  vec3 direction;
  vec3 halfplane;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float viewdepend;
};

struct material_properties {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_exponent;
};

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform float u_time;
uniform int u_numLights;
uniform directional_light light[8];
uniform material_properties material;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec4 a_instanceColor;
attribute float a_useInstanceColor;
attribute vec3 a_normal;
attribute mat4 a_singleMatrix;
attribute vec3 a_modelCenter;
attribute vec3 a_modelDir;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
   v_texCoord = a_texCoord0;
   v_color = vec4(0.0,0.0,0.0,0.0);
   vec4 inColor = a_useInstanceColor > 0.0 ? a_instanceColor : a_color;
   if (u_numLights > 0)
   {
     vec4 ambient = vec4(0.0,0.0,0.0,0.0);
     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);
     for (int ii=0;ii<8;ii++)
     {
        if (ii>=u_numLights)
           break;
        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;
        float ndotl;
//        float ndoth;
        ndotl = max(0.0, dot(adjNorm, light[ii].direction));
//        ndotl = pow(ndotl,0.5);
//        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));
        ambient += light[ii].ambient;
        diffuse += ndotl * light[ii].diffuse;
     }
     v_color = vec4(ambient.xyz * material.ambient.xyz * inColor.xyz + diffuse.xyz * inColor.xyz,inColor.a) * u_fade;
   } else {
     v_color = inColor * u_fade;
   }
   vec3 center = a_modelDir * u_time + a_modelCenter;
   vec3 vertPos = (a_singleMatrix * vec4(a_position,1.0)).xyz + center;

   gl_Position = u_mvpMatrix * vec4(vertPos,1.0);
}
)";

// Triangle shader for models
ProgramGLES *BuildDefaultTriShaderModelGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderModelTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

static const char *vertexShaderScreenTexTri = R"(
precision highp float;

struct directional_light {
  vec3 direction;
  vec3 halfplane;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float viewdepend;
};

struct material_properties {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_exponent;
};

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform vec2  u_scale;
uniform vec2  u_texScale;
uniform vec2  u_screenOrigin;
uniform int u_numLights;
uniform directional_light light[8];
uniform material_properties material;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec4 a_color;
attribute vec3 a_normal;
attribute mat4 a_singleMatrix;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
   v_texCoord = a_texCoord0;
   v_color = vec4(0.0,0.0,0.0,0.0);
   if (u_numLights > 0)
   {
     vec4 ambient = vec4(0.0,0.0,0.0,0.0);
     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);
     for (int ii=0;ii<8;ii++)
     {
        if (ii>=u_numLights)
           break;
        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;
        float ndotl;
//        float ndoth;\n
        ndotl = max(0.0, dot(adjNorm, light[ii].direction));
//        ndotl = pow(ndotl,0.5);\n
//        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n
        ambient += light[ii].ambient;
        diffuse += ndotl * light[ii].diffuse;
     }
     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;
   } else {
     v_color = a_color * u_fade;
   }

   vec4 screenPt = (u_mvpMatrix * vec4(a_position,1.0));
   screenPt /= screenPt.w;
   v_texCoord = vec2((screenPt.x+u_screenOrigin.x)*u_scale.x*u_texScale.x,(screenPt.y+u_screenOrigin.y)*u_scale.y*u_texScale.y);

   gl_Position = u_mvpMatrix * (a_singleMatrix * vec4(a_position,1.0));
}
)";
    
// Triangles with screen textures
ProgramGLES *BuildDefaultTriShaderScreenTextureGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderScreenTexTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

static const char *vertexShaderTriMultiTex = R"(
precision highp float;

struct directional_light {
  vec3 direction;
  vec3 halfplane;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float viewdepend;
};

struct material_properties {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_exponent;
};

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform int u_numLights;
uniform directional_light light[8];
uniform material_properties material;
uniform float u_interp;
    
uniform vec2 u_texOffset0;
uniform vec2 u_texScale0;
uniform vec2 u_texOffset1;
uniform vec2 u_texScale1;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec2 a_texCoord1;
attribute vec4 a_color;
attribute vec3 a_normal;

varying vec2 v_texCoord0;
varying vec2 v_texCoord1;
varying vec4 v_color;

void main()
{
    if (u_texScale0.x != 0.0)
        v_texCoord0 = vec2(a_texCoord0.x*u_texScale0.x,a_texCoord0.y*u_texScale0.y) + u_texOffset0;
    else
        v_texCoord0 = a_texCoord0;

    if (u_texScale1.x != 0.0)
        v_texCoord1 = vec2(a_texCoord0.x*u_texScale1.x,a_texCoord0.y*u_texScale1.y) + u_texOffset1;
    else
        v_texCoord1 = a_texCoord0;

    v_color = vec4(0.0,0.0,0.0,0.0);
   if (u_numLights > 0)
   {
     vec4 ambient = vec4(0.0,0.0,0.0,0.0);
     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);
     for (int ii=0;ii<8;ii++)
     {
        if (ii>=u_numLights)
           break;
        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;
        float ndotl;
//        float ndoth;
        ndotl = max(0.0, dot(adjNorm, light[ii].direction));
//        ndotl = pow(ndotl,0.5);
//        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));
        ambient += light[ii].ambient;
        diffuse += ndotl * light[ii].diffuse;
     }
     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a) * u_fade;
   } else {
     v_color = a_color * u_fade;
   }

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
}
)";
    
static const char *fragmentShaderTriMultiTex = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform sampler2D s_baseMap1;
uniform float u_interp;

varying vec2      v_texCoord0;
varying vec2      v_texCoord1;
varying vec4      v_color;

void main()
{
  vec4 baseColor0 = texture2D(s_baseMap0, v_texCoord0);
  vec4 baseColor1 = texture2D(s_baseMap1, v_texCoord1);
  gl_FragColor = v_color * mix(baseColor0,baseColor1,u_interp);
}
)";
    
// Triangles with multiple textures
ProgramGLES *BuildDefaultTriShaderMultitexGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTriMultiTex,fragmentShaderTriMultiTex);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

static const char *fragmentShaderTriMultiTexRamp = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform sampler2D s_baseMap1;
uniform sampler2D s_colorRamp;
uniform float u_interp;

varying vec2      v_texCoord0;
varying vec2      v_texCoord1;
varying vec4      v_color;

void main()
{
  float baseVal0 = texture2D(s_baseMap0, v_texCoord0).r;
  float baseVal1 = texture2D(s_baseMap1, v_texCoord1).r;
  float index = mix(baseVal0,baseVal1,u_interp);
  gl_FragColor = v_color * texture2D(s_colorRamp,vec2(index,0.5));
}
)";

// Triangles that use the ramp textures
ProgramGLES *BuildDefaultTriShaderRamptexGLES(const std::string &name,SceneRenderer *renderer)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTriMultiTex,fragmentShaderTriMultiTexRamp);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

static const char *vertexShaderTriNightDay = R"(
precision highp float;

struct directional_light {
  vec3 direction;
  vec3 halfplane;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float viewdepend;
};

struct material_properties {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float specular_exponent;
};

uniform mat4  u_mvpMatrix;
uniform float u_fade;
uniform int u_numLights;
uniform directional_light light[1];
uniform material_properties material;
uniform float u_interp;

uniform vec2 u_texOffset0;
uniform vec2 u_texScale0;
uniform vec2 u_texOffset1;
uniform vec2 u_texScale1;

attribute vec3 a_position;
attribute vec2 a_texCoord0;
attribute vec2 a_texCoord1;
attribute vec4 a_color;
attribute vec3 a_normal;

varying mediump vec2 v_texCoord0;
varying mediump vec2 v_texCoord1;
varying mediump vec4 v_color;
varying mediump vec3 v_adjNorm;
varying mediump vec3 v_lightDir;

void main()
{
    if (u_texScale0.x != 0.0)
        v_texCoord0 = vec2(a_texCoord0.x*u_texScale0.x,a_texCoord0.y*u_texScale0.y) + u_texOffset0;
    else
        v_texCoord0 = a_texCoord0;
    
    if (u_texScale1.x != 0.0)
        v_texCoord1 = vec2(a_texCoord0.x*u_texScale1.x,a_texCoord0.y*u_texScale1.y) + u_texOffset1;
    else
        v_texCoord1 = a_texCoord0;

   v_color = a_color;
   v_adjNorm = light[0].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;
   v_lightDir = (u_numLights > 0) ? light[0].direction : vec3(1,0,0);
   v_color = vec4(light[0].ambient.xyz * material.ambient.xyz * a_color.xyz + light[0].diffuse.xyz * a_color.xyz,a_color.a) * u_fade;

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
}
)";

static const char *fragmentShaderTriNightDay = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform sampler2D s_baseMap1;

varying vec2      v_texCoord0;
varying vec2      v_texCoord1;
varying vec4      v_color;
varying vec3      v_adjNorm;
varying vec3      v_lightDir;

void main()
{
   float ndotl = max(0.0, dot(v_adjNorm, v_lightDir));
   ndotl = pow(ndotl,0.5);

// Note: Put the color back
  vec4 baseColor0 = texture2D(s_baseMap0, v_texCoord0);
  vec4 baseColor1 = texture2D(s_baseMap1, v_texCoord1);
  gl_FragColor = mix(baseColor0,baseColor1,1.0-ndotl);
}
)";
    
// Day/night support for triangles
ProgramGLES *BuildDefaultTriShaderNightDayGLES(const std::string &name,SceneRenderer *sceneRender)
{
    ProgramGLES *shader = new ProgramGLES(name,vertexShaderTriNightDay,fragmentShaderTriNightDay);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    return shader;
}

}
