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


namespace WhirlyKitShader
{
    
#define WKSVertexPositionAttribute 0
#define WKSVertexColorAttribute 1
#define WKSVertexNormalAttribute 2
#define WKSVertexTextureBaseAttribute 3
// A maximum of two texture coordinates at the moment
#define WKSVertexTextureCoordMax 2
    
// Where we start with basic textures
#define WKSTextureEntryBase 0
    
// Where we start with data lookup texture (like color ramps)
#define WKSTextureEntryLookup 4
    
#define WKSUniformBuffer 8
// Uniforms for the basic case.  Nothing fancy.
struct Uniforms
{
    simd::float4x4 mvpMatrix;
    simd::float4x4 mvMatrix;
    simd::float4x4 mvNormalMatrix;
};

#define WKSUniformDrawStateBuffer 9
// Things that change per drawable (like fade)
struct UniformDrawStateA {
    int numTextures;           // Number of textures we may find on input
    float fade;                // Fade tends to change by time
    float interp;              // Used to interpolate between two textures (if appropriate)
    simd::float2 screenOrigin; // Used for texture pinning in screen space
    simd::float4x4 singleMat;  // Note: Use this rather than changing the uniforms
    bool clipCoords;           // If set, the geometry coordinates aren't meant to be transformed
};

//// Lighting support //////

// A single light
struct Light {
    simd::float3 direction;
    simd::float3 halfPlane;
    simd::float4 ambient;
    simd::float4 diffuse;
    simd::float4 specular;
    float viewDepend;
};

// Material definition
struct Material {
    simd::float4 ambient;
    simd::float4 diffuse;
    simd::float4 specular;
    float specularExponent;
};

#define WKSLightingBuffer 10
// Lighting together in one struct
struct Lighting {
    Material mat;
    int numLights;
    Light lights[8];
};
    
#define WKSTexIndirectStartBuffer 12
// Texture lookup indirection
// Used for treating one textures coordinates as coordinates in the parent
struct TexIndirect {
    simd::float2 offset;
    simd::float2 scale;
};
    
// Instructions to the screen space renders, usually per-drawable
struct ScreenSpaceUniforms {
    float w2;       // Width / 2.0 in screen space
    float real_w2;  // Width/2 in real coordinates
    float edge;     // Edge falloff control
    simd::float2 texScale;  // Texture scaling specific to wide vectors
    simd::float4 color;  // Color override.  TODO: Use the standard one.  Seriouslly.
};

}
