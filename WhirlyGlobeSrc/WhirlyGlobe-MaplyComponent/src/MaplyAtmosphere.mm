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
"uniform float fCameraHeight;\n"
"uniform float fCameraHeight2;\n"
"uniform float fInnerRadius;\n"
"uniform float fOuterRadius;\n"
"uniform float fOuterRadius2;\n"
"\n"
"const float pi = 3.14159265359;\n"
"const float Kr = 0.0025;\n"
"const float fKr4PI = Kr * 4.0 * pi;\n"
"const float Km = 0.0015;\n"
"const float fKm4PI = Km * 4.0 * pi;\n"
"const float ESun = 15.0;\n"
"const float fKmESun = Km * ESun;\n"
"const float fKrESun = Kr * ESun;\n"
"const vec3 v3InvWavelength = vec3(1.0 / pow(0.650, 4.0),1.0 / pow(0.570, 4.0),1.0 / pow(0.475, 4.0));\n"
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
"varying vec3 v3RayleighColor;\n"
"varying vec3 v3MieColor;\n"
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
"   v3MieColor = v3FrontColor * fKmESun;\n"
"   v3RayleighColor = v3FrontColor * (v3InvWavelength * fKrESun);\n"
"   v3Direction = v3CameraPos - v3Pos;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;

static const char *fragmentShaderTri =
"precision highp float;\n"
"\n"
"const float g = -0.95;\n"
"const float g2 = g * g;\n"
"const float fExposure = 2.0;\n"
"uniform vec3 v3LightPos;\n"
"\n"
"varying vec3 v3Direction;"
"varying vec3 v3RayleighColor;\n"
"varying vec3 v3MieColor;\n"
"\n"
"void main()\n"
"{\n"
"  float fCos = dot(v3LightPos, v3Direction) / length(v3Direction);\n"
"  float fCos2 = fCos*fCos;\n"
"  float rayPhase = 0.75 * (1.0 + fCos2);\n"
"  float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);\n"
"  vec3 color = (rayPhase * v3RayleighColor) + (miePhase * v3MieColor);\n"
"  gl_FragColor = vec4(vec3(1.0) - exp(-fExposure * color),color.b);\n"
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
//        shader.program->setUniform(k_v3InvWavelength, Vector3f(1.0f / (float)pow(0.650f, 4),
//                                                               1.0f / (float)pow(0.570f, 4),
//                                                               1.0f / (float)pow(0.475f, 4)));
        //    "uniform float fInnerRadius;"
        shader.program->setUniform(k_fInnerRadius, (float)1.0);
        //    "uniform float fOuterRadius;"
        shader.program->setUniform(k_fOuterRadius, (float)AtmosphereHeight);
        //    "uniform float fOuterRadius2;"
        shader.program->setUniform(k_fOuterRadius2, (float)(AtmosphereHeight*AtmosphereHeight));
        float scale = 1.0f / (AtmosphereHeight - 1.0);
        //    "uniform float fScale;"
        shader.program->setUniform(k_fScale, scale);
        //    "uniform float fScaleDepth;"
        float scaleDepth = 0.25;
        shader.program->setUniform(k_fScaleDepth, scaleDepth);
        //    "uniform float fScaleOverScaleDepth;"
        shader.program->setUniform(k_fScaleOverScaleDepth, scale / scaleDepth);
        //    "uniform int nSamples;"
        shader.program->setUniform(k_nSamples, 2);
        //    "uniform float fSamples;"
        shader.program->setUniform(k_fSamples, 2.0f);
    }
    
    //    "uniform vec3 v3CameraPos;"
    Matrix4d invMat = frameInfo.viewAndModelMat4d.inverse();
    Vector4d cameraPos = invMat * Vector4d(0,0,0,1.0);
    cameraPos /= cameraPos.w();
    shader.program->setUniform(k_v3CameraPos, Vector3f(cameraPos.x(),cameraPos.y(),cameraPos.z()));
    
    //    "uniform vec3 v3LightPos;"
    shader.program->setUniform(k_v3LightPos, Vector3f(sunDir.x,sunDir.y,sunDir.z));
    //    "uniform float fCameraHeight;"
    float height = frameInfo.heightAboveSurface+1.0;
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
                                                kMaplyDrawPriority: @(0),
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
