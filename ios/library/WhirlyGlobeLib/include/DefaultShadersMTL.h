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
    
// Wide Vector vertex attribute positions
// TODO: Combine these
#define WKSVertexWideVecTexInfoAttribute 6
#define WKSVertexWideVecP1Attribute 7
#define WKSVertexWideVecN0Attribute 8
#define WKSVertexWideVecC0Attribute 9
    
// Screen space vertex attribute positions
// TODO: Combine these
#define WKSVertexScreenSpaceOffsetAttribute 6
#define WKSVertexScreenSpaceRotAttribute 7
#define WKSVertexScreenSpaceDirAttribute 8
    
// Model instance vertex attribute positions
// TODO: Combine these
#define WKSVertexInstanceColorAttribute 6
#define WKSVertexInstanceMatrixAttribute 7
#define WKSVertexInstanceCenterAttribute 8
#define WKSVertexInstanceDirAttribute 9
    
// Billboard offsets
// TODO: Billboards should be instances
#define WKSVertexBillboardOffsetAttribute 6
    
// Where we start with basic textures
#define WKSTextureEntryBase 0
    
// Where we start with data lookup texture (like color ramps)
#define WKSTextureEntryLookup 4
    
#define WKSUniformBuffer 10
// Uniforms for the basic case.  Nothing fancy.
struct Uniforms
{
    simd::float4x4 mvpMatrix;
    simd::float4x4 mvpInvMatrix;
    simd::float4x4 mvMatrix;
    simd::float4x4 mvNormalMatrix;
    simd::float3 eyePos;
    simd::float2 pixDispSize;  // Size of a single pixel in display coordinates
    simd::float2 frameSize;    // Output framebuffer size
    uint frameCount;            // Starts at zero and goes up from there every frame
    int outputTexLevel;        // Normally 0, unless we're running a reduce
    bool globeMode;
};

#define WKSUniformDrawStateBuffer 11
// Things that change per drawable (like fade)
struct UniformDrawStateA {
    int numTextures;           // Number of textures we may find on input
    float fade;                // Fade tends to change by time
    float interp;              // Used to interpolate between two textures (if appropriate)
    simd::float2 screenOrigin; // Used for texture pinning in screen space
    simd::float4x4 singleMat;  // TODO: Use this rather than changing the uniforms
    bool clipCoords;           // If set, the geometry coordinates aren't meant to be transformed
};
    
// Things that change per particle drawable
#define WKSUniformDrawStateParticleBuffer 13
struct UniformDrawStateParticle {
    float pointSize;   // If set, the size of points to be rendered
    float time; // Relative time globally
    float lifetime;  // Total lifetime of a particle
    float frameLen; // Length of a single frame
};

// Input buffer for the particles
#define WKSParticleBuffer 6

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

#define WKSLightingBuffer 12
// Lighting together in one struct
struct Lighting {
    Material mat;
    int numLights;
    Light lights[8];
};
    
#define WKSTexIndirectStartBuffer 13
// Texture lookup indirection
// Used for treating one texture's coordinates as coordinates in the parent
struct TexIndirect {
    simd::float2 offset;
    simd::float2 scale;
};

#define WKSUniformDrawStateWideVecBuffer 15
// Instructions to the wide vector shaders, usually per-drawable
struct UniformWideVec {
    float w2;       // Width / 2.0 in screen space
    float real_w2;  // Width/2 in real coordinates
    float edge;     // Edge falloff control
    float texScale;  // Texture scaling specific to wide vectors
    simd::float4 color;  // Color override.  TODO: Use the standard one.  Seriously.
};
    
#define WKSUniformDrawStateScreenSpaceBuffer 15
// Instructions to the screen space shaders, usually per-drawable
struct UniformScreenSpace {
    simd::float2 scale;
    float time;                // For moving objects, this is the base time
    bool keepUpright;
    bool activeRot;
    bool hasMotion;            // For objects that can move, check this
};
    
#define WKSUniformDrawStateModelInstanceBuffer 15
// Instructions to the model instance shaders, per-drawable
struct UniformModelInstance {
    float time;                // For moving objects, this is the base time
    bool hasMotion;            // For objects that can move, check this
    bool useInstanceColor;     // For model instance, if set use the instance color
};

#define WKSModelInstanceBuffer 16
// Input model instance info
struct VertexTriModelInstance
{
    // Instance stuff
    simd::float4 color;
    simd::float4x4 mat;
    simd::float3 center;
    simd::float3 dir;
};
// If we're using the indirect instancing (can be driven by the GPU) this is
//  where the indirect buffer lives
#define WKSInstanceIndirectBuffer 17

#define WKSUniformDrawStateBillboardBuffer 15
// Instructions to the billboard shaders, per-drawable
struct UniformBillboard {
    simd::float3 eyeVec;
    bool groundMode;
};
    
}

// These have syntax that only the shader language can grok
// We put them here so we can include them in other Metal libraries
#ifdef __METAL_VERSION__
// Vertices with position, color, and normal
struct VertexA
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float4 color [[attribute(WKSVertexColorAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
};

// Position, color, and dot project (for backface checking)
struct ProjVertexA
{
    float4 position [[position]];
    float4 color;
    float dotProd;
};

// Just position and color
struct ProjVertexB
{
    float4 position [[position]];
    float4 color;
};

// Ye olde triangle vertex
struct VertexTriA
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float4 color [[attribute(WKSVertexColorAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WKSVertexTextureBaseAttribute)]];
};

// Output vertex to the fragment shader
struct ProjVertexTriA {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
};

// Triangle vertex with a couple of texture coordinates
struct VertexTriB
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float4 color [[attribute(WKSVertexColorAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
    float2 texCoord0 [[attribute(WKSVertexTextureBaseAttribute)]];
    float2 texCoord1 [[attribute(WKSVertexTextureBaseAttribute+1)]];
};

// Output vertex to the fragment shader
struct ProjVertexTriB {
    float4 position [[position]];
    float4 color;
    float2 texCoord0;
    float2 texCoord1;
};

/**
 Wide Vector Shaders
 These work to build/render objects in 2D space, but based
 on 3D locations.
 */

struct VertexTriWideVec
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
    float4 texInfo [[attribute(WKSVertexWideVecTexInfoAttribute)]];
    float3 p1 [[attribute(WKSVertexWideVecP1Attribute)]];
    float3 n0 [[attribute(WKSVertexWideVecN0Attribute)]];
    float c0 [[attribute(WKSVertexWideVecC0Attribute)]];
};

// Wide Vector vertex passed to fragment shader
struct ProjVertexTriWideVec {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
    float dotProd;
};

// Input vertex data for Screen Space shaders
struct VertexTriScreenSpace
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WKSVertexTextureBaseAttribute)]];
    float4 color [[attribute(WKSVertexColorAttribute)]];
    float2 offset [[attribute(WKSVertexScreenSpaceOffsetAttribute)]];
    float3 rot [[attribute(WKSVertexScreenSpaceRotAttribute)]];
    float3 dir [[attribute(WKSVertexScreenSpaceDirAttribute)]];
};

// Triangle vertex with a couple of texture coordinates
struct VertexTriBillboard
{
    float3 position [[attribute(WKSVertexPositionAttribute)]];
    float3 offset [[attribute(WKSVertexBillboardOffsetAttribute)]];
    float4 color [[attribute(WKSVertexColorAttribute)]];
    float3 normal [[attribute(WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WKSVertexTextureBaseAttribute)]];
};
    
#endif
