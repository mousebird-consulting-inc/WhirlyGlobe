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

struct VertexArgBufferA {
    constant Uniforms *uniforms               [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState  [[ id(WKSUniformDrawStateArgBuffer) ]];
};

// Vertex shader for simple line on the globe
vertex ProjVertexA vertexLineOnly_globe(
    VertexA vert [[stage_in]],
    constant VertexArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexA outVert;
    
    outVert.color = float4(vert.color) * vertArgs.uniDrawState->fade;
    if (vertArgs.uniDrawState->clipCoords) {
        outVert.dotProd = 1.0;
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vert.position,1.0);
    } else {
        float4 pt = vertArgs.uniforms->mvMatrix * float4(vert.position, 1.0);
        pt /= pt.w;
        float4 testNorm = vertArgs.uniforms->mvNormalMatrix * float4(vert.normal,0.0);
        outVert.dotProd = dot(-pt.xyz,testNorm.xyz);
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vert.position,1.0);
    }
    
    return outVert;
}

// An empty argument buffer for fragments
// TODO: Figure out how to do without this
struct FragmentArgEmpty {
    float nothing [[ id(0) ]];
};

// Fragment shader for simple line case
fragment float4 fragmentLineOnly_globe(
    ProjVertexA in [[stage_in]],
    constant FragmentArgEmpty & uniEmpty [[buffer(WKSFragmentArgBuffer)]])
{
    if (in.dotProd <= 0.0)
        discard_fragment();
    return in.color;
}

// Back facing calculation for the globe
float calcGlobeDotProd(constant Uniforms *uniforms,float3 pos, float3 norm)
{
    if (!uniforms->globeMode)
        return 1.0;
    
    // Used to evaluate pixels behind the globe
    float4 pt = uniforms->mvMatrix * float4(pos,1.0);
    pt /= pt.w;
    float4 testNorm = uniforms->mvNormalMatrix * float4(norm,0.0);
    return dot(-pt.xyz,testNorm.xyz);
}

// Vertex shader for simple line on the flat map (no backface checking)
vertex ProjVertexB vertexLineOnly_flat(
    VertexA vert [[stage_in]],
    constant VertexArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexB outVert;
    
    outVert.color = float4(vert.color) * vertArgs.uniDrawState->fade;
    if (vertArgs.uniDrawState->clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vert.position,1.0);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentLineOnly_flat(
    ProjVertexB vert [[stage_in]],
    constant FragmentArgEmpty & uniEmpty [[buffer(WKSFragmentArgBuffer)]])
{
    return vert.color;
}

typedef struct RegularTextures {
    // Number of valid textures
    int numTextures                    [[ id(WKSTexBufNumTextures) ]];
    // Texture indirection (for accessing sub-textures)
    float offset [[ id(WKSTexBuffIndirectOffset) ]] [2*WKSTextureMax];
    float scale [[ id(WKSTexBuffIndirectScale) ]] [2*WKSTextureMax];
    texture2d<float, access::sample> tex [[ id(WKSTexBuffTextures) ]] [WKSTextureMax];
} RegularTextures;

// Resolve texture coordinates with their parent offsts, if necessary
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
                      constant Lighting *lighting,
                      float4x4 mvpMatrix)
{
    float4 ambient(0.0,0.0,0.0,0.0);
    float4 diffuse(0.0,0.0,0.0,0.0);
    
    if (lighting->numLights == 0)
        return float4(1.0,1.0,1.0,1.0);

    for (int ii=0;ii<lighting->numLights;ii++) {
        constant Light *light = &lighting->lights[ii];
        float3 adjNorm = light->viewDepend > 0.0 ? normalize((mvpMatrix * float4(norm.xyz, 0.0)).xyz) : norm.xzy;
        float ndotl = max(0.0, dot(adjNorm, light->direction));
        ambient += light->ambient;
        diffuse += ndotl * light->diffuse;
    }
    
    return float4(ambient.xyz * lighting->mat.ambient.xyz * color.xyz + diffuse.xyz * color.xyz,color.a);
}

struct VertexTriArgBufferA {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    bool hasTextures                              [[ id(WKSHasTexturesArg)] ];
};

// Simple vertex shader for triangle with no lighting
vertex ProjVertexTriA vertexTri_noLight(
                VertexTriA vert [[stage_in]],
                constant VertexTriArgBufferA & vertArgs [[buffer(WKSVertexArgBuffer)]],
                constant RegularTextures & texArgs  [[buffer(WKSTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    if (vertArgs.uniDrawState->clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vert.position,1.0);
    outVert.color = float4(vert.color) * vertArgs.uniDrawState->fade;
    
    if (texArgs.numTextures > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

struct VertexTriArgBufferB {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    constant Lighting *lighting                   [[ id(WKSLightingArgBuffer) ]];
    bool hasTextures                              [[ id(WKSHasTexturesArg)] ];
};

// Simple vertex shader for triangle with basic lighting
vertex ProjVertexTriA vertexTri_light(
                VertexTriA vert [[stage_in]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                    constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    if (vertArgs.uniDrawState->clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vert.position,1.0);
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    float4(vert.color),
                                    vertArgs.lighting,
                                    vertArgs.uniforms->mvpMatrix); // * vertArgs.uniDrawState->fade;
                                                                    // TODO: Figure out why this doesn't work
    if (texArgs.numTextures > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texArgs,0);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentTri_basic(
                ProjVertexTriA vert [[stage_in]],
                constant VertexTriArgBufferA & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                    constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    if (texArgs.numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord);
    }
    return vert.color;
}

// Vertex shader that handles up to two textures
vertex ProjVertexTriB vertexTri_multiTex(
                VertexTriB vert [[stage_in]],
                constant VertexTriArgBufferB & vertArgs [[buffer(WKSVertexArgBuffer)]],
                    constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    ProjVertexTriB outVert;

    float3 vertPos = (vertArgs.uniDrawState->singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState->clipCoords)
        outVert.position = float4(vert.position,1.0);
    else {
        outVert.position = vertArgs.uniforms->mvpMatrix * float4(vertPos,1.0);
    }
    outVert.color = resolveLighting(vertPos,
                                    vert.normal,
                                    float4(vert.color),
                                    vertArgs.lighting,
                                    vertArgs.uniforms->mvpMatrix) * vertArgs.uniDrawState->fade;

    // Handle the various texture coordinate input options (none, 1, or 2)
    if (texArgs.numTextures == 0) {
        outVert.texCoord0 = float2(0.0,0.0);
        outVert.texCoord1 = float2(0.0,0.0);
    } else if (texArgs.numTextures == 1) {
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
                                     constant VertexTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                                     constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    constant UniformDrawStateA *drawStateA = fragArgs.uniDrawState;
    int numTextures = texArgs.numTextures;
    
    // Handle none, 1 or 2 textures
    if (numTextures == 0) {
        return vert.color * drawStateA->fade;
    } else if (numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * texArgs.tex[0].sample(sampler2d, vert.texCoord0);
    } else {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float4 color0 = texArgs.tex[0].sample(sampler2d, vert.texCoord0);
        // Note: There are times we may not want to reuse the same texture coordinates
        float4 color1 = texArgs.tex[1].sample(sampler2d, vert.texCoord0);
        return vert.color * mix(color0,color1,fragArgs.uniDrawState->interp);
    }
}

// Fragment shader that handles two textures and does a ramp lookup
fragment float4 fragmentTri_multiTexRamp(ProjVertexTriB vert [[stage_in]],
                                         constant VertexTriArgBufferB & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                                         constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    // Handle none, 1 or 2 textures
    if (texArgs.numTextures == 0) {
        return vert.color;
    } else if (texArgs.numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float index = texArgs.tex[0].sample(sampler2d, vert.texCoord0).r;
//        return vert.color * index;
        return vert.color * texArgs.tex[WKSTextureEntryLookup].sample(sampler2d,float2(index,0.5));
    } else {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float index0 = texArgs.tex[0].sample(sampler2d, vert.texCoord0).r;
        // Note: There are times we may not want to reuse the same texture coordinates
        float index1 = texArgs.tex[1].sample(sampler2d, vert.texCoord0).r;
        float index = mix(index0,index1,fragArgs.uniDrawState->interp);
        return vert.color * texArgs.tex[WKSTextureEntryLookup].sample(sampler2d,float2(index,0.5));
//        return vert.color * index;
    }
}

struct VertexTriWideArgBuffer {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    constant UniformWideVec *wideVec              [[ id(WKSUniformDrawStateWideVecArgBuffer) ]];
    bool hasTextures                              [[ id(WKSHasTexturesArg)] ];
};

vertex ProjVertexTriWideVec vertexTri_wideVec(
            VertexTriWideVec vert [[stage_in]],
            constant VertexTriWideArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]],
                   constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    ProjVertexTriWideVec outVert;
    
    outVert.color = vertArgs.wideVec->color * vertArgs.uniDrawState->fade;
    
    float t0 = vert.c0 * vertArgs.wideVec->real_w2;
    t0 = clamp(t0,0.0,1.0);
    float3 realPos = (vert.p1 - vert.position) * t0 + vert.n0 * vertArgs.wideVec->real_w2 + vert.position;
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * vertArgs.wideVec->real_w2) * vertArgs.wideVec->texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = vertArgs.uniforms->mvpMatrix * float4(realPos,1.0);
    screenPos /= screenPos.w;
    outVert.position = float4(screenPos.xy,0,1.0);

    outVert.dotProd = calcGlobeDotProd(vertArgs.uniforms,vert.position,vert.normal);
    
    return outVert;
}

// Fragment share that takes the back of the globe into account
fragment float4 fragmentTri_wideVec(
            ProjVertexTriWideVec vert [[stage_in]],
            constant VertexTriWideArgBuffer & fragArgs [[buffer(WKSFragmentArgBuffer)]],
                constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    // Dot/dash pattern
    float patternVal = 1.0;
    if (texArgs.numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, address::repeat, filter::linear);
//        patternVal = fragArgs.tex->tex[0].sample(sampler2d, float2(0.5,vert.texCoord.y)).r;
        // Note: Debugging
    }
    float alpha = 1.0;
    float across = vert.texCoord.x * fragArgs.wideVec->w2;
    if (across < fragArgs.wideVec->edge)
        alpha = across/fragArgs.wideVec->edge;
    if (across > fragArgs.wideVec->w2-fragArgs.wideVec->edge)
        alpha = (fragArgs.wideVec->w2-across)/fragArgs.wideVec->edge;
    return vert.dotProd > 0.0 ? fragArgs.wideVec->color * alpha * patternVal * fragArgs.uniDrawState->fade : float4(0.0);
}

struct VertexTriSSArgBuffer {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    constant UniformScreenSpace *ss               [[ id(WKSUniformDrawStateScreenSpaceArgBuffer) ]];
    bool hasTextures                              [[ id(WKSHasTexturesArg)] ];
};

// Screen space (no motion) vertex shader
vertex ProjVertexTriA vertexTri_screenSpace(
            VertexTriScreenSpace vert [[stage_in]],
            constant VertexTriSSArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]],
                 constant RegularTextures & texArgs [[buffer(WKSTextureArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 pos = vert.position;
    if (vertArgs.ss->hasMotion)
        pos += vertArgs.ss->time * vert.dir;
    
    outVert.color = vert.color * vertArgs.uniDrawState->fade;
    outVert.texCoord = vert.texCoord;
    
    // Convert from model space into display space
    float4 pt = vertArgs.uniforms->mvMatrix * float4(pos,1.0);
    pt /= pt.w;
    
    // Make sure the object is facing the user (only for the globe)
    float dotProd = 1.0;
    if (vertArgs.uniforms->globeMode) {
        float4 testNorm = vertArgs.uniforms->mvNormalMatrix * float4(vert.normal,0.0);
        dotProd = dot(-pt.xyz,testNorm.xyz);
    }
    
    // Project the point all the way to screen space
    float4 screenPt = vertArgs.uniforms->mvpMatrix * float4(pos,1.0);
    screenPt /= screenPt.w;
    
    // Project the rotation into display space and drop the Z
    float2 screenOffset;
    if (vertArgs.ss->activeRot) {
        float4 projRot = vertArgs.uniforms->mvNormalMatrix * float4(vert.rot,0.0);
        float2 rotY = normalize(projRot.xy);
        float2 rotX(rotY.y,-rotY.x);
        screenOffset = vert.offset.x*rotX + vert.offset.y*rotY;
    } else
        screenOffset = vert.offset;
    
    outVert.position = (dotProd > 0.0 && pt.z <= 0.0) ? float4(screenPt.xy + float2(screenOffset.x*vertArgs.ss->scale.x,screenOffset.y*vertArgs.ss->scale.y),0.0,1.0) : float4(0.0,0.0,0.0,0.0);
    
    return outVert;
}

struct VertexTriModelArgBuffer {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    constant Lighting *lighting                   [[ id(WKSLightingArgBuffer) ]];
    constant UniformModelInstance *uniMI          [[ id(WKSUniformDrawStateModelInstanceArgBuffer) ]];
    constant VertexTriModelInstance *modelInsts   [[ id(WKSModelInstanceArgBuffer) ]];
};

// Vertex shader for models
vertex ProjVertexTriB vertexTri_model(
          VertexTriB vert [[stage_in]],
          uint instanceID [[instance_id]],
          constant VertexTriModelArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriB outVert;
    
    VertexTriModelInstance inst = vertArgs.modelInsts[instanceID];
    
    // Take movement into account
    float3 center = inst.center;
    if (vertArgs.uniMI->hasMotion)
        center += vertArgs.uniMI->time * inst.dir;
    float3 vertPos = (inst.mat * float4(vert.position,1.0)).xyz + center;
    
    outVert.position = vertArgs.uniforms->mvpMatrix * float4(vertPos,1.0);
    float4 color = vertArgs.uniMI->useInstanceColor ? inst.color : vert.color;
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    color,
                                    vertArgs.lighting,
                                    vertArgs.uniforms->mvpMatrix) * vertArgs.uniDrawState->fade;
    outVert.texCoord0 = vert.texCoord0;
    outVert.texCoord1 = vert.texCoord1;

    return outVert;
}

struct VertexTriBillboardArgBuffer {
    constant Uniforms *uniforms                   [[ id(WKSUniformArgBuffer) ]];
    constant UniformDrawStateA *uniDrawState      [[ id(WKSUniformDrawStateArgBuffer) ]];
    constant Lighting *lighting                   [[ id(WKSLightingArgBuffer) ]];
    constant UniformBillboard *uniBB              [[ id(WKSUniformDrawStateBillboardArgBuffer) ]];
};

// Vertex shader for billboards
// TODO: These should be model instances.  Ew.
vertex ProjVertexTriA vertexTri_billboard(
            VertexTriBillboard vert [[stage_in]],
            constant VertexTriBillboardArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 newPos;
    // Billboard is rooted to its position
    if (vertArgs.uniBB->groundMode) {
        float3 axisX = normalize(cross(vertArgs.uniBB->eyeVec,vert.normal));
        float3 axisZ = normalize(cross(axisX,vert.normal));
        newPos = vert.position + axisX * vert.offset.x + vert.normal * vert.offset.y + axisZ * vert.offset.z;
    } else {
        // Billboard orients fully toward the eye
        float4 pos = vertArgs.uniforms->mvMatrix * float4(vert.position,1.0);
        float3 pos3 = (pos/pos.w).xyz;
        newPos = float3(pos3.x + vert.offset.x,pos3.y+vert.offset.y,pos3.z+vert.offset.z);
    }
    outVert.position = vertArgs.uniforms->mvpMatrix * float4(newPos,1.0);

    // TODO: Support lighting.  Maybe
//    outVert.color = resolveLighting(vert.position,
//                                    vert.normal,
//                                    float4(vert.color),
//                                    lighting,
//                                    uniforms.mvpMatrix) * uniDrawState.fade;
    outVert.color = vert.color;
    outVert.texCoord = vert.texCoord;

    return outVert;

}
