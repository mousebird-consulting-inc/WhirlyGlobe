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

// Vertex shader for simple line on the globe
vertex ProjVertexA vertexLineOnly_globe(
    VertexA vert [[stage_in]],
    constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
    constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]])
{
    ProjVertexA outVert;
    
    outVert.color = float4(vert.color) * uniDrawState.fade;
    if (uniDrawState.clipCoords) {
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

// Fragment shader for simple line case
fragment float4 framentLineOnly_globe(
    ProjVertexA in [[stage_in]])
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
    constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
    constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]])
{
    ProjVertexB outVert;
    
    outVert.color = float4(vert.color) * uniDrawState.fade;
    if (uniDrawState.clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentLineOnly_flat(
    ProjVertexB vert [[stage_in]],
    constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]])
{
    return vert.color;
}

// Resolve texture coordinates with their parent offsts, if necessary
float2 resolveTexCoords(float2 texCoord,TexIndirect texIndr)
{
    if (texIndr.scale.x == 0.0)
        return texCoord;
    
    return texCoord * texIndr.scale + texIndr.offset;
}

// Calculate lighting for the given position and normal
float4 resolveLighting(float3 pos,
                      float3 norm,
                      float4 color,
                      constant Lighting &lighting,
                      constant float4x4 &mvpMatrix)
{
    float4 ambient(0.0,0.0,0.0,0.0);
    float4 diffuse(0.0,0.0,0.0,0.0);

    for (int ii=0;ii<lighting.numLights;ii++) {
        constant Light &light = lighting.lights[ii];
        float3 adjNorm = light.viewDepend > 0.0 ? normalize((mvpMatrix * float4(norm.xyz, 0.0)).xyz) : norm.xzy;
        float ndotl = max(0.0, dot(adjNorm, light.direction));
        ambient += light.ambient;
        diffuse += ndotl * light.diffuse;
    }
    
    return float4(ambient.xyz * lighting.mat.ambient.xyz * color.xyz + diffuse.xyz * color.xyz,color.a);
}

// Simple vertex shader for triangle with no lighting
vertex ProjVertexTriA vertexTri_noLight(VertexTriA vert [[stage_in]],
                                        constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                        constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                        constant TexIndirect &texIndirect [[buffer(WKSTexIndirectStartBuffer)]])
{
    ProjVertexTriA outVert;
    
    if (uniDrawState.clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    outVert.color = float4(vert.color) * uniDrawState.fade;
    
    if (uniDrawState.numTextures > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texIndirect);
    
    return outVert;
}

// Simple vertex shader for triangle with basic lighting
vertex ProjVertexTriA vertexTri_light(VertexTriA vert [[stage_in]],
                                      constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                      constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                      constant Lighting &lighting [[buffer(WKSLightingBuffer)]],
                                      constant TexIndirect &texIndirect [[buffer(WKSTexIndirectStartBuffer)]])
{
    ProjVertexTriA outVert;
    
    if (uniDrawState.clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    float4(vert.color),
                                    lighting,
                                    uniforms.mvpMatrix) * uniDrawState.fade;
    if (uniDrawState.numTextures > 0)
        outVert.texCoord = resolveTexCoords(vert.texCoord,texIndirect);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentTri_basic(ProjVertexTriA vert [[stage_in]],
                                      constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                      constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                      texture2d<float,access::sample> tex [[texture(0)]])
{
    if (uniDrawState.numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * tex.sample(sampler2d, vert.texCoord);
    } else
        return vert.color;
}

// Vertex shader that handles up to two textures
vertex ProjVertexTriB vertexTri_multiTex(VertexTriB vert [[stage_in]],
                                         constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                         constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                         constant Lighting &lighting [[buffer(WKSLightingBuffer)]],
                                         constant TexIndirect &texIndirect0 [[buffer(WKSTexIndirectStartBuffer)]],
                                         constant TexIndirect &texIndirect1 [[buffer(WKSTexIndirectStartBuffer+1)]])
{
    ProjVertexTriB outVert;
    
    if (uniDrawState.clipCoords)
        outVert.position = float4(vert.position,1.0);
    else
        outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    float4(vert.color),
                                    lighting,
                                    uniforms.mvpMatrix) * uniDrawState.fade;

    // Handle the various texture coordinate input options (none, 1, or 2)
    if (uniDrawState.numTextures == 0) {
        outVert.texCoord0 = float2(0.0,0.0);
        outVert.texCoord1 = float2(0.0,0.0);
    } else if (uniDrawState.numTextures == 1) {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texIndirect0);
        outVert.texCoord1 = outVert.texCoord0;
    } else {
        outVert.texCoord0 = resolveTexCoords(vert.texCoord0,texIndirect0);
        // Note: Rather than reusing texCoord0, we should just attach the same data to texCoord1
        outVert.texCoord1 = resolveTexCoords(vert.texCoord0,texIndirect1);
    }
    
    return outVert;
}

// Fragment shader that handles to two textures
fragment float4 fragmentTri_multiTex(ProjVertexTriB vert [[stage_in]],
                                     constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                     constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                     texture2d<float,access::sample> tex0 [[texture(0)]],
                                     texture2d<float,access::sample> tex1 [[texture(1)]])
{
    // Handle none, 1 or 2 textures
    if (uniDrawState.numTextures == 0) {
        return vert.color;
    } else if (uniDrawState.numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        return vert.color * tex0.sample(sampler2d, vert.texCoord0);
    } else {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float4 color0 = tex0.sample(sampler2d, vert.texCoord0);
        // Note: There are times we may not want to reuse the same texture coordinates
        float4 color1 = tex1.sample(sampler2d, vert.texCoord0);
        return vert.color * mix(color0,color1,uniDrawState.interp);
    }
}

// Fragment shader that handles two textures and does a ramp lookup
fragment float4 fragmentTri_multiTexRamp(ProjVertexTriB vert [[stage_in]],
                                         constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                         constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                         texture2d<float,access::sample> tex0 [[texture(0)]],
                                         texture2d<float,access::sample> tex1 [[texture(1)]],
                                         texture2d<float,access::sample> rampTex [[texture(WKSTextureEntryLookup)]])
{
    // Handle none, 1 or 2 textures
    if (uniDrawState.numTextures == 0) {
        return vert.color;
    } else if (uniDrawState.numTextures == 1) {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float index = tex0.sample(sampler2d, vert.texCoord0).r;
//        return vert.color * index;
        return vert.color * rampTex.sample(sampler2d,float2(index,0.5));
    } else {
        constexpr sampler sampler2d(coord::normalized, filter::linear);
        float index0 = tex0.sample(sampler2d, vert.texCoord0).r;
        // Note: There are times we may not want to reuse the same texture coordinates
        float index1 = tex1.sample(sampler2d, vert.texCoord0).r;
        float index = mix(index0,index1,uniDrawState.interp);
        return vert.color * rampTex.sample(sampler2d,float2(index,0.5));
//        return vert.color * index;
    }
}

vertex ProjVertexTriWideVec vertexTri_wideVec(VertexTriWideVec vert [[stage_in]],
                                            constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                            constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                            constant UniformWideVec &uniSS [[buffer(WKSUniformDrawStateWideVecBuffer)]])
{
    ProjVertexTriWideVec outVert;
    
    outVert.color = uniSS.color * uniDrawState.fade;
    
    float t0 = vert.c0 * uniSS.real_w2;
    t0 = clamp(t0,0.0,1.0);
    float3 realPos = (vert.p1 - vert.position) * t0 + vert.n0 * uniSS.real_w2 + vert.position;
    float texPos = ((vert.texInfo.z - vert.texInfo.y) * t0 + vert.texInfo.y + vert.texInfo.w * uniSS.real_w2) * uniSS.texScale;
    outVert.texCoord = float2(vert.texInfo.x, texPos);
    float4 screenPos = uniforms.mvpMatrix * float4(realPos,1.0);
    screenPos /= screenPos.w;
    outVert.position = float4(screenPos.xy,0,1.0);

    outVert.dotProd = calcGlobeDotProd(uniforms,vert.position,vert.normal);
    
    return outVert;
}

// Fragment share that takes the back of the globe into account
fragment float4 fragmentTri_wideVec(ProjVertexTriWideVec vert [[stage_in]],
                                        constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                        constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                        constant UniformWideVec &uniSS [[buffer(WKSUniformDrawStateWideVecBuffer)]],
                                        texture2d<float,access::sample> tex [[texture(0)]])
{
    // Dot/dash pattern
    float patternVal = 1.0;
    if (uniDrawState.numTextures > 0) {
        constexpr sampler sampler2d(coord::normalized, address::repeat, filter::linear);
        patternVal = tex.sample(sampler2d, float2(0.5,vert.texCoord.y)).r;
    }
    float alpha = 1.0;
    float across = vert.texCoord.x * uniSS.w2;
    if (across < uniSS.edge)
        alpha = across/uniSS.edge;
    if (across > uniSS.w2-uniSS.edge)
        alpha = (uniSS.w2-across)/uniSS.edge;
    return vert.dotProd > 0.0 ? uniSS.color * alpha * patternVal * uniDrawState.fade : float4(0.0);
}

// Screen space (no motion) vertex shader
vertex ProjVertexTriA vertexTri_screenSpace(VertexTriScreenSpace vert [[stage_in]],
                                      constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                      constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                      constant UniformScreenSpace &uniSS [[buffer(WKSUniformDrawStateScreenSpaceBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 pos = vert.position;
    if (uniSS.hasMotion)
        pos += uniSS.time * vert.dir;
    
    outVert.color = vert.color * uniDrawState.fade;
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
    float4 screenPt = uniforms.mvpMatrix * float4(pos,1.0);
    screenPt /= screenPt.w;
    
    // Project the rotation into display space and drop the Z
    float2 screenOffset;
    if (uniSS.activeRot) {
        float4 projRot = uniforms.mvNormalMatrix * float4(vert.rot,0.0);
        float2 rotY = normalize(projRot.xy);
        float2 rotX(rotY.y,-rotY.x);
        screenOffset = vert.offset.x*rotX + vert.offset.y*rotY;
    } else
        screenOffset = vert.offset;
    
    outVert.position = (dotProd > 0.0 && pt.z <= 0.0) ? float4(screenPt.xy + float2(screenOffset.x*uniSS.scale.x,screenOffset.y*uniSS.scale.y),0.0,1.0) : float4(0.0,0.0,0.0,0.0);
    
    return outVert;
}

// Vertex shader for models
vertex ProjVertexTriB vertexTri_model(VertexTriB vert [[stage_in]],
                                      constant VertexTriModelInstance *modelInsts [[buffer(WKSModelInstanceBuffer)]],
                                      constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                      constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                      constant UniformModelInstance &uniMI [[buffer(WKSUniformDrawStateModelInstanceBuffer)]],
                                      constant Lighting &lighting [[buffer(WKSLightingBuffer)]],
                                      uint instanceID [[instance_id]])
{
    ProjVertexTriB outVert;
    
    VertexTriModelInstance inst = modelInsts[instanceID];
    
    // Take movement into account
    float3 center = inst.center;
    if (uniMI.hasMotion)
        center += uniMI.time * inst.dir;
    float3 vertPos = (inst.mat * float4(vert.position,1.0)).xyz + center;
    
    outVert.position = uniforms.mvpMatrix * float4(vertPos,1.0);
    float4 color = uniMI.useInstanceColor ? inst.color : vert.color;
    outVert.color = resolveLighting(vert.position,
                                    vert.normal,
                                    color,
                                    lighting,
                                    uniforms.mvpMatrix) * uniDrawState.fade;
    outVert.texCoord0 = vert.texCoord0;
    outVert.texCoord1 = vert.texCoord1;

    return outVert;
}

// Vertex shader for billboards
// TODO: These should be model instances.  Ew.
vertex ProjVertexTriA vertexTri_billboard(VertexTriBillboard vert [[stage_in]],
                                  constant Uniforms &uniforms [[buffer(WKSUniformBuffer)]],
                                  constant UniformDrawStateA &uniDrawState [[buffer(WKSUniformDrawStateBuffer)]],
                                  constant UniformBillboard &uniBB [[buffer(WKSUniformDrawStateBillboardBuffer)]],
                                  constant Lighting &lighting [[buffer(WKSLightingBuffer)]])
{
    ProjVertexTriA outVert;
    
    float3 newPos;
    // Billboard is rooted to its position
    if (uniBB.groundMode) {
        float3 axisX = normalize(cross(uniBB.eyeVec,vert.normal));
        float3 axisZ = normalize(cross(axisX,vert.normal));
        newPos = vert.position + axisX * vert.offset.x + vert.normal * vert.offset.y + axisZ * vert.offset.z;
    } else {
        // Billboard orients fully toward the eye
        float4 pos = uniforms.mvMatrix * float4(vert.position,1.0);
        float3 pos3 = (pos/pos.w).xyz;
        newPos = float3(pos3.x + vert.offset.x,pos3.y+vert.offset.y,pos3.z+vert.offset.z);
    }
    outVert.position = uniforms.mvpMatrix * float4(newPos,1.0);

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
