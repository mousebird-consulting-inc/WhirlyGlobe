/*  Atmosphere.metal
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 2/2/22.
 *  Copyright 2022 mousebird consulting. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 *  file except in compliance with the License. You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
 *  OF ANY KIND, either express or implied. See the License for the specific language
 *  governing permissions and limitations under the License.
 */

#include <metal_stdlib>
#import "../include/DefaultShadersMTL.h"
#import "../include/AtmosphereShadersMTL.h"

using namespace metal;
using namespace WhirlyKitShader;
using namespace WhirlyKitAtmosphereShader;

// Arguments we're expecting for the shader

struct VertexTriArgBuffer {
    UniformDrawStateA uniDrawState  [[ id(WKSUniformDrawStateEntry) ]];
    AtmosShaderVertUniforms varUni      [[ id(AtmosUniformVertEntry) ]];
};

struct FragTriArgBuffer {
    UniformDrawStateA uniDrawState [[ id(WKSUniformDrawStateEntry) ]];
    AtmosShaderFragUniforms varUni [[ id(AtmosUniformFragEntry) ]];
};

struct ProjVertexTriAtmos {
    float4 position [[invariant]] [[position]];
    float3 lightPos;
    float3 direction;
    float3 rayleighColor;
    float3 mieColor;
};

float scale(float fCos)
{
    const float x = 1.0 - fCos;
    return exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

vertex ProjVertexTriAtmos vertexTri_atmos(
        VertexTriA vert [[stage_in]],
        uint vertID [[vertex_id]],
        constant Uniforms &uniforms [[ buffer(WKSVertUniformArgBuffer) ]],
        constant Lighting &lighting [[ buffer(WKSVertLightingArgBuffer) ]],
        constant VertexTriArgBuffer & vertArgs [[buffer(WKSVertexArgBuffer)]])
{
    ProjVertexTriAtmos out;

    const float3 camPos = vertArgs.varUni.cameraPos;//(vertArgs.uniDrawState.singleMat * float4(vertArgs.varUni.cameraPos,1.0)).xyz;
    const float3 lightPos = vertArgs.varUni.lightPos;//(vertArgs.uniDrawState.singleMat * float4(vertArgs.varUni.lightPos,1.0)).xyz;
    const float3 vertPos = (vertArgs.uniDrawState.singleMat * float4(vert.position,1.0)).xyz;
    if (vertArgs.uniDrawState.clipCoords) {
        out.position = float4(vertPos,1);
    } else {
        out.position = uniforms.pMatrix * (uniforms.mvMatrix * float4(vertPos,1) +
                                           uniforms.mvMatrixDiff * float4(vertPos,1));
    }
    out.lightPos = lightPos;

    const float3 pos = vert.position;//vertPos.xyz;////out.position;
    float3 ray = pos - camPos;
    float far = length(ray);
    ray /= far;

    const float B = 2.0 * dot(camPos, ray);
    const float C = vertArgs.varUni.cameraHeight2 - vertArgs.varUni.outerRadius2;
    const float det = max(0.0, B * B - 4.0 * C);
    const float near = 0.5 * (-B - sqrt(det));

    const float3 start = camPos + ray * near;
    far -= near;

    const float scaleDepth = vertArgs.varUni.scaleDepth;
    const float startAngle = dot(ray, start) / vertArgs.varUni.outerRadius;
    const float startDepth = exp(-1.0 / scaleDepth);
    const float startOffset = startDepth * scale(startAngle) * scaleDepth;
    
    const float sampleLength = far / vertArgs.varUni.samples;
    const float scaledLength = sampleLength * vertArgs.varUni.scale;
    const float3 sampleRay = ray * sampleLength;
    float3 samplePoint = start + sampleRay * 0.5;

    float3 frontColor = float3(0, 0, 0);
    for (int i=0; i<vertArgs.varUni.nSamples; i++)
    {
        const float height = length(samplePoint);
        const float depth = exp(vertArgs.varUni.scaleOverScaleDepth * (vertArgs.varUni.innerRadius - height));
        const float lightAngle = dot(lightPos, samplePoint) / height;
        const float cameraAngle = dot(ray, samplePoint) / height;
        const float scatter = startOffset + depth * ((scale(lightAngle) - scale(cameraAngle)*scaleDepth));
        const float3 attenuate = exp(-scatter * (vertArgs.varUni.invWavelength * vertArgs.varUni.kr4PI + vertArgs.varUni.km4PI));
        frontColor += attenuate * (depth * scaledLength);
        samplePoint += sampleRay;
    }

    out.mieColor = frontColor * vertArgs.varUni.kmESun;
    out.rayleighColor = frontColor * (vertArgs.varUni.invWavelength * vertArgs.varUni.krESun + vertArgs.varUni.km4PI);
    out.direction = camPos - pos;
 
    return out;
}

fragment float4 fragmentTri_atmos(
        ProjVertexTriAtmos vert [[stage_in]],
        constant Uniforms &uniforms [[ buffer(WKSFragUniformArgBuffer) ]],
        constant FragTriArgBuffer &fragArgs [[ buffer(WKSFragmentArgBuffer) ]])
{
    const float g = fragArgs.varUni.g;
    const float g2 = fragArgs.varUni.g2;
    const float cos = dot(vert.lightPos /*fragArgs.varUni.lightPos*/, normalize(vert.direction)) / length(vert.direction);
    const float cos2 = cos * cos;
    const float rayPhase = 0.75 + 0.75 * cos2;
    const float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + cos2) / pow(1.0 + g2 - 2.0*g*cos, 1.5);
    
    float3 color = rayPhase * vert.rayleighColor + miePhase * vert.mieColor;
    color = 1.0 - exp(color * -fragArgs.varUni.exposure);

    return float4(color,color.b);
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

