/*  AtmosphereShaders.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 2/2/22.
 *  Copyright 2022 mousebird consulting.  All rights reserved.
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

#ifndef AtmosphereShaders_h
#define AtmosphereShaders_h

namespace WhirlyKitAtmosphereShader
{

// Entries in the free form argument buffers
// We start after 400 from the default shaders
enum AtmosArgBufferEntries {
    AtmosUniformVertEntry = 501,
    AtmosUniformFragEntry = 502,
};


// Uniforms passed into the shaders
struct AtmosShaderVertUniforms
{
    float discardZero;    // Zero is a discard value
    simd_float2 minMax;   // Data value min/max
    int component;        // Which component we're pulling from

    simd::float4x4 mvpMatrix;   // "uniform mat4  u_mvpMatrix;\n"+
    simd::float3 cameraPos;     // "uniform vec3 u_v3CameraPos;\n"+
    float cameraHeight;         // "uniform float u_fCameraHeight;\n"+
    float cameraHeight2;        // "uniform float u_fCameraHeight2;\n"+
    simd::float3 lightPos;      // "uniform vec3 u_v3LightPos;\n"+

    float innerRadius;          // "uniform float u_fInnerRadius;\n"+
    float innerRadius2;         // "uniform float u_fInnerRadius2;\n"+
    float outerRadius;          // "uniform float u_fOuterRadius;\n"+
    float outerRadius2;         // "uniform float u_fOuterRadius2;\n"+
    float scale;                // "uniform float u_fScale;\n"+
    float scaleDepth;           // "uniform float u_fScaleDepth;\n"+
    float scaleOverScaleDepth;  // "uniform float u_fScaleOverScaleDepth;\n"+

    float kr;                   // "uniform float u_Kr;\n"+
    float kr4PI;                // "uniform float u_Kr4PI;\n"+
    float km;                   // "uniform float u_Km;\n"+
    float km4PI;                // "uniform float u_Km4PI;\n"+
    float eSun;                 // "uniform float u_ESun;\n"+
    float kmESun;               // "uniform float u_KmESun;\n"+
    float krESun;               // "uniform float u_KrESun;\n"+
    simd::float3 invWavelength; // "uniform vec3 u_v3InvWavelength ;\n"+
    float samples;              // "uniform float u_fSamples;\n"+
    int nSamples;               // "uniform int u_nSamples;\n"+
};

struct AtmosShaderFragUniforms
{
    float g;
    float g2;
    float exposure;
    simd::float3 lightPos;
};

}

#endif /* AtmosphereShaders_h */

