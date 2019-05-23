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

// Vertices with position, color, and normal
struct VertexA
{
    float3 a_position [[attribute(0)]];
    float4 a_color [[attribute(1)]];
    float3 a_normal [[attribute(2)]];
};

// Position, color, and dot project (for backface checking)
struct ProjVertexA
{
    float4 position [[position]];
    float4 color;
    float dotProd;
};

// Vertex shader for simple line on the globe
vertex ProjVertexA vertexLineOnly_globe(
    VertexA vert [[stage_in]],
    constant UniformsTri &uniforms [[buffer(9)]],
    constant UniformDrawStateA &uniDrawState [[buffer(10)]])
{
    ProjVertexA outVert;
    
    float4 pt = uniforms.mvMatrix * float4(vert.a_position, 1.0);
    pt /= pt.w;
    float4 testNorm = uniforms.mvNormalMatrix * float4(vert.a_normal,0.0);
    outVert.dotProd = dot(-pt.xyz,testNorm.xyz);
    outVert.color = float4(vert.a_color) * uniDrawState.fade;
    outVert.position = uniforms.mvpMatrix * float4(vert.a_position,1.0);
    
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

// Just position and color
struct ProjVertexB
{
    float4 position [[position]];
    float4 color;
};

// Vertex shader for simple line on the flat map (no backface checking)
vertex ProjVertexB vertexLineOnly_flat(
    VertexA vert [[stage_in]],
    constant UniformsTri &uniforms [[buffer(9)]],
    constant UniformDrawStateA &uniDrawState [[buffer(10)]])
{
    ProjVertexB outVert;
    
    outVert.color = float4(vert.a_color) * uniDrawState.fade;
    outVert.position = uniforms.mvpMatrix * float4(vert.a_position,1.0);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentLineOnly_flat(
    ProjVertexB vert [[stage_in]],
    constant UniformsTri &uniforms [[buffer(9)]])
{
    return vert.color;
}

// Ye olde triangle vertex
struct VertexTriA
{
    float3 a_position [[attribute(0)]];
    float4 a_color [[attribute(1)]];
    float3 a_normal [[attribute(2)]];
    float2 a_texCoord [[attribute(3)]];
};

// Output vertex to the fragment shader
struct ProjVertexTriA {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
};

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
    
    return float4(ambient.xyz * lighting.mat.ambient.xyz * color.xyz + diffuse.xyz * color.xyz,1.0);
}

// Simple vertex shader for triangle with no lighting
vertex ProjVertexTriA vertexTri_noLight(VertexTriA vert [[stage_in]],
                                        constant UniformsTri &uniforms [[buffer(8)]],
                                        constant UniformDrawStateA &uniDrawState [[buffer(9)]],
                                        constant TexIndirect &texIndirect [[buffer(11)]])
{
    ProjVertexTriA outVert;
    
    outVert.position = uniforms.mvpMatrix * float4(vert.a_position,1.0);
    outVert.color = float4(vert.a_color) * uniDrawState.fade;
    outVert.texCoord = resolveTexCoords(vert.a_texCoord,texIndirect);
    
    return outVert;
}

// Simple vertex shader for triangle with basic lighting
vertex ProjVertexTriA vertexTri_light(VertexTriA vert [[stage_in]],
                                      constant UniformsTri &uniforms [[buffer(8)]],
                                      constant UniformDrawStateA &uniDrawState [[buffer(9)]],
                                      constant Lighting &lighting [[buffer(10)]],
                                      constant TexIndirect &texIndirect [[buffer(11)]])
{
    ProjVertexTriA outVert;
    
    outVert.position = uniforms.mvpMatrix * float4(vert.a_position,1.0);
    outVert.color = resolveLighting(vert.a_position,
                                    vert.a_normal,
                                    float4(vert.a_color),
                                    lighting,
                                    uniforms.mvpMatrix);
    outVert.texCoord = resolveTexCoords(vert.a_texCoord,texIndirect);
    
    return outVert;
}

// Simple fragment shader for lines on flat map
fragment float4 fragmentTri_noLight(ProjVertexTriA vert [[stage_in]],
                                      constant UniformsTri &uniforms [[buffer(8)]],
                                      texture2d<float,access::sample> tex [[texture(0)]])
{
//    if (tex.) {
//        constexpr sampler sampler2d(coord::normalized, filter::linear);
//        return vert.color * tex.sample(sampler2d, vert.texCoord);
//    } else {
        return vert.color;
//    }
}
