/*
 *  MaplyAtmosphere.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/30/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import <WhirlyGlobe.h>
#import "MaplyAtmosphere.h"
#import "MaplyShape.h"
#import "MaplyShader_private.h"
#import "MaplyActiveObject_private.h"

#define k_v3CameraPos "v3CameraPos"
#define k_v3LightPos "v3LightPos"
#define k_v3InvWavelength "v3InvWavelength"
#define k_fCameraHeight "fCameraHeight"
#define k_fCameraHeight2 "fCameraHeight2"
#define k_fInnerRadius "fInnerRadius"
#define k_fOuterRadius "fOuterRadius"
#define k_fOuterRadius2 "fOuterRadius2"
#define k_fKrESun "fKrESun"
#define k_fKmESun "fKmESun"
#define k_fKr4PI "fKr4PI"
#define k_fKm4PI "fKm4PI"
#define k_fScale "fScale"
#define k_fScaleDepth "fScaleDepth"
#define k_fScaleOverScaleDepth "fScaleOverScaleDepth"
#define k_nSamples "nSamples"
#define k_fSamples "fSamples"
#define k_fg "fg"
#define k_fg2 "fg2"
#define k_fExposure "fExposure"

using namespace WhirlyKit;
using namespace Eigen;

static const char *vertexShaderTri =
"precision highp float;\n"
"\n"
"uniform mat4  u_mvpMatrix;\n"
"uniform vec3 v3CameraPos;\n"
"\n"
"uniform vec3 v3LightPos;\n"
"uniform vec3 v3InvWavelength;\n"
"uniform float fCameraHeight;\n"
"uniform float fCameraHeight2;\n"
"uniform float fInnerRadius;\n"
"uniform float fOuterRadius;\n"
"uniform float fOuterRadius2;\n"
"\n"
"uniform float fKrESun;\n"
"uniform float fKmESun;\n"
"uniform float fKr4PI;\n"
"uniform float fKm4PI;\n"
"\n"
"uniform float fScale;\n"
"uniform float fScaleDepth;\n"
"uniform float fScaleOverScaleDepth;\n"
"uniform int nSamples;\n"
"uniform float fSamples;\n"
"\n"
"attribute vec3 a_position;\n"
"\n"
"varying vec3 v3Direction;"
"varying vec4 v4RayleighColor;\n"
"varying vec4 v4MieColor;\n"
"\n"
"float getNearIntersection(vec3 pos, vec3 ray, float fDist2, float fRad2)\n"
"{\n"
"  float B = 2.0 * dot(pos, ray);\n"
"  float C = fDist2 - fRad2;\n"
"  float fDet = max(0.0, B*B - 4.0 * C);\n"
"  return 0.5 * (-B - sqrt(fDet));\n"
"}\n"
"\n"
"float scale(float fCos)\n"
"{\n"
"  float x = 1.0 - fCos;\n"
"  return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));\n"
"}\n"
"\n"
"void main()\n"
"{"
"   vec3 v3Pos = a_position.xyz;\n"
"   vec3 v3Ray = v3Pos - v3CameraPos;\n"
"   float fFar = length(v3Ray);\n"
"   v3Ray /= fFar;\n"
"\n"
"   float fNear = getNearIntersection(v3CameraPos, v3Ray, fCameraHeight2, fOuterRadius2);\n"
"\n"
"   vec3 v3Start = v3CameraPos + v3Ray * fNear;\n"
"   fFar -= fNear;\n"
"\n"
"   float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;\n"
"   float fStartDepth = exp(-1.0/fScaleDepth);\n"
"   float fStartOffset = fStartDepth * scale(fStartAngle);\n"
"\n"
"   float fSampleLength = fFar / fSamples;\n"
"   float fScaledLength = fSampleLength * fScale;\n"
"   vec3 v3SampleRay = v3Ray * fSampleLength;\n"
"   vec3 v3SamplePoint = v3Start + (v3SampleRay * 0.5);\n"
"\n"
"   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"
"   for (int i=0; i<nSamples; i++)\n"
"   {\n"
"     float fHeight = length(v3SamplePoint);\n"
"     float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));\n"
"     float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;\n"
"     float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;\n"
"     float fScatter = (fStartOffset + fDepth *(scale(fLightAngle) - scale(fCameraAngle)));\n"
"     vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));\n"
"     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"
"     v3SamplePoint += v3SampleRay;\n"
"   }\n"
"\n"
"   v4MieColor = vec4(v3FrontColor * fKmESun, 1.0);\n"
"   v4RayleighColor = vec4(v3FrontColor * (v3InvWavelength * fKrESun), 1.0);\n"
"   v3Direction = v3CameraPos - v3Pos;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;

static const char *fragmentShaderTri =
"precision highp float;\n"
"\n"
"uniform vec3 v3LightPos;\n"
"uniform float fg;\n"
"uniform float fg2;\n"
"uniform float fExposure;\n"
"\n"
"varying vec3 v3Direction;"
"varying vec4 v4RayleighColor;\n"
"varying vec4 v4MieColor;\n"
"\n"
"void main()\n"
"{\n"
"  float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);\n"
"  float fCos2 = fCos*fCos;\n"
"  float rayPhase = 0.75 * (1.0 + fCos2);\n"
"  float miePhase = 1.5 * ((1.0 - fg2) / (2.0 + fg2)) * (1.0 + fCos2) / pow(1.0 + fg2 - 2.0*fg*fCos, 1.5);\n"
"  vec4 color = (rayPhase * v4RayleighColor) + (miePhase * v4MieColor);\n"
"  color.a = color.b;\n"
"  gl_FragColor = vec4(vec3(1.0) - exp(-fExposure * color.xyz),color.a);\n"
"}\n"
;

static const double AtmosphereHeight = 1.025;
#define kAtmosphereShader @"Atmosphere Shader"

@interface SunUpdater : MaplyActiveObject
@end

@implementation SunUpdater
{
    bool changed;
    bool started;
    MaplyCoordinate3d sunDir;
    MaplyShader *shader;
}

- (id)initWithShader:(MaplyShader *)inShader viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithViewController:viewC];
    changed = true;
    started = false;
    shader = inShader;
    
    return self;
}

- (bool)hasUpdate
{
    return changed;
}

- (void)setSunDirection:(MaplyCoordinate3d)inSunDir
{
    sunDir = inSunDir;
    changed = true;
}

// Thanks to: http://stainlessbeer.weebly.com/planets-9-atmospheric-scattering.html
//  for the parameter values.

- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    [frameInfo.sceneRenderer useContext];
    [frameInfo.sceneRenderer forceDrawNextFrame];
    glUseProgram(shader.program->getProgram());
    
    // Set the parameters that never change
    if (started)
    {
        //    "uniform vec3 v3InvWavelength;"
        shader.program->setUniform(k_v3InvWavelength, Vector3f(1.0f / (float)pow(0.650f, 4),
                                                               1.0f / (float)pow(0.570f, 4),
                                                               1.0f / (float)pow(0.475f, 4)));
        //    "uniform float fInnerRadius;"
        shader.program->setUniform(k_fInnerRadius, (float)1.0);
        //    "uniform float fOuterRadius;"
        shader.program->setUniform(k_fOuterRadius, (float)AtmosphereHeight);
        //    "uniform float fOuterRadius2;"
        shader.program->setUniform(k_fOuterRadius2, (float)(AtmosphereHeight*AtmosphereHeight));
        //    "uniform float fKrESun;"
        shader.program->setUniform(k_fKrESun, (float)(0.0000025f * 10));
        //    "uniform float fKmESun;"
        shader.program->setUniform(k_fKmESun, (float)(0.00015f * 10));
        
        //    "uniform float fKr4PI;"
        shader.program->setUniform(k_fKr4PI, (float)(0.0025f * 4 * M_PI));
        //    "uniform float fKm4PI;"
        shader.program->setUniform(k_fKm4PI, (float)(0.0015f * 4 * M_PI));

        float scale = 1.0f / (AtmosphereHeight - 1.0);
        //    "uniform float fScale;"
        shader.program->setUniform(k_fScale, scale);
        //    "uniform float fScaleDepth;"
        shader.program->setUniform(k_fScaleDepth, 0.25f);
        //    "uniform float fScaleOverScaleDepth;"
        shader.program->setUniform(k_fScaleOverScaleDepth, scale / 0.5f);
        //    "uniform int nSamples;"
        shader.program->setUniform(k_nSamples, 2);
        //    "uniform float fSamples;"
        shader.program->setUniform(k_fSamples, 2.0f);
        
        //    "uniform float fg;"
        shader.program->setUniform(k_fg, (float)-0.90);
        //    "uniform float fg2;"
        shader.program->setUniform(k_fg2, (float)0.81f);
        //    "uniform float fExposure;"
        shader.program->setUniform(k_fExposure, (float)2.0);
    }
    
    //    "uniform vec3 v3CameraPos;"
    Vector3f cameraPos = frameInfo.eyeVec;
    cameraPos *= (1.0+frameInfo.heightAboveSurface);
    shader.program->setUniform(k_v3CameraPos, cameraPos);
    
    //    "uniform vec3 v3LightPos;"
    shader.program->setUniform(k_v3LightPos, (Vector3f)(Vector3f(sunDir.x,sunDir.y,sunDir.z)));
    //    "uniform float fCameraHeight;"
    float height = frameInfo.heightAboveSurface;
    shader.program->setUniform(k_fCameraHeight, height);
    //    "uniform float fCameraHeight2;"
    shader.program->setUniform(k_fCameraHeight2, height*height);
    
    changed = false;
    started = true;
    
    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
}

@end

@implementation MaplyAtmosphere
{
    WhirlyGlobeViewController __weak *viewC;
    MaplyComponentObject *compObj;
    MaplyShader *shader;
    SunUpdater *sunUpdater;
}

- (id)initWithViewC:(WhirlyGlobeViewController *)inViewC
{
    self = [super init];
    
    viewC = inViewC;

    // Atmosphere shader
    shader = [self setupShader];
    
    if (!shader)
        return nil;

    // Make a sphere for the outer atmosphere
    MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
    sphere.center = MaplyCoordinateMake(0, 0);
    sphere.height = -1.0;
    sphere.radius = AtmosphereHeight;
    compObj = [viewC addShapes:@[sphere] desc:@{kMaplyZBufferRead: @(NO),
                                                kMaplyZBufferWrite: @(NO),
                                                kMaplyShapeSampleX: @(60),
                                                kMaplyShapeSampleY: @(30),
                                                kMaplyShader: kAtmosphereShader}];
    
    sunUpdater = [[SunUpdater alloc] initWithShader:shader viewC:viewC];
    [viewC addActiveObject:sunUpdater];
    
    return self;
}

- (void)setSunDirection:(MaplyCoordinate3d)sunDir
{
    if (sunUpdater)
        [sunUpdater setSunDirection:sunDir];
}

- (MaplyShader *)setupShader
{
    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTri] viewC:viewC];
    if (!theShader.valid)
        return nil;
    if (theShader)
        [viewC addShaderProgram:theShader sceneName:kAtmosphereShader];
    
    return theShader;
}

- (void)removeFromViewC
{
    if (compObj)
        [viewC removeObject:compObj];
    compObj = nil;
    if (sunUpdater)
        [viewC removeActiveObject:sunUpdater];
    sunUpdater = nil;
    // Note: Should remove shader
}

@end
