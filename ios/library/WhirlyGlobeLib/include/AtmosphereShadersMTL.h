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
enum AtmosArgBufferEntries
{
    AtmosUniformVertEntry = 501,
    AtmosUniformFragEntry = 502,
};


// Uniforms passed into the shaders
struct AtmosShaderVertUniforms
{
    simd::float3 lightPos;      // sun position (todo: use scene light info)
    float cameraHeight;
    float innerRadius;          // The inner (planetary) radius
    float outerRadius;          // The outer (atmosphere) radius
    float scale;                // 1 / (outerRadius - innerRadius)
    float scaleDepth;           // The altitude at which the atmosphere's average density is found
    float scaleOverScaleDepth;
    float c;                    // cameraHeight^2 - outerRadius^2
    float kr;                   // scattering parameters
    float kr4PI;
    float km;
    float km4PI;
    float eSun;
    float kmESun;
    float krESun;
    simd::float3 invWavelength; // 1 / pow(wavelength, 4) for RGB
    int samples;                // Number of samples to take between entry and exit points
};

struct AtmosShaderFragUniforms
{
    simd::float3 lightPos;      // sun position (todo: use scene light info)
    float g;                    // ?
    float g2;
    float exposure;
};

}

#endif /* AtmosphereShaders_h */

