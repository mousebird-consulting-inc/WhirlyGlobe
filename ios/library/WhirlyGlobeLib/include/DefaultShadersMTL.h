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

/** Expressions are used to change values like width and opacity over zoom levels. **/
#define WKSExpStops 8

// Expression types
typedef enum {
    ExpNone = 0,ExpLinear,ExpExponential
} ExpType;

// Single floating point value expression
struct FloatExp {
    ExpType type;
    int numStops;
    float base;
    float stopInputs[WKSExpStops];
    float stopOutputs[WKSExpStops];
};

// Color value expression
struct ColorExp {
    ExpType type;
    int numStops;
    float base;
    float stopInputs[WKSExpStops];
    simd::float4 stopOutputs[WKSExpStops];
};

/** Attributes within the [[stage_in]] for vertex shaders **/
    
// Basic vertex attribute positions
typedef enum {
    WKSVertexPositionAttribute = 0,
    WKSVertexColorAttribute,
    WKSVertexNormalAttribute,
    WKSVertexTextureBaseAttribute
} WKSVertexAttributes;
    
// Wide Vector vertex attribute positions
typedef enum {
    WKSVertexWideVecTexInfoAttribute = 5,
    WKSVertexWideVecP1Attribute,
    WKSVertexWideVecN0Attribute,
    WKSVertexWideVecC0Attribute
} WKSVertexWideVecAttributes;
    
// Screen space vertex attribute positions
typedef enum {
    WKSVertexScreenSpaceOffsetAttribute = 5,
    WKSVertexScreenSpaceRotAttribute,
    WKSVertexScreenSpaceDirAttribute
} WKSVertexScreenSpaceAttributes;
    
// Model instance vertex attribute positions
typedef enum {
    WKSVertexInstanceColorAttribute = 5,
    WKSVertexInstanceMatrixAttribute,
    WKSVertexInstanceCenterAttribute,
    WKSVertexInstanceDirAttribute
} WKSVertexInstanceAttributes;
    
// Billboard offsets
// TODO: Billboards should be instances
typedef enum {
    WKSVertexBillboardOffsetAttribute = 6
} WKSVertexBillboardAttributes;

// Maximum number of textures we currently support
#define WKSTextureMax 8
// Textures passed into the shader start here
#define WKSTextureEntryLookup 5

#define MaxZoomSlots 32

// All the buffer entries (other than stage_in) for the vertex shaders
typedef enum {
    WKSVertUniformArgBuffer = 10,
    WKSVertLightingArgBuffer = 11,
    // These are free form with their own subsections
    WKSVertexArgBuffer = 12,
    // Textures are optional
    WKSVertTextureArgBuffer = 13,
    // Model instances
    WKSVertModelInstanceArgBuffer = 14,
    // If we're using the indirect instancing (can be driven by the GPU) this is
    //  where the indirect buffer lives
    WKSVertInstanceIndirectBuffer = 15,
    WKSVertMaxBuffer
} WKSVertexArgumentBuffers;

// All the buffer entries for the fragment shaders
typedef enum {
    WKSFragUniformArgBuffer = 0,
    WKSFragLightingArgBuffer = 1,
    WKSFragmentArgBuffer = 2,
    WKSFragTextureArgBuffer = 4,
    WKSFragMaxBuffer
} WKSFragArgumentBuffer;

// Entries in the free form argument buffer
// These must be in order, but you can add new ones at the end
typedef enum {
    WKSUniformDrawStateEntry = 0,
    WKSUniformVecEntryExp = 99,
    WKSUniformWideVecEntry = 100,
    WKSUniformWideVecEntryExp = 110,
    WKSUniformScreenSpaceEntry = 200,
    WKSUniformScreenSpaceEntryExp = 210,
    WKSUniformModelInstanceEntry = 300,
    WKSUniformBillboardEntry = 400
} WKSArgBufferEntries;

// Uniforms for the basic case.  Nothing fancy.
struct Uniforms
{
    simd::float4x4 mvpMatrix;
    simd::float4x4 mvpMatrixDiff;
    simd::float4x4 mvpInvMatrix;
    simd::float4x4 mvMatrix;
    simd::float4x4 mvMatrixDiff;
    simd::float4x4 mvNormalMatrix;
    simd::float4x4 pMatrix;
    simd::float4x4 pMatrixDiff;
    simd::float3 eyePos;
    simd::float3 eyeVec;
    simd::float2 screenSizeInDisplayCoords;  // Size of the whole frame in display coords
    simd::float2 frameSize;    // Output framebuffer size
    uint frameCount;            // Starts at zero and goes up from there every frame
    int outputTexLevel;        // Normally 0, unless we're running a reduce
    float currentTime;         // Current time relative to the start of the renderer
    float height;              // Height above the ground/globe
    float zoomSlots[MaxZoomSlots];  // Zoom levels calculated by the sampling layers
    bool globeMode;
};

// Things that change per drawable (like fade)
struct UniformDrawStateA {
    simd::float4x4 singleMat; // Individual transform used by model instances
    simd::float2 screenOrigin; // Used for texture pinning in screen space
    float interp;              // Used to interpolate between two textures (if appropriate)
    int outputTexLevel;        // Normally 0, unless we're running a reduce
    int whichOffsetMatrix;     // Normally 0, unless we're in 2D mode drawing the same stuff multiple times
    float fadeUp,fadeDown;     // Fading in/out values
    float minVisible,maxVisible;  // Visibility by height
    float minVisibleFadeBand,maxVisibleFadeBand;
    int zoomSlot;              // Used to pass continuous zoom info
    bool clipCoords;           // If set, the geometry coordinates aren't meant to be transformed
    bool hasExp;                // Look for a UniformWideVecExp structure for color, opacity, and width
};

// Uniform expressions optionally passed to basic polygon shaders
struct UniformDrawStateExp {
    FloatExp opacityExp;
    ColorExp colorExp;
};
    
// Things that change per particle drawable
struct UniformDrawStateParticle {
    float pointSize;   // If set, the size of points to be rendered
    float time; // Relative time globally
    float lifetime;  // Total lifetime of a particle
    float frameLen; // Length of a single frame
};

//// Lighting support //////

// A single light
struct Light {
    simd::float4 ambient;
    simd::float4 diffuse;
    simd::float4 specular;
    simd::float3 direction;
    simd::float3 halfPlane;
    float viewDepend;
};

// Material definition
struct Material {
    simd::float4 ambient;
    simd::float4 diffuse;
    simd::float4 specular;
    float specularExponent;
};

// Lighting together in one struct
struct Lighting {
    Material mat;
    int numLights;
    Light lights[8];
};

// These are in their own structure with the textures
#define WKSTexBufTexPresent 100
#define WKSTexBuffIndirectOffset 110
#define WKSTexBuffIndirectScale 130
#define WKSTexBuffTextures 170

// Instructions to the wide vector shaders, usually per-drawable
struct UniformWideVec {
    float w2;       // Width / 2.0 in screen space
    float edge;     // Edge falloff control
    float texRepeat;  // Texture scaling specific to wide vectors
    simd::float4 color;  // Color override.  TODO: Use the standard one.  Seriously.
    bool hasExp;      // Look for a UniformWideVecExp structure for color, opacity, and width
};

// For variable width (and color, etc) lines we'll
struct UniformWideVecExp {
    FloatExp widthExp;
    FloatExp opacityExp;
    ColorExp colorExp;
};
    
// Instructions to the screen space shaders, usually per-drawable
struct UniformScreenSpace {
    float startTime;                // For moving objects, this is the base time
    bool keepUpright;
    bool activeRot;
    bool hasMotion;            // For objects that can move, check this
    bool hasExp;      // Look for a UniformScreenSpaceExp structure for color, opacity, and scale
};

// For variable width (and color, etc) lines we'll
struct UniformScreenSpaceExp {
    FloatExp scaleExp;
    FloatExp opacityExp;
    ColorExp colorExp;
};
    
// Instructions to the model instance shaders, per-drawable
struct UniformModelInstance {
    float startTime;                // For moving objects, this is the base time
    bool hasMotion;            // For objects that can move, check this
    bool useInstanceColor;     // For model instance, if set use the instance color
};

// Input model instance info
struct VertexTriModelInstance
{
    // Instance stuff
    simd::float4 color;
    simd::float4x4 mat;
    simd::float3 center;
    simd::float3 dir;
};

// Instructions to the billboard shaders, per-drawable
struct UniformBillboard {
    bool groundMode;
};
    
}

// These have syntax that only the shader language can grok
// We put them here so we can include them in other Metal libraries
#ifdef __METAL_VERSION__
// Vertices with position, color, and normal
struct VertexA
{
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
};

// Position, color, and dot project (for backface checking)
struct ProjVertexA
{
    float4 position [[invariant]] [[position]];
    float4 color;
    float dotProd;
};

// Just position and color
struct ProjVertexB
{
    float4 position [[invariant]] [[position]];
    float4 color;
};

// Ye olde triangle vertex
struct VertexTriA
{
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WhirlyKitShader::WKSVertexTextureBaseAttribute)]];
};

// Output vertex to the fragment shader
struct ProjVertexTriA {
    float4 position [[invariant]] [[position]];
    float4 color;
    float2 texCoord;
};

// Triangle vertex with a couple of texture coordinates
struct VertexTriB
{
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
    float2 texCoord0 [[attribute(WhirlyKitShader::WKSVertexTextureBaseAttribute)]];
    float2 texCoord1 [[attribute(WhirlyKitShader::WKSVertexTextureBaseAttribute+1)]];
};

// Output vertex to the fragment shader
struct ProjVertexTriB {
    float4 position [[invariant]] [[position]];
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
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
    float4 texInfo [[attribute(WhirlyKitShader::WKSVertexWideVecTexInfoAttribute)]];
    float3 p1 [[attribute(WhirlyKitShader::WKSVertexWideVecP1Attribute)]];
    float3 n0 [[attribute(WhirlyKitShader::WKSVertexWideVecN0Attribute)]];
    float c0 [[attribute(WhirlyKitShader::WKSVertexWideVecC0Attribute)]];
};

// Wide Vector vertex passed to fragment shader
struct ProjVertexTriWideVec {
    float4 position [[invariant]] [[position]];
    float4 color;
    float2 texCoord;
    float dotProd;
    float w2;
};

// Input vertex data for Screen Space shaders
struct VertexTriScreenSpace
{
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WhirlyKitShader::WKSVertexTextureBaseAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    float2 offset [[attribute(WhirlyKitShader::WKSVertexScreenSpaceOffsetAttribute)]];
    float3 rot [[attribute(WhirlyKitShader::WKSVertexScreenSpaceRotAttribute)]];
    float3 dir [[attribute(WhirlyKitShader::WKSVertexScreenSpaceDirAttribute)]];
};

// Triangle vertex with a couple of texture coordinates
struct VertexTriBillboard
{
    float3 position [[attribute(WhirlyKitShader::WKSVertexPositionAttribute)]];
    float3 offset [[attribute(WhirlyKitShader::WKSVertexBillboardOffsetAttribute)]];
    float4 color [[attribute(WhirlyKitShader::WKSVertexColorAttribute)]];
    float3 normal [[attribute(WhirlyKitShader::WKSVertexNormalAttribute)]];
    float2 texCoord [[attribute(WhirlyKitShader::WKSVertexTextureBaseAttribute)]];
};

typedef struct RegularTextures {
    // A bit per texture that's present
    int texPresent                          [[ id(WKSTexBufTexPresent) ]];
    // Texture indirection (for accessing sub-textures)
    float offset                            [[ id(WKSTexBuffIndirectOffset) ]] [2*WKSTextureMax];
    float scale                             [[ id(WKSTexBuffIndirectScale) ]] [2*WKSTextureMax];
    metal::texture2d<float, metal::access::sample> tex    [[ id(WKSTexBuffTextures) ]] [WKSTextureMax];
} RegularTextures;

struct VertexTriArgBufferA {
    WhirlyKitShader::UniformDrawStateA uniDrawState      [[ id(WhirlyKitShader::WKSUniformDrawStateEntry) ]];
    bool hasTextures;
};

struct VertexTriArgBufferAExp {
    WhirlyKitShader::UniformDrawStateA uniDrawState      [[ id(WhirlyKitShader::WKSUniformDrawStateEntry) ]];
    WhirlyKitShader::UniformDrawStateExp drawStateExp    [[ id(WhirlyKitShader::WKSUniformVecEntryExp) ]];
    bool hasTextures;
};

struct FragTriArgBufferB {
    WhirlyKitShader::UniformDrawStateA uniDrawState      [[ id(WhirlyKitShader::WKSUniformDrawStateEntry) ]];
    bool hasTextures;
};

#endif
