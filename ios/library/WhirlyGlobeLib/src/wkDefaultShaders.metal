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
using namespace metal;

struct VertexPCN
{
    float3 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float3 normal [[attribute(2)]];
};

struct Uniforms
{
    float4x4 mvpMatrix;
    float4x4 mvMatrix;
    float4x4 mvNormalMatrix;
    float fade;
};

struct ProjVertex
{
    float4 position [[position]];
    float4 color;
    float dotProd;
};

// Vertex shader for simple line on the globe
vertex ProjVertex vertexLine(
    VertexPCN vert [[stage_in]],
    constant Uniforms &uniforms [[buffer(1)]])
{
    ProjVertex outVert;
    
    float4 pt = uniforms.mvMatrix * float4(vert.position, 1.0);
    pt /= pt.w;
    float4 testNorm = uniforms.mvNormalMatrix * float4(vert.normal,0.0);
    outVert.dotProd = dot(-pt.xyz,testNorm.xyz);
    outVert.color = vert.color * uniforms.fade;
    outVert.position = uniforms.mvpMatrix * float4(vert.position,1.0);
    
    return outVert;
}

fragment float4 framentLine(
    ProjVertex in [[stage_in]])
{
    if (in.dotProd <= 0.0)
        discard_fragment();
    return in.color;
}
