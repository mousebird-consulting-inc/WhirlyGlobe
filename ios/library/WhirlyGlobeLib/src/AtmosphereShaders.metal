/*  AtmosphereShaders.metal
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

extern float calculateFade(constant Uniforms &, constant UniformDrawStateA &);

float scale(float fCos)
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
    const float startOffset = startDepth * scale(startAngle) * scaleDepth;
    
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
        const float scatter = startOffset + depth * scaleDepth * (scale(lightAngle) - scale(cameraAngle));
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

