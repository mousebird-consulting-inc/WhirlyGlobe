/*  wkDefaultShaders.metal
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#include <metal_stdlib>
#import "../include/DefaultShadersMTL.h"

using namespace metal;
using namespace WhirlyKitShader;

struct VertexArgBufferA {
    UniformDrawStateA uniDrawState  [[ id(WKSUniformDrawStateEntry) ]];
};
struct VertexTriArgBufferB {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    bool hasTextures;
    bool hasLighting;
};
struct VertexTriArgBufferC {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformDrawStateExp drawStateExp    [[ id(WKSUniformVecEntryExp) ]];
    bool hasTextures;
    bool hasLighting;
};


// Calculate fade based on time
// (or) based on height.
float calculateFade(constant Uniforms &uni,
                    constant UniformDrawStateA &uniA)
{
    // Figure out if we're fading in or out
    float fade = 1.0;
    if (uniA.fadeDown < uniA.fadeUp)
    {
        // Heading to 1
        if (uni.currentTime < uniA.fadeDown)
            fade = 0.0;
        else
            if (uni.currentTime > uniA.fadeUp)
                fade = 1.0;
            else
                fade = (uni.currentTime - uniA.fadeDown)/(uniA.fadeUp - uniA.fadeDown);
    } else if (uniA.fadeUp < uniA.fadeDown) {
        // Heading to 0
        if (uni.currentTime < uniA.fadeUp)
            fade = 1.0;
        else
            if (uni.currentTime > uniA.fadeDown)
                fade = 0.0;
            else
                fade = 1.0-(uni.currentTime - uniA.fadeUp)/(uniA.fadeDown - uniA.fadeUp);
    }

    // Deal with the range based fade
    if (uni.height > 0.0)
    {
        float factor = 1.0;
        if (uniA.minVisibleFadeBand != 0.0)
        {
            float a = (uni.height - uniA.minVisible)/uniA.minVisibleFadeBand;
            if (a >= 0.0 && a < 1.0)
                factor = a;
        }
        if (uniA.maxVisibleFadeBand != 0.0)
        {
            float b = (uniA.maxVisible - uni.height)/uniA.maxVisibleFadeBand;
            if (b >= 0.0 && b < 1.0)
                factor = b;
        }

        fade = fade * factor;
    }

    return fade;
}

#if !MAPLY_MINIMAL

// Get zoom from appropriate slot
float ZoomFromSlot(constant Uniforms &uniforms,int zoomSlot)
{
    if (zoomSlot < 0 || zoomSlot >= MaxZoomSlots)
        return 0.0;
    float zoom = uniforms.zoomSlots[zoomSlot];
    if (zoom > 1000000.0)  // MAXFLOAT means it isn't on
        return 0.0;
    
    return zoom;
}

// Calculate a float based on the current zoom level
float ExpCalculateFloat(constant FloatExp &floatExp,float zoom,float defaultVal)
{
    if (floatExp.numStops == 0 || floatExp.type == ExpNone)
        return defaultVal;
    
    int stopA=0,stopB=-1;
    if (zoom <= floatExp.stopInputs[stopA])
        return floatExp.stopOutputs[stopA];
    for (int which = 1;which < floatExp.numStops;which++) {
        stopB = which;
        if (floatExp.stopInputs[stopA] <= zoom && zoom < floatExp.stopInputs[stopB]) {
            float zoomA = floatExp.stopInputs[stopA];
            float zoomB = floatExp.stopInputs[stopB];
            float valA = floatExp.stopOutputs[stopA];
            float valB = floatExp.stopOutputs[stopB];
            switch (floatExp.type) {
                case WhirlyKitShader::ExpLinear:
                {
                    float t = (zoom-zoomA)/(zoomB-zoomA);
                    return t * (valB-valA) + valA;
                }
                    break;
                case WhirlyKitShader::ExpExponential:
                {
                    float ratio = 1.0;
                    if (floatExp.base == 1.0)
                        ratio = (zoom-zoomA)/(zoomB-zoomA);
                    else {
                        float soFar = zoom-zoomA;
                        ratio = (pow(floatExp.base, soFar) - 1.0) / (pow(floatExp.base,zoomB-zoomA) - 1.0);
                    }
                    return ratio * (valB - valA) + valA;
                }
                    break;
                default:
                    break;
            }
        }
        stopA = stopB;
    }
    
    return floatExp.stopOutputs[stopB];
}

// Calculate a color based on the current zoom level
float4 ExpCalculateColor(constant ColorExp &floatExp,float zoom,float4 defaultVal)
{
    if (floatExp.numStops == 0 || floatExp.type == ExpNone)
        return defaultVal;
    
    int stopA=0,stopB=-1;
    if (zoom <= floatExp.stopInputs[stopA])
        return floatExp.stopOutputs[stopA];
    for (int which = 1;which < floatExp.numStops;which++) {
        stopB = which;
        if (floatExp.stopInputs[stopA] <= zoom && zoom < floatExp.stopInputs[stopB]) {
            float zoomA = floatExp.stopInputs[stopA];
            float zoomB = floatExp.stopInputs[stopB];
            float4 valA = floatExp.stopOutputs[stopA];
            float4 valB = floatExp.stopOutputs[stopB];
            switch (floatExp.type) {
                case WhirlyKitShader::ExpLinear:
                {
                    float t = (zoom-zoomA)/(zoomB-zoomA);
                    return t * (valB-valA) + valA;
                }
                    break;
                case WhirlyKitShader::ExpExponential:
                {
                    float ratio = 1.0;
                    if (floatExp.base == 1.0)
                        ratio = (zoom-zoomA)/(zoomB-zoomA);
                    else {
                        float soFar = zoom-zoomA;
                        ratio = (pow(floatExp.base, soFar) - 1.0) / (pow(floatExp.base,zoomB-zoomA) - 1.0);
                    }
                    return ratio * (valB - valA) + valA;
                }
                    break;
                default:
                    break;
            }
        }
        stopA = stopB;
    }
    
    return floatExp.stopOutputs[stopB];
}

// Vertex shader for simple line on the globe
vertex ProjVertexA vertexLineOnly_globe(
    VertexA vert [[stage_in]],
    constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
    constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
    constant VertexArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexA outVert;
    
    outVert.color = float4(vert.color) * calculateFade(uniforms,vertArgs.uniDrawState);
    if (vertArgs.uniDrawState.clipCoords) {
        outVert.dotProd = 1.0;
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    } else {
        float4 pt = uniforms.mvMatrix * float4(vert.position, 1.0);
        pt /= pt.w;
        float4 testNorm = uniforms.mvNormalMatrix * float4(vert.normal,0.0);
        outVert.dotProd = dot(-pt.xyz,testNorm.xyz);
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    }
    
    return outVert;
}

// An empty argument buffer for fragments
// TODO: Figure out how to do without this
struct FragmentArgEmpty {
    bool nothing [[ id(0) ]];
};

// Fragment shader for simple line case
fragment float4 fragmentLineOnly_globe(
    ProjVertexA in [[stage_in]],
    constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
    constant Lighting &lighting [[ buffer(WKSFragLightingArgBuffer) ]],
    constant FragmentArgEmpty & uniEmpty [[buffer(WKSFragmentArgBuffer)]])
{
    if (in.dotProd <= 0.0)
        discard_fragment();
    return in.color;
}

// Back facing calculation for the globe
float calcGlobeDotProd(constant Uniforms &uniforms,float3 pos, float3 norm)
{
    if (!uniforms.globeMode)
        return 1.0;
    
    // Used to evaluate pixels behind the globe
    float4 pt = uniforms.mvMatrix * float4(pos,1.0);
    pt /= pt.w;
    float4 testNorm = uniforms.mvNormalMatrix * float4(norm,0.0);
    return dot(-pt.xyz,testNorm.xyz);
}

// Vertex shader for simple line on the flat map (no backface checking)
vertex ProjVertexB vertexLineOnly_flat(
    VertexA vert [[stage_in]],
    constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
    constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
    constant VertexArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexB outVert;
    
    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;

    outVert.color = float4(vert.color) * calculateFade(uniforms,vertArgs.uniDrawState);
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else
        outVert.position = uniforms.mvpMatrix * float4(vertPos,1.0);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentLineOnly_flat(
    ProjVertexB vert [[stage_in]],
    constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
    constant Lighting &lighting [[ buffer(WKSFragLightingArgBuffer) ]],
    constant FragmentArgEmpty & uniEmpty [[buffer(WKSFragmentArgBuffer)]])
{
    return vert.color;
}

#endif //!MAPLY_MINIMAL

// True if the given texture entry is there
bool TextureIsPresent(int bits,int which)
{
    return bits & (1<<which);
}

// Return the number of base textures
int TexturesBase(int bits)
{
    int count = 0;
    for (int ii=0;ii<WKSTextureEntryLookup;ii++)
        if (bits & (1<<ii))
            count++;
    
    return count;
}

float2 resolveTexCoords(float2 texCoord,constant RegularTextures &regTex,int texIdx)
{
    float2 offset(regTex.offset[texIdx*2],regTex.offset[texIdx*2+1]);
    float2 scale(regTex.scale[texIdx*2],regTex.scale[texIdx*2+1]);
    if (scale.x == 0.0)
        return texCoord;
    
    return texCoord * scale + offset;
}

// Calculate lighting for the given position and normal
float4 resolveLighting(float3 pos,
                      float3 norm,
                      float4 color,
                      constant Lighting &lighting,
                      float4x4 mvpMatrix)
{
#if !MAPLY_MINIMAL
    float4 ambient(0.0,0.0,0.0,0.0);
    float4 diffuse(0.0,0.0,0.0,0.0);
    
    if (lighting.numLights == 0)
        return color;

    for (int ii=0;ii<lighting.numLights;ii++) {
        constant Light *light = &lighting.lights[ii];
        float3 adjNorm = light->viewDepend > 0.0 ? normalize((mvpMatrix * float4(norm.xyz, 0.0)).xyz) : norm.xzy;
        float ndotl = max(0.0, dot(adjNorm, light->direction));
        ambient += light->ambient;
        diffuse += ndotl * light->diffuse;
    }
    
    return float4(ambient.xyz * lighting.mat.ambient.xyz * color.xyz + diffuse.xyz * color.xyz,color.a);
#else
    return color;
#endif //!MAPLY_MINIMAL
}

float4 basicPos(const float3 position,
                const constant Uniforms &uniforms,
                const constant UniformDrawStateA &uniDrawState)
{
    const float4 vertPos = float4((uniDrawState.singleMat * float4(position, 1.0f)).xyz, 1.0f);
    return uniDrawState.clipCoords ? vertPos :
        uniforms.pMatrix * (uniforms.mvMatrix * vertPos + uniforms.mvMatrixDiff * vertPos);
}

// Simple vertex shader for triangle with no lighting
vertex ProjVertexTriA vertexTri_noLight(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant VertexTriArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs  [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    outVert.position = basicPos(vert.position, uniforms, vertArgs.uniDrawState);
    outVert.color = vert.color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

#if !MAPLY_MINIMAL

// Simple vertex shader for triangle with no lighting that handles expressions
vertex ProjVertexTriA vertexTri_noLightExp(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant VertexTriArgBufferAExp & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs  [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        outVert.position = pt;
    }

    // Sort out expressions for color and opacity
    float4 color = vert.color;
    if (vertArgs.uniDrawState.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        color = ExpCalculateColor(vertArgs.drawStateExp.colorExp, zoom, color);
        float opacity = ExpCalculateFloat(vertArgs.drawStateExp.opacityExp, zoom, color.a);
        color.a = /*color.a * */opacity;
    }

    outVert.color = color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

#endif //!MAPLY_MINIMAL

// Simple vertex shader for triangle with basic lighting
vertex ProjVertexTriA vertexTri_light(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);
    
    outVert.position = basicPos(vert.position, uniforms, vertArgs.uniDrawState);
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    vert.color,
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

#if !MAPLY_MINIMAL

// Simple vertex shader for triangle with basic lighting and handles expressions
vertex ProjVertexTriA vertexTri_lightExp(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferC & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * vertArgs.uniDrawState.singleMat * float4(vert.position,1.0) +
                                        uniforms.mvMatrixDiff * vertArgs.uniDrawState.singleMat * float4(vert.position,1.0));
        outVert.position = pt;
    }
    
    // Sort out expressions for color and opacity
    float4 color = vert.color;
    if (vertArgs.uniDrawState.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        color = ExpCalculateColor(vertArgs.drawStateExp.colorExp, zoom, color);
        float opacity = ExpCalculateFloat(vertArgs.drawStateExp.opacityExp, zoom, color.a);
        color.a = /*color.a * */opacity;
    }

    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    color,
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);

    return outVert;
}

#endif //!MAPLY_MINIMAL

// Simple fragment shader for lines on flat map
fragment float4 fragmentTri_basic(
                ProjVertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);
    if (numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord);
    }
    return vert.color;
}

#if !MAPLY_MINIMAL
// Fragment shader that pulls the mask ID out only
fragment unsigned int fragmentTri_mask(ProjVertexTriA vert [[stage_in]],
                              constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                              constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                              constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    return vert.maskIDs[0];
}
#endif //!MAPLY_MINIMAL

// Vertex shader that handles up to two textures
vertex ProjVertexTriB vertexTri_multiTex(
                VertexTriB vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriB outVert;

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * vertArgs.uniDrawState.singleMat * float4(vert.position,1.0) +
                                        uniforms.mvMatrixDiff * vertArgs.uniDrawState.singleMat * float4(vert.position,1.0));
        outVert.position = pt;
    }
    outVert.color = resolveLighting(vertPos,
                                    vert.normal,
                                    float4(vert.color),
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);

    // Handle the various texture coordinate input options (none, 1, or 2)
    int numTextures = TexturesBase(texArgs.texPresent);
    if (numTextures == 0) {
        outVert.texCoord0 = float2(0.0,0.0);
        outVert.texCoord1 = float2(0.0,0.0);
    } else if (numTextures == 1) {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texArgs,0);
        outVert.texCoord1 = outVert.texCoord0;
    } else {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texArgs,0);
        outVert.texCoord1 = resolveTexCoords(vert.texCoord0,texArgs,1);
    }
    
    return outVert;
}

// Fragment shader that handles to two textures
fragment float4 fragmentTri_multiTex(
        ProjVertexTriB vert [[stage_in]],
        constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
        constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
        constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);
    
    // Handle none, 1 or 2 textures
    if (numTextures == 0) {
        return vert.color;
    } else if (numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord0);
    } else {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float4 color0 = texArgs.tex[0].sample(sampler2d, vert.texCoord0);
        // Note: There are times we may not want to reuse the same texture coordinates
        float4 color1 = texArgs.tex[1].sample(sampler2d, vert.texCoord0);
        return vert.color * mix(color0,color1,fragArgs.uniDrawState.interp);
    }
}

#if !MAPLY_MINIMAL
vertex ProjVertexTriNightDay vertexTri_multiTex_nightDay(
                VertexTriB vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriNightDay outVert;

    const float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        const float4 v = vertArgs.uniDrawState.singleMat * float4(vert.position,1.0);
        outVert.position = uniforms.pMatrix * (uniforms.mvMatrix * v + uniforms.mvMatrixDiff * v);
    }

    outVert.color = resolveLighting(vertPos, vert.normal, float4(vert.color),
                                    lighting, uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    //outVert.color = float4(1,1,1,1);

    // Handle the various texture coordinate input options (none, 1, or 2)
    const int numTextures = TexturesBase(texArgs.texPresent);
    if (numTextures == 0) {
        outVert.texCoord0 = float2(0.0,0.0);
        outVert.texCoord1 = float2(0.0,0.0);
    } else if (numTextures == 1) {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texArgs,0);
        outVert.texCoord1 = outVert.texCoord0;
    } else {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texArgs,0);
        outVert.texCoord1 = resolveTexCoords(vert.texCoord0,texArgs,1);
    }

    float3 adjNorm = vert.normal;
    float3 lightDir = float3(1,0,0);
    if (lighting.numLights > 0) {
        const constant thread Light &light = lighting.lights[0];
        if (light.viewDepend) {
            adjNorm = normalize((uniforms.mvpMatrix * float4(vert.normal.xyz, 0.0)).xyz);
        } else {
            adjNorm = vert.normal.xzy;
        }
        lightDir = light.direction;
    }
    outVert.ndotl = pow(max(0.0, dot(adjNorm, lightDir)), 0.5);

    return outVert;
}

fragment float4 fragmentTri_multiTex_nightDay(
        ProjVertexTriNightDay vert [[stage_in]],
        constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
        constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
        constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);

    // Handle none, 1 or 2 textures
    if (numTextures == 0) {
        return vert.color;
    } else if (numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord0);
    } else {
        // Note: There are times we may not want to reuse the same texture coordinates
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float4 color0 = texArgs.tex[0].sample(sampler2d, vert.texCoord0);
        float4 color1 = texArgs.tex[1].sample(sampler2d, vert.texCoord0);
        return vert.color * mix(color0,color1, 1.0 - vert.ndotl);
    }
}

// Fragment shader that handles two textures and does a ramp lookup
fragment float4 fragmentTri_multiTexRamp(ProjVertexTriB vert [[stage_in]],
                                         constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                                         constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                                         constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);
    float index = 0.0;

    constexpr sampler sampler2d(coord::normalized, filter::linear);

    // Handle none, 1 or 2 textures
    if (numTextures <= 1) {
        index = texArgs.tex[0].sample(sampler2d, vert.texCoord0).r;
    } else  {
        float index0 = texArgs.tex[0].sample(sampler2d, vert.texCoord0).r;
        // Note: There are times we may not want to reuse the same texture coordinates
        float index1 = texArgs.tex[1].sample(sampler2d, vert.texCoord0).r;
        index = mix(index0,index1,fragArgs.uniDrawState.interp);
    }

    // Use the lookup ramp if it's here
    float4 lookupColor(1.0,1.0,1.0,1.0);
    if (TextureIsPresent(texArgs.texPresent, WKSTextureEntryLookup)) {
        lookupColor = texArgs.tex[WKSTextureEntryLookup].sample(sampler2d,float2(index,0.5));
    }
    
    return vert.color * lookupColor;
}

struct TriWideArgBufferA {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformWideVec wideVec              [[ id(WKSUniformWideVecEntry) ]];
    bool hasTextures;
};

// Default wide vector program
vertex ProjVertexTriWideVec vertexTri_wideVec(
            VertexTriWideVec vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
            constant TriWideArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]],
            constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriWideVec outVert;
    outVert.maskIDs[0] = vert.mask0;
    outVert.maskIDs[1] = vert.mask1;

    float3 pos = (vertArgs.uniDrawState.singleMat * float4(vert.position.xyz,1.0)).xyz;

    // Pull out the width and possibly calculate one
    float w2 = vertArgs.wideVec.w2;
    if (w2 > 0.0) {
        w2 = w2 + vertArgs.wideVec.edge;
    }
    
    // Vary the offset over time for testing
//    float centerLine = vert.offset.z * (fmod(uniforms.currentTime,10.0)/10.0 * 200.0 - 100.0);
    float centerLine = vert.offset.z * vertArgs.wideVec.offset;

    outVert.color = vert.color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    float pixScale = min(uniforms.screenSizeInDisplayCoords.x,uniforms.screenSizeInDisplayCoords.y) / min(uniforms.frameSize.x,uniforms.frameSize.y);
    float realWidth2 = w2 * pixScale;
    float realCenterLine = centerLine * pixScale;
    
    float t0 = vert.c0 * (realWidth2 + realCenterLine);
    t0 = clamp(t0,-1.0,2.0);
    float3 dir = normalize(vert.p1 - vert.position);
    float3 realPosOffset = (vert.p1 - vert.position) * t0 +
                     dir * realWidth2 * vert.offset.y +
                     vert.n0 * (realCenterLine + realWidth2) +
                     vert.n0 * realWidth2 * vert.offset.x;
    
    float texScale = min(uniforms.frameSize.x,uniforms.frameSize.y)/(uniforms.screenSizeInDisplayCoords.x * vertArgs.wideVec.texRepeat);
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * realWidth2) * texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = uniforms.pMatrix * (uniforms.mvMatrix * float4(pos,1.0) + uniforms.mvMatrixDiff * float4(pos,1.0)) +
                       uniforms.pMatrix * (uniforms.mvMatrix * float4(realPosOffset,0.0) + uniforms.mvMatrixDiff * float4(realPosOffset,0.0));
    outVert.position = float4(screenPos.xy,0,screenPos.w);

    outVert.dotProd = calcGlobeDotProd(uniforms,pos,vert.normal);
    outVert.w2 = w2;
    
    return outVert;
}

struct TriWideArgBufferB {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformWideVec wideVec              [[ id(WKSUniformWideVecEntry) ]];
    UniformWideVecExp wideVecExp        [[ id(WKSUniformWideVecEntryExp) ]];
    bool hasTextures;
};

// This version handles expressions
vertex ProjVertexTriWideVec vertexTri_wideVecExp(
            VertexTriWideVec vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
            constant TriWideArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
            constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriWideVec outVert;
    
    float3 pos = (vertArgs.uniDrawState.singleMat * float4(vert.position.xyz,1.0)).xyz;
    float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);

    // Pull out the width and possibly calculate one
    float w2 = vertArgs.wideVec.w2;
    if (vertArgs.wideVec.hasExp)
        w2 = ExpCalculateFloat(vertArgs.wideVecExp.widthExp, zoom, 2.0*w2)/2.0;
    if (w2 > 0.0) {
        w2 = w2 + vertArgs.wideVec.edge;
    }
    
    // Pull out the center line offset, or calculate one
    float centerLine = vertArgs.wideVec.offset;
    if (vertArgs.wideVec.hasExp) {
        centerLine = ExpCalculateFloat(vertArgs.wideVecExp.offsetExp, zoom, centerLine);
    }
    centerLine = vert.offset.z * centerLine;

    float4 color = vert.color;
    if (vertArgs.wideVec.hasExp) {
        color = ExpCalculateColor(vertArgs.wideVecExp.colorExp, zoom, color);
        float opacity = ExpCalculateFloat(vertArgs.wideVecExp.opacityExp, zoom, color.a);
        color.a = /*color.a * */opacity;
    }
    outVert.color = color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    float pixScale = min(uniforms.screenSizeInDisplayCoords.x,uniforms.screenSizeInDisplayCoords.y) / min(uniforms.frameSize.x,uniforms.frameSize.y);
    float realWidth2 = w2 * pixScale;
    float realCenterLine = centerLine * pixScale;
    
    float t0 = vert.c0 * (realWidth2 + realCenterLine);
    t0 = clamp(t0,-1.0,2.0);
    float3 dir = normalize(vert.p1 - vert.position);
    float3 realPosOffset = (vert.p1 - vert.position) * t0 +
                     dir * realWidth2 * vert.offset.y +
                     vert.n0 * (realCenterLine + realWidth2) +
                     vert.n0 * realWidth2 * vert.offset.x;
    
    float texScale = min(uniforms.frameSize.x,uniforms.frameSize.y)/(uniforms.screenSizeInDisplayCoords.x * vertArgs.wideVec.texRepeat);
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * realWidth2) * texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = uniforms.pMatrix * (uniforms.mvMatrix * float4(pos,1.0) + uniforms.mvMatrixDiff * float4(pos,1.0)) +
                       uniforms.pMatrix * (uniforms.mvMatrix * float4(realPosOffset,0.0) + uniforms.mvMatrixDiff * float4(realPosOffset,0.0));
    outVert.position = float4(screenPos.xy,0,screenPos.w);

    outVert.dotProd = calcGlobeDotProd(uniforms,pos,vert.normal);
    outVert.w2 = w2;
    
    return outVert;
}

struct TriWideArgBufferFrag {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformWideVec wideVec              [[ id(WKSUniformWideVecEntry) ]];
    bool hasTextures;
};

// Fragment share that takes the back of the globe into account
fragment float4 fragmentTri_wideVec(
            ProjVertexTriWideVec vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
            constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
            constant TriWideArgBufferFrag & fragArgs [[buffer(WKSFragmentArgBuffer)]],
            constant WideVecTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    const int numTextures = TexturesBase(texArgs.texPresent);

    // Dot/dash pattern
    float4 patternColor(1.0,1.0,1.0,1.0);
    if (texArgs.texPresent & (1<<WKSTextureEntryLookup)) {
        if (vert.maskIDs[0] > 0 || vert.maskIDs[1] > 0) {
            // Pull the maskID from the input texture
            constexpr sampler sampler2d(coord::normalized, filter::linear);
            float2 loc(vert.position.x/uniforms.frameSize.x,vert.position.y/uniforms.frameSize.y);
            unsigned int maskID = texArgs.maskTex.sample(sampler2d, loc).r;
            if (vert.maskIDs[0] == maskID || vert.maskIDs[1] == maskID)
                discard_fragment();
        }
    }

    if (numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, address::repeat, filter::linear);
        // Just pulling the alpha at the moment
        // If we use the rest, we get interpolation down to zero, which isn't quite what we want here
        patternColor.a = texArgs.tex[0].sample(sampler2d, vert.texCoord).a;
    }
    float alpha = 1.0;
    float across = vert.w2 * vert.texCoord.x;
    if (across < fragArgs.wideVec.edge)
        alpha = across/fragArgs.wideVec.edge;
    if (across > vert.w2-fragArgs.wideVec.edge)
        alpha = (vert.w2-across)/fragArgs.wideVec.edge;
    
    return vert.dotProd > 0.0 ?
    float4(vert.color.rgb*patternColor.rgb,vert.color.a*alpha*patternColor.a)  : float4(0.0);
}

struct IntersectInfo {
    bool valid;
    float2 interPt;
    float c;
    float ta,tb;
};

// wedge product (2D cross product)
// if A ^ B > 0, A is to the right of B
float wedge(float2 a, float2 b) {
    return a.x * b.y - a.y * b.x;
}

// Intersect two lines
IntersectInfo intersectLines(float2 a0, float2 a1, float2 b0, float2 b1)
{
    const float2 dA = a0 - a1;
    const float2 dB = b0 - b1;

    // Solve the system of equations formed by equating the lines to get their common point.
    const float denom = wedge(dA, dB);
    if (denom == 0.0) {
        // If the denominator comes out zero, the lines are parallel and do not intersect
        return { .valid = false };
    }

    const float tA = (a0.x * a1.y - a0.y * a1.x);
    const float tB = (b0.x * b1.y - b0.y * b1.x);
    const float2 inter = float2((tA * dB.x - dA.x * tB), (tA * dB.y - dA.y * tB)) / denom;

    return {
        .valid = true,
        .interPt = inter,
        .ta = 0.0,
        .tb = 0.0,
    };
}

// Intersect two offset lines
IntersectInfo intersectWideLines(float2 p0,float2 p1,float2 p2,
                                 float2 n0,float2 n1)
{
    return intersectLines(p0 + n0, p1 + n0, p1 + n1, p2 + n1);
}

struct TriWideArgBufferC {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformWideVec wideVec              [[ id(WKSUniformWideVecEntry) ]];
    UniformWideVecExp wideVecExp        [[ id(WKSUniformWideVecEntryExp) ]];
    bool hasTextures;
};

// Used to track what info we have about a center point
struct CenterInfo {
    /// Screen coordinates of the line segment endpoint
    float2 screenPos;
    /// Length of the segment (in screen coordinates)
    float len;
    /// Normalized direction of the segment
    float2 nDir;
    /// Normalized plane normal, perpendicular to the segment
    float2 norm;
};

float3 viewPos(constant simd::float4x4 &mat, float3 vec) {
    const float4 p = mat * float4(vec,1.0);
    return p.xyz;   // / p.w; ?
}

float2 screenPos(constant Uniforms &u, float3 viewPos) {
    const float4 p4 = float4(viewPos, 1.0);
    const float4 s = u.pMatrix * (u.mvMatrix * p4 + u.mvMatrixDiff * p4);
    return s.xy / s.w;
}

constant constexpr float wideVecMinTurnThreshold = 1e-5;
constant constexpr float wideVecMaxTurnThreshold = 0.99999998476;  // sin(89.99 deg)
constant constexpr int WideVecPolyStartGeom = 0;
constant constexpr int WideVecPolyBodyGeom = 1;
constant constexpr int WideVecPolyEndGeom = 2;
constant constexpr float4 discardPt(0,0,-1e6,NAN);

// Performance version of wide vector shader
vertex ProjVertexTriWideVecPerf vertexTri_wideVecPerf(
          VertexTriWideVecB vert [[stage_in]],
          constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
          constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
          constant TriWideArgBufferC &vertArgs [[buffer(WKSVertexArgBuffer)]],
          uint instanceID [[instance_id]],
          constant VertexTriWideVecInstance *wideVecInsts   [[ buffer(WKSVertModelInstanceArgBuffer) ]],
          constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriWideVecPerf outVert = {
        .position = discardPt,
        .roundJoin = false,
    };

    // Vertex index within the instance, 0-11
    // Odd indexes are on the left, evens are on the right.
    const int whichVert = (vert.index >> 16) & 0xffff;
    // Polygon index within the segment.  0=Start cap, 1=body, 2=end cap
    const int whichPoly = vert.index & 0xffff;
    // Are we on the left edge, or the right?
    const bool isLeft = (whichVert & 1);
    // Are we on the starting end of the segment or the end?
    const bool isEnd = (whichVert > 5);

    // Track vertex for debugging
    //outVert.whichVert = whichVert;

    const float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);

    // Pull out the width and possibly calculate one
    float w2 = vertArgs.wideVec.w2;
    if (vertArgs.wideVec.hasExp) {
        w2 = ExpCalculateFloat(vertArgs.wideVecExp.widthExp, zoom, 2.0*w2)/2.0;
    }

    // w2 includes edge-blend, strokeWidth does not
    const float strokeWidth = 2 * w2;
    if (w2 > 0.0) {
        w2 = w2 + vertArgs.wideVec.edge;
    }

    // Disable joins for narrow lines
    auto joinType = (w2 >= 1) ? vertArgs.wideVec.join : WKSVertexLineJoinNone;

    // Find the various instances representing center points.
    // We need one behind and one ahead of us.
    //                     previous segment
    //                     |      this segment
    //                     |      |    next segment
    //                     |      |    |
    bool instValid[4] = { false, true, false, false };
    VertexTriWideVecInstance inst[4] = { {}, wideVecInsts[instanceID], {}, {} };

    if (inst[1].prev != -1 && joinType != WKSVertexLineJoinNone) {
        inst[0] = wideVecInsts[inst[1].prev];
        instValid[0] = true;
    }
    if (inst[1].next != -1) {
        inst[2] = wideVecInsts[inst[1].next];
        instValid[2] = true;

        if (inst[2].next != -1 && joinType != WKSVertexLineJoinNone) {
            inst[3] = wideVecInsts[inst[2].next];
            instValid[3] = true;
        }
    } else {
        // We need at least this and next
        return outVert;
    }

    const auto capType = vertArgs.wideVec.cap;
    const auto isStartCap = (whichPoly == WideVecPolyStartGeom && !instValid[0]);
    const auto isEndCap = (whichPoly == WideVecPolyEndGeom && !instValid[3]);

    // Butt is the default cap style, the line ends at the point.
    if ((isStartCap || isEndCap) && capType == WhirlyKitShader::WKSVertexLineCapButt) {
        return outVert;
    }

    // Figure out position on the screen for each center point.
    // centers[1] represents the segment leading to the current point.
    // centers[2] represents the segment leading from the current point.
    // centers[X] = vector from centers[X-1] to centers[X]
    CenterInfo centers[4];
    for (unsigned int ii=0;ii<4;ii++) {
        if (!instValid[ii]) {
            continue;
        }
        const float3 centerPos = viewPos(vertArgs.uniDrawState.singleMat, inst[ii].center);
        centers[ii].screenPos = screenPos(uniforms, centerPos);
        
        // Make sure the object is facing the user (only for the globe)
        if (uniforms.globeMode && ii == 1) {
            float4 pt = uniforms.mvMatrix * float4(centerPos,1.0);
            pt /= pt.w;

            if (pt.z > 0.0) {
                return outVert;
            } else {
                const float4 testNorm = uniforms.mvNormalMatrix * float4(centerPos,0.0);
                if (dot(-pt.xyz, testNorm.xyz) <= 0.0) {
                    return outVert;
                }
            }
        }
    }

    const float2 screenScale(2.0/uniforms.frameSize.x,2.0/uniforms.frameSize.y);    // ~(0.001,0.001)
    const float pixScale = min(uniforms.screenSizeInDisplayCoords.x,uniforms.screenSizeInDisplayCoords.y) /
                           min(uniforms.frameSize.x,uniforms.frameSize.y);
    const float texRepeatY = vertArgs.wideVec.texRepeat;
    const float texScale = 1 / pixScale / texRepeatY;   // texture coords / display coords = ~1000

    // Calculate directions and normals.  Done in isotropic coords to
    // avoid skewing everything when later multiplying by `screenScale`.
    for (unsigned int ii=1;ii<4;ii++) {
        if (instValid[ii-1]) {
            const float2 scaledDir = (centers[ii].screenPos - centers[ii-1].screenPos) / screenScale;
            centers[ii].len = length(scaledDir);
            centers[ii].nDir = normalize(scaledDir);
            centers[ii].norm = float2(-centers[ii].nDir.y, centers[ii].nDir.x);
        }
    }
    
    // Textures are based on un-projected coords, we'll need to know how that relates to projected
    // ones in order to adjust texture coordinates based on screen-based intersections.
    const float projScale = centers[2].len / inst[1].segLen;

    // Pull out the center line offset, or calculate one
    float centerLine = vertArgs.wideVec.offset;
    if (vertArgs.wideVec.hasExp) {
        centerLine = ExpCalculateFloat(vertArgs.wideVecExp.offsetExp, zoom, centerLine);
    }

    // Intersect on the left or right depending
    const float interSgn = (whichVert & 1) ? 1 : -1;

    // Do the offset intersection
    bool intersectValid = false;
    bool turningLeft = false;
    const int interIdx = isEnd ? 1 : 0;
    float2 offsetCenter = centers[interIdx + 1].screenPos + centers[2].norm * centerLine * screenScale;
    float2 interPt(0, 0), realInterPt(0, 0);
    float dotProd = 0, theta = 0, miterLength = 0;

    // If we're on the far end of the body segment, we need this and the next two segments.
    // Otherwise we need the previous, this, and the next segment.
    if (instValid[interIdx] && instValid[interIdx+1] && instValid[interIdx+2]) {
        
        // Don't even bother computing intersections for very acute angles or very small turns
        dotProd = dot(centers[interIdx+1].nDir, centers[interIdx+2].nDir);
        if (-wideVecMaxTurnThreshold < dotProd &&
            dotProd < wideVecMaxTurnThreshold &&
            abs(abs(dotProd) - 1) >= wideVecMinTurnThreshold) {

            // Interior turn angle, both vectors are normalized.
            theta = M_PI_F - acos(dotProd);

            // "If the miter length divided by the stroke width exceeds the miterlimit then:
            //   miter: the join is converted to a bevel
            //   miter-clip: the miter is clipped at half the miter length from the intersection"
            if (joinType == WKSVertexLineJoinMiter || joinType == WKSVertexLineJoinMiterClip) {
                miterLength = abs(1 / sin(theta / 2));
                if (miterLength > vertArgs.wideVec.miterLimit) {
                    if (joinType == WKSVertexLineJoinMiter) {
                        joinType = WKSVertexLineJoinBevel;
                    } else if (joinType == WKSVertexLineJoinMiterClip) {
                        miterLength = vertArgs.wideVec.miterLimit;
                    }
                }
            }
            
            // Intersect the left or right sides of prev-this and this-next, plus offset
            thread const CenterInfo &prev = centers[interIdx+0];
            thread const CenterInfo &cur  = centers[interIdx+1];
            thread const CenterInfo &next = centers[interIdx+2];
            const float2 edgeDist = screenScale * (interSgn * w2 + centerLine);
            const float2 n0 = edgeDist * cur.norm;
            const float2 n1 = edgeDist * next.norm;
            const IntersectInfo interInfo = intersectLines(prev.screenPos + n0, cur.screenPos + n0,
                                                           cur.screenPos + n1, next.screenPos + n1);
            if (interInfo.valid) {
                const float c = wedge(next.screenPos - cur.screenPos, cur.screenPos - prev.screenPos);
                turningLeft = (c < 0);
                realInterPt = interPt = interInfo.interPt;

                // Limit the distance to the smaller of half way back along the previous segment
                // or half way forward along the next one to keep consecutive segments from colliding.
                const float maxAdjDist = min(cur.len, next.len) / 2;

                // If we're using offsets, we also need to know the point where the offset lines
                // intersect, as the distance to the original intersection point isn't helpful.
                // todo: this doesn't make sense for small angles, but what's the actual threshold?
                if (centerLine != 0) {
                    const float2 cn0 = cur.norm * centerLine * screenScale;
                    const float2 cn1 = next.norm * centerLine * screenScale;
                    const IntersectInfo i2 = intersectLines(prev.screenPos + cn0, cur.screenPos + cn0,
                                                            cur.screenPos + cn1, next.screenPos + cn1);
                    if (i2.valid && length_squared((i2.interPt - offsetCenter)/screenScale) < maxAdjDist*maxAdjDist) {
                        offsetCenter = i2.interPt;
                    }
                }
                
                const float2 interVec = (interPt - offsetCenter) / screenScale;
                const float interDist2 = length_squared(interVec);
                const float maxClipDist2 = (maxAdjDist * vertArgs.wideVec.interClipLimit) *
                                           (maxAdjDist * vertArgs.wideVec.interClipLimit);

                // Limit intersection distance.
                // For up to a multiple of that, adjust the intersection point back along its
                // length to that limit, effectively narrowing the line instead of just giving up.
                if (interDist2 <= maxAdjDist*maxAdjDist) {
                    intersectValid = true;
                } else if (vertArgs.wideVec.interClipLimit > 0 && interDist2 <= maxClipDist2) {
                    interPt = offsetCenter + normalize(interVec) * sqrt(interDist2) * screenScale;
                    intersectValid = true;
                }
            }
        }
    }

    // Endcaps not used for miter case, discard them.
    if ((joinType == WKSVertexLineJoinMiter || joinType == WKSVertexLineJoinMiterSimple) &&
        whichPoly != WideVecPolyBodyGeom && !isStartCap && !isEndCap) {
        return outVert;
    }

    // Note: We're putting a color in the instance, but then it's hard to change
    //  So we'll pull the color out of the basic drawable
    float4 color = vert.color;
    if (vertArgs.wideVec.hasExp) {
        color = ExpCalculateColor(vertArgs.wideVecExp.colorExp, zoom, color);
        color.a = ExpCalculateFloat(vertArgs.wideVecExp.opacityExp, zoom, color.a);
    }

    outVert.color = color * calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.w2 = w2;
    outVert.edge = vertArgs.wideVec.edge;
    outVert.maskIDs[0] = inst[1].mask0;
    outVert.maskIDs[1] = inst[1].mask1;

    // Work out the corner positions by extending the normals
    const float2 realEdge = interSgn * w2 * screenScale;
    const float2 corner = offsetCenter + centers[2].norm * realEdge;
    const float turnSgn = turningLeft ? -1 : 1;
    const bool isInsideEdge = (isLeft == turningLeft);

    // Current corner is the default result for all points.
    float2 pos = corner;

    // Texture position is based on cumulative distance along the line.
    // Note that `inst[2].totalLen` wraps to zero at the end, and we don't want that.
    // Texture coords are used for edge blending, so we need to set them up even if there are no textures.
    float texY = inst[1].totalLen + (isEnd ? inst[1].segLen : 0);
    float texX = isLeft ? -1 : 1;

    bool discardTri = false;

    if (isStartCap || isEndCap) {
        // Square extends beyond the point by half a width.
        // Round uses the same geometry but rounds it off with the fragment shader.
        if (capType == WhirlyKitShader::WKSVertexLineCapSquare ||
            capType == WhirlyKitShader::WKSVertexLineCapRound) {
            switch (whichVert) {
                case 0: case 1: case 10: case 11:
                    pos = corner + centers[2].nDir * w2 * screenScale * (isEnd ? 1 : -1);
                    texY += w2 / projScale * (isEnd ? 1 : -1);
                    break;
            }
        }
        if (capType == WhirlyKitShader::WKSVertexLineCapRound) {
            outVert.roundJoin = true;
            outVert.centerPos = offsetCenter / screenScale;
            outVert.screenPos = pos / screenScale;
            outVert.midDir = centers[2].nDir * (isEnd ? 1 : -1);
        }
    } else {
        if (joinType == WKSVertexLineJoinNone || !intersectValid) {
            // Trivial case, just use the corner
            outVert.position = float4(pos, 0, 1);
            outVert.texCoord = -vertArgs.wideVec.texOffset + float2(texX, texY * texScale);
            return outVert;
        }

        // Since there is one, use the intersect point by default
        pos = interPt;

        // We'll need the corner on the opposite side for several things.
        const float2 realOtherEdge = -interSgn * w2 * screenScale;
        const float2 otherCorner = offsetCenter + centers[2].norm * realOtherEdge;

        // Miter is mostly handled above by using the intersect points instead of corners.
        if (joinType == WKSVertexLineJoinMiter ||
            joinType == WKSVertexLineJoinMiterSimple) {
            // Add the difference between the intersection point and the original corner,
            // accounting for the textures being based on un-projected coordinates.
            texY += dot((interPt - corner) / screenScale, centers[2].nDir) / projScale;
        }
        // For a bevel, use the intersect point for the inside of the turn, but not the outside.
        // Round piggypacks on bevel.
        else if (joinType == WKSVertexLineJoinBevel || joinType == WKSVertexLineJoinRound) {
            switch (whichVert) {
                // Start cap, 0-3-1, 0-2-3
                case 2: case 10:
                    discardTri = true;  // Not using triangle #2
                    break;
                // Merge inside corners to avoid overlap, use default outside corner.
                case 0: case 3: case 8: case 9:
                case 4: case 5: case 6: case 7:
                    pos = isInsideEdge ? interPt : corner;
                    break;
                case 1: case 11: {
                    // Use the point halfway between the outside corner and the one on the opposite side.
                    const float2 norm = centers[(whichVert == 1) ? 1 : 3].norm;
                    const float2 c = offsetCenter + centers[2].norm * realEdge * turnSgn;
                    const float2 e = turnSgn * interSgn * w2 * screenScale;
                    pos = (offsetCenter + c + norm * e) / 2;
                    // We're placing the vertex on the "wrong" side, so fix the texture X.
                    texX *= turnSgn;
                    break;
                }
            }

            // For the round case, extend the center of the bevel out into a "tip," which will be
            // turned into a round extension by the fragment shader.  This isn't exactly right, but
            // I think we need more geometry to do better.
            // todo: fix texture Y-coords
            if (joinType == WKSVertexLineJoinRound && whichPoly != WideVecPolyBodyGeom && !discardTri) {
                outVert.roundJoin = true;
                outVert.centerPos = offsetCenter / screenScale;
                outVert.screenPos = pos / screenScale;

                // Direction bisecting the turn toward the outside (right for a left turn)
                const float2 mid = isEnd ? (centers[2].nDir - centers[3].nDir) :
                                           (centers[1].nDir - centers[2].nDir);
                outVert.midDir = normalize(mid / screenScale);

                if (whichVert == 1 || whichVert == 11) {
                    // Extend the corner far enough to cover the necessary round-ness.
                    // This should probably be related to the turn angle, we're just fudging it.
                    const float extend = 2 * w2;
                    pos += outVert.midDir * extend * screenScale;
                    outVert.screenPos = pos / screenScale;
                }
            }
        } else if (joinType == WKSVertexLineJoinMiterClip) {
            // Direction of intersect point (bisecting the segment directions)
            const float2 interVec = (realInterPt - offsetCenter) / screenScale;
            const float2 interDir = normalize(interVec) * turnSgn * interSgn;
            // And the perpendicular
            const float2 interNorm = float2(-interDir.y, interDir.x);
            // Distance from centerline intersect and edge intersect
            const float interDist = length(interVec);
            // "the miter is clipped by a line perpendicular to the line bisecting
            //  the angle between the two path segments at a distance of half the
            //  value of miter length from the intersection of the two path segments."
            const float midExt = miterLength / 2 * strokeWidth + vertArgs.wideVec.edge;

            switch (whichVert) {
                // Start cap, 0-3-1, 0-2-3
                case 0: case 8: {
                    // Out to the miter cap, then perpendicular to the line edge.
                    // The half-width of the clipped miter cap is the tangent of half the interior
                    // turn angle times the amount the original intersection extends beyond the cap.
                    const float miterEdgeW2 = (interDist - midExt) * tan(theta / 2);
                    pos = offsetCenter + screenScale * (interDir * midExt -
                           interNorm * miterEdgeW2 * turnSgn * (whichVert ? -1 : 1));
                    break;
                }
                case 1: case 10:
                    // Extend the turn bisector to the miter clip edge
                    pos = turningLeft ? (offsetCenter + interDir * midExt * screenScale) :
                                        ((whichVert == 1) ? corner : otherCorner);
                    break;
                case 2: case 9:
                    // Extend segment endpoint outward along the intersection angle by the miter length
                    pos = turningLeft ? ((whichVert == 2) ? corner : otherCorner) :
                                        (offsetCenter + interDir * midExt * screenScale);
                    break;
                case 3: case 11:
                    // The edge intersect, or the one on the other side
                    pos = turningLeft ? interPt : (offsetCenter + (offsetCenter - interPt));
                    break;
                // Body segment, 4-7-5, 4-6-7
                // Merge inside corners to avoid overlap, use default outside corner.
                case 4: case 5: case 6: case 7:
                    pos = isInsideEdge ? interPt : corner;
                    break;
            }

            // Fix texture X-coords for vertices that were placed opposite the usual even/odd side.
            // todo: fix texture Y-coords around the join.  Might have to pre-compute for that.
            switch (whichVert) {
                case 1: case 9: texX = -texX;   // fall through
                case 0: case 2: case 3: case 8: case 10: case 11: texX *= -turnSgn;
            }
        }
    }

    if (!discardTri) {
        outVert.position = float4(pos, 0, 1);
        // Opposite values because we're showing back faces.
        outVert.texCoord = -vertArgs.wideVec.texOffset + float2(texX, texY * texScale);
    }

    return outVert;
}

constant constexpr auto defaultWideTexFiltering =
#if defined(__HAVE_BICUBIC_FILTERING__)
    filter::bicubic;
#else
    filter::linear;
#endif

// Fragment shader that takes the back of the globe into account
fragment float4 fragmentTri_wideVecPerf(
            ProjVertexTriWideVecPerf vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
            constant TriWideArgBufferFrag & fragArgs [[buffer(WKSFragmentArgBuffer)]],
            constant WideVecTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    if (texArgs.texPresent & (1<<WKSTextureEntryLookup) && (vert.maskIDs[0] > 0 || vert.maskIDs[1] > 0)) {
        // Pull the maskID from the input texture
        constexpr sampler sampler2d(coord::normalized, filter::nearest);
        const float2 loc = vert.position.xy / uniforms.frameSize;
        unsigned int maskID = texArgs.maskTex.sample(sampler2d, loc).r;
        if (vert.maskIDs[0] == maskID || vert.maskIDs[1] == maskID) {
            discard_fragment();
        }
    }

    // If there's a texture, apply its alpha channel to get dash patterns, or whatever.
    const int numTextures = TexturesBase(texArgs.texPresent);
    float patternAlpha = 1.0;
    if (numTextures > 0) {
        const auto filt = defaultWideTexFiltering;  // todo: pull this from the args
        constexpr sampler sampler2d(coord::normalized, address::repeat, filt);
        // Just pulling the alpha at the moment
        // If we use the rest, we get interpolation down to zero, which isn't quite what we want here
        patternAlpha = texArgs.tex[0].sample(sampler2d, vert.texCoord).a;
    }

    // Reduce alpha of the "tip" for round joins to get a round look.
    float roundAlpha = 1.0;
    if (vert.roundJoin && dot(vert.screenPos - vert.centerPos, vert.midDir) > 0) {
        const float r = length_squared(vert.screenPos - vert.centerPos) / vert.w2 / vert.w2;
        roundAlpha = clamp((r > 0.95) ? (1 - r) * 20 : 1.0, 0.0, 1.0);
    }

    // Reduce alpha along the edges to get smooth blending.
    const float edgeAlpha = (vert.edge > 0) ? clamp((1 - abs(vert.texCoord.x)) * vert.w2 / vert.edge, 0.0, 1.0) : 1.0;

    return vert.color * float4(1,1,1,edgeAlpha * patternAlpha * roundAlpha);
}


struct VertexTriSSArgBufferA {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformScreenSpace ss      [[ id(WKSUniformScreenSpaceEntry) ]];
    bool hasTextures;
};

// Screen space vertex shader
vertex ProjVertexTriA vertexTri_screenSpace(
            VertexTriScreenSpace vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
            constant VertexTriSSArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]],
            constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    float3 pos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.ss.hasMotion)
        pos += (uniforms.currentTime - vertArgs.ss.startTime) * vert.dir;
    
    outVert.color = vert.color * calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.texCoord = vert.texCoord;
    
    // Convert from model space into display space
    float4 pt = uniforms.mvMatrix * float4(pos,1.0);
    pt /= pt.w;
    
    // Make sure the object is facing the user (only for the globe)
    float dotProd = 1.0;
    if (uniforms.globeMode) {
        float4 testNorm = uniforms.mvNormalMatrix * float4(vert.normal,0.0);
        dotProd = dot(-pt.xyz,testNorm.xyz);
    }
    
    // Project the point all the way to screen space
    float4 screenPt = uniforms.pMatrix * (uniforms.mvMatrix * float4(pos,1.0) + uniforms.mvMatrixDiff * float4(pos,1.0));
    screenPt /= screenPt.w;
    
    // Project the rotation into display space and drop the Z
    float2 screenOffset;
    if (vertArgs.ss.activeRot) {
        float4 projRot = uniforms.mvNormalMatrix * float4(vert.rot,0.0);
        float2 rotY = normalize(projRot.xy);
        float2 rotX(rotY.y,-rotY.x);
        screenOffset = vert.offset.x*rotX + vert.offset.y*rotY;
    } else
        screenOffset = vert.offset;
    
    outVert.maskIDs[0] = vert.maskID;

    float2 scale = float2(2.0/uniforms.frameSize.x,2.0/uniforms.frameSize.y);
    outVert.position = (dotProd > 0.0 && pt.z <= 0.0) ? float4(screenPt.xy + float2(screenOffset.x*scale.x,screenOffset.y*scale.y),0.0,1.0) : float4(0.0,0.0,0.0,0.0);
    
    return outVert;
}

struct VertexTriSSArgBufferB {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformScreenSpace ss      [[ id(WKSUniformScreenSpaceEntry) ]];
    UniformScreenSpaceExp ssExp [[ id(WKSUniformScreenSpaceEntryExp)]];
    bool hasTextures;
};

// Screen space vertex shader that handles expressions
vertex ProjVertexTriA vertexTri_screenSpaceExp(
            VertexTriScreenSpace vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
            constant VertexTriSSArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
            constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    float zoomScale = 1.0;
    if (vertArgs.ss.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        zoomScale = ExpCalculateFloat(vertArgs.ssExp.scaleExp, zoom, zoom);
    }
    
    float3 pos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.ss.hasMotion)
        pos += (uniforms.currentTime - vertArgs.ss.startTime) * vert.dir;
    
    float4 color = vert.color;
    if (vertArgs.ss.hasExp) {
        color = ExpCalculateColor(vertArgs.ssExp.colorExp, zoomScale, color);
        float opacity = ExpCalculateFloat(vertArgs.ssExp.opacityExp, zoomScale, color.a);
        color.a = /*color.a * */opacity;
    }

    outVert.color = color * calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.texCoord = vert.texCoord;
    
    // Convert from model space into display space
    float4 pt = uniforms.mvMatrix * float4(pos,1.0);
    pt /= pt.w;
    
    // Make sure the object is facing the user (only for the globe)
    float dotProd = 1.0;
    if (uniforms.globeMode) {
        float4 testNorm = uniforms.mvNormalMatrix * float4(vert.normal,0.0);
        dotProd = dot(-pt.xyz,testNorm.xyz);
    }
    
    // Project the point all the way to screen space
    float4 screenPt = uniforms.pMatrix * (uniforms.mvMatrix * float4(pos,1.0) + uniforms.mvMatrixDiff * float4(pos,1.0));
    screenPt /= screenPt.w;
    
    // Project the rotation into display space and drop the Z
    float2 screenOffset;
    if (vertArgs.ss.activeRot) {
        float4 projRot = uniforms.mvNormalMatrix * float4(vert.rot,0.0);
        float2 rotY = normalize(projRot.xy);
        float2 rotX(rotY.y,-rotY.x);
        screenOffset = vert.offset.x*rotX + vert.offset.y*rotY;
    } else
        screenOffset = vert.offset;
    
    outVert.maskIDs[0] = vert.maskID;

    float2 scale = float2(2.0/uniforms.frameSize.x,2.0/uniforms.frameSize.y) * zoomScale;
    outVert.position = (dotProd > 0.0 && pt.z <= 0.0) ? float4(screenPt.xy + float2(screenOffset.x*scale.x,screenOffset.y*scale.y),0.0,1.0) : float4(0.0,0.0,0.0,0.0);
    
    return outVert;
}

struct VertexTriModelArgBuffer {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformModelInstance uniMI          [[ id(WKSUniformModelInstanceEntry) ]];
    bool hasLighting;
};

// Vertex shader for models
vertex ProjVertexTriB vertexTri_model(
          VertexTriB vert [[stage_in]],
          uint instanceID [[instance_id]],
          constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
          constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
          constant VertexTriModelArgBuffer &vertArgs [[buffer(WKSVertexArgBuffer)]],
          constant VertexTriModelInstance *modelInsts   [[ buffer(WKSVertModelInstanceArgBuffer) ]])
{
    ProjVertexTriB outVert;
    
    VertexTriModelInstance inst = modelInsts[instanceID];
    
    // Take movement into account
    float3 center = inst.center;
    if (vertArgs.uniMI.hasMotion)
        center += (uniforms.currentTime - vertArgs.uniMI.startTime) * inst.dir;
    float3 vertPos = (inst.mat * (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0))).xyz + center;
    
    outVert.position = uniforms.mvpMatrix * float4(vertPos,1.0);
    float4 color = vertArgs.uniMI.useInstanceColor ? inst.color : vert.color;
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    color,
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.texCoord0 = vert.texCoord0;
    outVert.texCoord1 = vert.texCoord1;

    return outVert;
}

struct VertexTriBillboardArgBuffer {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformBillboard uniBB              [[ id(WKSUniformBillboardEntry) ]];
};

// Vertex shader for billboards
// TODO: These should be model instances.  Ew.
vertex ProjVertexTriA vertexTri_billboard(
            VertexTriBillboard vert [[stage_in]],
            constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
            constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
            constant VertexTriBillboardArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriA outVert;
    outVert.maskIDs = uint2(0,0);

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;

    float3 newPos;
    // Billboard is rooted to its position
    if (vertArgs.uniBB.groundMode) {
        float3 axisX = -normalize(cross(uniforms.eyeVec,vert.normal));
        float3 axisZ = normalize(cross(axisX,vert.normal));
        newPos = vertPos + axisX * vert.offset.x + vert.normal * vert.offset.y + axisZ * vert.offset.z;
        outVert.position = uniforms.mvpMatrix * float4(newPos,1.0);
    } else {
        // Billboard orients fully toward the eye
        float4 pos = uniforms.mvMatrix * float4(vertPos,1.0);
        float3 pos3 = (pos/pos.w).xyz;
        newPos = float3(pos3.x + vert.offset.x,pos3.y+vert.offset.y,pos3.z+vert.offset.z);
        outVert.position = uniforms.pMatrix * float4(newPos,1.0);
    }

    // TODO: Support lighting.  Maybe
//    outVert.color = resolveLighting(vert.position,
//                                    vert.normal,
//                                    float4(vert.color),
//                                    lighting,
//                                    uniforms.mvpMatrix) * uniDrawState.fade;
    outVert.color = vert.color * calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.texCoord = vert.texCoord;

    return outVert;

}



// Atmosphere shaders

#import "../include/AtmosphereShadersMTL.h"
using namespace WhirlyKitAtmosphereShader;

struct VertexTriArgBuffer {
    UniformDrawStateA uniDrawState [[ id(WKSUniformDrawStateEntry) ]];
    AtmosShaderVertUniforms varUni [[ id(AtmosUniformVertEntry) ]];
    bool hasLighting;   // provide WKSVertLightingArgBuffer
};

struct FragTriArgBuffer {
    UniformDrawStateA uniDrawState [[ id(WKSUniformDrawStateEntry) ]];
    AtmosShaderFragUniforms varUni [[ id(AtmosUniformFragEntry) ]];
    bool hasLighting;   // provide WKSFragLightingArgBuffer
};

struct ProjVertexTriAtmos {
    float4 position [[invariant]] [[position]];
    float3 direction;
    float3 rayleighColor;
    float3 mieColor;
    float fade;
};

float scatteringScale(float fCos)
{
    const float x = 1.0 - fCos;
    return exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

// Find the intersections with the sphere, taking care to avoid loss of significance,
// and return the smaller/closer one.
float solveQuadratic(const float a, const float b, const float c)
{
    const float discr = b * b - 4 * a * c;
    if (discr < 0) return 0;
    else if (discr == 0) return - 0.5 * b / a;
    const float q = -0.5 * (b + sign(b) * sqrt(discr));
    return min(q / a, c / q);
}

vertex ProjVertexTriAtmos vertexTri_atmos(
        VertexTriA vert [[stage_in]],
        uint vertID [[vertex_id]],
        constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
        constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
        constant VertexTriArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriAtmos out;

    //const float3 lightPos = (lighting.numLights > 0) ? lighting.lights[0].direction : float3(1,0,0);
    const float3 lightPos = vertArgs.varUni.lightPos;
    const float3 eyePos = uniforms.eyePos;
    const float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords) {
        out.position = float4(vertPos,1);
    } else {
        out.position = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1) +
                                           uniforms.mvMatrixDiff * float4(vertPos,1));
    }

    float3 ray = vert.position - eyePos;
    float far = length(ray);
    ray /= far;

    // Calculate the closest intersection of the ray with the outer atmosphere
    // (which is the near point of the ray passing through the atmosphere)
    const float near = solveQuadratic(1, 2.0 * dot(eyePos, ray), vertArgs.varUni.c);

    // Calculate the ray's start and end positions in the atmosphere,
    // then calculate its scattering offset
    const float3 start = eyePos + ray * near;
    far -= near;

    // Initialize the scattering loop variables
    const float scaleDepth = vertArgs.varUni.scaleDepth;
    const float startAngle = dot(ray, start) / vertArgs.varUni.outerRadius;
    const float startDepth = exp(-1.0 / scaleDepth);
    const float startOffset = startDepth * scatteringScale(startAngle) * scaleDepth;
    
    const float sampleLength = far / vertArgs.varUni.samples;
    const float scaledLength = sampleLength * vertArgs.varUni.scale;
    const float3 sampleRay = ray * sampleLength;
    float3 samplePoint = start + sampleRay * 0.5;

    // Now loop through the sample points
    float3 frontColor = float3(0, 0, 0);
    for (int i=0; i<vertArgs.varUni.samples; i++)
    {
        const float height = length(samplePoint);
        const float depth = exp(vertArgs.varUni.scaleOverScaleDepth * (vertArgs.varUni.innerRadius - height));
        const float lightAngle = dot(lightPos, samplePoint) / height;
        const float cameraAngle = dot(ray, samplePoint) / height;
        const float scatter = startOffset + depth * scaleDepth * (scatteringScale(lightAngle) - scatteringScale(cameraAngle));
        const float3 attenuate = exp(-scatter * (vertArgs.varUni.invWavelength * vertArgs.varUni.kr4PI + vertArgs.varUni.km4PI));
        frontColor += attenuate * (depth * scaledLength);
        samplePoint += sampleRay;
    }

    out.mieColor = frontColor * vertArgs.varUni.kmESun;
    out.rayleighColor = frontColor * (vertArgs.varUni.invWavelength * vertArgs.varUni.krESun + vertArgs.varUni.km4PI);
    out.direction = eyePos - vert.position;
    out.fade = calculateFade(uniforms, vertArgs.uniDrawState);
 
    return out;
}

fragment float4 fragmentTri_atmos(
        ProjVertexTriAtmos vert [[stage_in]],
        constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
        constant Lighting &lighting [[ buffer(WKSFragLightingArgBuffer) ]],
        constant FragTriArgBuffer &fragArgs [[ buffer(WKSFragmentArgBuffer) ]])
{
    const float3 lightPos = (lighting.numLights > 0) ? lighting.lights[0].direction : float3(1,0,0);
    const float g = fragArgs.varUni.g;
    const float g2 = fragArgs.varUni.g2;
    const float cs = dot(normalize(lightPos), normalize(vert.direction)) / length(vert.direction);
    const float cs2 = cs * cs;
    const float rayPhase = 0.75 + 0.75 * cs2;
    const float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + cs2) / pow(1.0 + g2 - 2.0 * g * cs, 1.5);
    const float3 color = 1.0 - exp((rayPhase * vert.rayleighColor + miePhase * vert.mieColor) * -fragArgs.varUni.exposure);
    return float4(color, max(color.r, color.b) * vert.fade);
}

vertex ProjVertexTriA vertexTri_atmosGround(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriA out;
    out.maskIDs = uint2(0,0);
    
    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords) {
        out.position = float4(vertPos,1.0);
    } else {
        float4 v = vertArgs.uniDrawState.singleMat * float4(vert.position,1.0);
        out.position = uniforms.pMatrix * (uniforms.mvMatrix * v + uniforms.mvMatrixDiff * v);
    }
    
    out.color = float4(1,1,1,1);
    
    return out;
}

fragment float4 fragmentTri_atmosGround(
                ProjVertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                constant FragTriArgBuffer & fragArgs [[buffer(WKSFragmentArgBuffer)]])
{
    return vert.color;
}



// Stars shader.  Simple points
struct VertexIn {
    packed_float3 a_position;
    float         a_size;
};

struct VertexOut {
    float4 computedPosition [[position]];
    float4 color;
};

vertex VertexOut vertStars(constant VertexIn* vertex_array [[ buffer(0) ]],
                           unsigned int vid [[ vertex_id ]],
                           constant VertexArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]]) {
    VertexIn v = vertex_array[vid];
    VertexOut outVertex = VertexOut();
    outVertex.computedPosition = float4(v.a_position, 1.0);
    outVertex.color = float4(1,1,1,1);
    return outVertex;
}

fragment float4 fragmentStars(ProjVertexTriB vert [[stage_in]],
                                     constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                                     constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                                     constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);
    
    // Handle none, 1 or 2 textures
    if (numTextures == 0) {
        return vert.color;
    } else {
        return vert.color;
    }
}

#endif //!MAPLY_MINIMAL
