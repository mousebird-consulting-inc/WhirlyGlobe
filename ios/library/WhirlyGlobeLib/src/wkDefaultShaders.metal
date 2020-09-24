/*
 *  wkDefaultShaders.metal
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

#include <metal_stdlib>
#import "../include/DefaultShadersMTL.h"

using namespace metal;
using namespace WhirlyKitShader;

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
    } else {
        if (uniA.fadeUp < uniA.fadeDown)
        {
            // Heading to 0
            if (uni.currentTime < uniA.fadeUp)
                fade = 1.0;
            else
                if (uni.currentTime > uniA.fadeDown)
                    fade = 0.0;
                else
                    fade = 1.0-(uni.currentTime - uniA.fadeUp)/(uniA.fadeDown - uniA.fadeUp);
        }
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


struct VertexArgBufferA {
    UniformDrawStateA uniDrawState  [[ id(WKSUniformDrawStateEntry) ]];
};

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
}

// Simple vertex shader for triangle with no lighting
vertex ProjVertexTriA vertexTri_noLight(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant VertexTriArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs  [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        pt /= pt.w;
        outVert.position = pt;
    }

    outVert.color = vert.color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

// Simple vertex shader for triangle with no lighting that handles expressions
vertex ProjVertexTriA vertexTri_noLightExp(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant VertexTriArgBufferAExp & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs  [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;

    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        pt /= pt.w;
        outVert.position = pt;
    }

    // Sort out expressions for color and opacity
    float4 color = vert.color;
    if (vertArgs.uniDrawState.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        color = ExpCalculateColor(vertArgs.drawStateExp.colorExp, zoom, color);
        float opacity = ExpCalculateFloat(vertArgs.drawStateExp.opacityExp, zoom, 1.0);
        color.a = color.a * opacity;
    }

    outVert.color = color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}


struct VertexTriArgBufferB {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    bool hasTextures;
    bool hasLighting;
};

// Simple vertex shader for triangle with basic lighting
vertex ProjVertexTriA vertexTri_light(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        pt /= pt.w;
        outVert.position = pt;
    }
    
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

struct VertexTriArgBufferC {
    UniformDrawStateA uniDrawState      [[ id(WKSUniformDrawStateEntry) ]];
    UniformDrawStateExp drawStateExp    [[ id(WKSUniformVecEntryExp) ]];
    bool hasTextures;
    bool hasLighting;
};

// Simple vertex shader for triangle with basic lighting and handles expressions
vertex ProjVertexTriA vertexTri_lightExp(
                VertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
                constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
                constant VertexTriArgBufferC & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSVertTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords)
        outVert.position = float4(vertPos,1.0);
    else {
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        pt /= pt.w;
        outVert.position = pt;
    }
    
    // Sort out expressions for color and opacity
    float4 color = vert.color;
    if (vertArgs.uniDrawState.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        color = ExpCalculateColor(vertArgs.drawStateExp.colorExp, zoom, color);
        float opacity = ExpCalculateFloat(vertArgs.drawStateExp.opacityExp, zoom, 1.0);
        color.a = color.a * opacity;
    }

    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    color,
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    if (TexturesBase(texArgs.texPresent) > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    outVert.color = color;
    
    return outVert;
}


// Simple fragment shader for lines on flat map
fragment float4 fragmentTri_basic(
                ProjVertexTriA vert [[stage_in]],
                constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
                constant FragTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    if (TexturesBase(texArgs.texPresent) > 0) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord);
    }
    return vert.color;
}

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
        float4 pt = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1.0) + uniforms.mvMatrixDiff * float4(vertPos,1.0));
        pt /= pt.w;
        outVert.position = pt;
    }
    outVert.color = resolveLighting(vertPos,
                                    vert.normal,
                                    float4(vert.color),
                                    lighting,
                                    uniforms.mvpMatrix) *
                    calculateFade(uniforms,vertArgs.uniDrawState);
    outVert.color = vert.color;

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
fragment float4 fragmentTri_multiTex(ProjVertexTriB vert [[stage_in]],
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
    
    float3 pos = (vertArgs.uniDrawState.singleMat * float4(vert.position.xyz,1.0)).xyz;
    
    // Pull out the width and possibly calculate one
    float w2 = vertArgs.wideVec.w2;
    if (w2 > 0.0) {
        w2 = w2 + vertArgs.wideVec.edge;
    }

    outVert.color = vertArgs.wideVec.color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    float realWidth2 = w2 * min(uniforms.screenSizeInDisplayCoords.x,uniforms.screenSizeInDisplayCoords.y) / min(uniforms.frameSize.x,uniforms.frameSize.y);
    float t0 = vert.c0 * realWidth2;
    t0 = clamp(t0,0.0,1.0);
    float3 realPos = (vert.p1 - vert.position) * t0 + vert.n0 * realWidth2 + pos;
    float texScale = min(uniforms.frameSize.x,uniforms.frameSize.y)/(uniforms.screenSizeInDisplayCoords.x * vertArgs.wideVec.texRepeat);
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * realWidth2) * texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = uniforms.pMatrix * (uniforms.mvMatrix * float4(realPos,1.0) + uniforms.mvMatrixDiff * float4(realPos,1.0));
    screenPos /= screenPos.w;
    outVert.position = float4(screenPos.xy,0,1.0);

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
    
    // Pull out the width and possibly calculate one
    float w2 = vertArgs.wideVec.w2;
    if (vertArgs.wideVec.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        w2 = ExpCalculateFloat(vertArgs.wideVecExp.widthExp, zoom, 2.0*w2)/2.0;
    }
    if (w2 > 0.0) {
        w2 = w2 + vertArgs.wideVec.edge;
    }

    outVert.color = vertArgs.wideVec.color * calculateFade(uniforms,vertArgs.uniDrawState);
    
    float realWidth2 = w2 * min(uniforms.screenSizeInDisplayCoords.x,uniforms.screenSizeInDisplayCoords.y) / min(uniforms.frameSize.x,uniforms.frameSize.y);
    float t0 = vert.c0 * realWidth2;
    t0 = clamp(t0,0.0,1.0);
    float3 posOffset = (vert.p1 - vert.position) * t0 + vert.n0 * realWidth2;
    float texScale = min(uniforms.frameSize.x,uniforms.frameSize.y)/(uniforms.screenSizeInDisplayCoords.x * vertArgs.wideVec.texRepeat);
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * realWidth2) * texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = uniforms.pMatrix * (uniforms.mvMatrix * float4(pos,1.0) + uniforms.mvMatrixDiff * float4(pos,1.0)) +
            uniforms.pMatrix * (uniforms.mvMatrix * float4(posOffset,0.0) + uniforms.mvMatrixDiff * float4(posOffset,0.0));
    screenPos /= screenPos.w;
    outVert.position = float4(screenPos.xy,0,1.0);

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
            constant TriWideArgBufferFrag & fragArgs [[buffer(WKSFragmentArgBuffer)]],
            constant RegularTextures & texArgs [[buffer(WKSFragTextureArgBuffer)]])
{
    int numTextures = TexturesBase(texArgs.texPresent);

    // Dot/dash pattern
    float patternVal = 1.0;
    if (numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, address::repeat, filter::linear);
        patternVal = texArgs.tex[0].sample(sampler2d, float2(0.5,vert.texCoord.y)).r;
    }
    float alpha = 1.0;
    float across = vert.w2 * vert.texCoord.x;
    if (across < fragArgs.wideVec.edge)
        alpha = across/fragArgs.wideVec.edge;
    if (across > vert.w2-fragArgs.wideVec.edge)
        alpha = (vert.w2-across)/fragArgs.wideVec.edge;
    
    return vert.dotProd > 0.0 ? float4(fragArgs.wideVec.color.rgb,fragArgs.wideVec.color.a*alpha) * patternVal : float4(0.0);
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
    
    float zoomScale = 1.0;
    if (vertArgs.ss.hasExp) {
        float zoom = ZoomFromSlot(uniforms, vertArgs.uniDrawState.zoomSlot);
        zoomScale = ExpCalculateFloat(vertArgs.ssExp.scaleExp, zoom, zoom);
    }
    
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
