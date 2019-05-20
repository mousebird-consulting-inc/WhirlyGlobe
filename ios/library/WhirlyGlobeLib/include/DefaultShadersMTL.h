/*
 *  DefaultShadersMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

namespace WhirlyKit
{

// Uniforms for the basic case.  Nothing fancy.
struct UniformsA
{
    metal::float4x4 mvpMatrix;
    metal::float4x4 mvMatrix;
    metal::float4x4 mvNormalMatrix;
    float fade;
};
    
//// Lighting support //////

// A single light
struct Light {
    metal::float3 direction;
    metal::float3 halfPlane;
    metal::float4 ambient;
    metal::float4 diffuse;
    metal::float4 specular;
    float viewDepend;
};

// Material definition
struct Material {
    metal::float4 ambient;
    metal::float4 diffuse;
    metal::float4 specular;
    float specularExponent;
};

// Lighting together in one struct
struct Lighting {
    int numLights;
    Light lights[8];
    Material mat;
};

// General purpose uniforms for these shaders
struct UniformsTri {
    metal::float4x4 mvpMatrix;
    metal::float4x4 mvMatrix;
    metal::float4x4 mvNormalMatrix;
    float fade;
};

// Texture lookup indirection
// Used for treating one textures coordinates as coordinates in the parent
struct TexIndirect {
    metal::float2 offset;
    metal::float2 scale;
};

}
