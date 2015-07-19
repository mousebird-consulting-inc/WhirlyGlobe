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

#define k_v3CameraPos "u_v3CameraPos"
#define k_v3LightPos "u_v3LightPos"
#define k_v3InvWavelength "u_v3InvWavelength"
#define k_fCameraHeight "u_fCameraHeight"
#define k_fCameraHeight2 "u_fCameraHeight2"
#define k_fInnerRadius "u_fInnerRadius"
#define k_fInnerRadius2 "u_fInnerRadius2"
#define k_fOuterRadius "u_fOuterRadius"
#define k_fOuterRadius2 "u_fOuterRadius2"
#define k_fScale "u_fScale"
#define k_fScaleDepth "u_fScaleDepth"
#define k_fScaleOverScaleDepth "u_fScaleOverScaleDepth"

using namespace WhirlyKit;
using namespace Eigen;

static const char *vertexShaderTri =
"precision highp float;\n"
"\n"
"uniform mat4  u_mvpMatrix;\n"
"uniform vec3 u_v3CameraPos;\n"
"uniform vec3 u_v3LightPos;\n"
"uniform float u_fCameraHeight2;\n"
"uniform float u_fInnerRadius;\n"
"uniform float u_fInnerRadius2;\n"
"uniform float u_fOuterRadius;\n"
"uniform float u_fOuterRadius2;\n"
"\n"
"const float pi = 3.14159265359;\n"
"const float Kr = 0.0025;\n"
"const float fKr4PI = Kr * 4.0 * pi;\n"
"const float Km = 0.0010;\n"
"const float fKm4PI = Km * 4.0 * pi;\n"
"const float ESun = 20.0;\n"
"const float fKmESun = Km * ESun;\n"
"const float fKrESun = Kr * ESun;\n"
"const vec3 v3InvWavelength = vec3(5.6020447463,9.4732844379,19.6438026201);\n"
"const float fSamples = 3.0;\n"
"const int nSamples = 3;\n"
"\n"
"uniform float u_fScale;\n"
"uniform float u_fScaleDepth;\n"
"uniform float u_fScaleOverScaleDepth;\n"
"const float fScaleDepth = 0.25;\n"
"\n"
"attribute vec3 a_position;\n"
"\n"
"varying highp vec3 v3Direction;"
"varying highp vec3 v3RayleighColor;\n"
"varying highp vec3 v3MieColor;\n"
"\n"
"float scale(float fCos)\n"
"{\n"
"  float x = 1.0 - fCos;\n"
"  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));\n"
"}\n"
"\n"
"void main()\n"
"{"
"   vec3 v3Pos = a_position.xyz;\n"
"   vec3 v3Ray = v3Pos - u_v3CameraPos;\n"
"   float fFar = length(v3Ray);\n"
"   v3Ray /= fFar;\n"
"\n"
"  float B = 2.0 * dot(u_v3CameraPos, v3Ray);\n"
"  float C = u_fCameraHeight2 - u_fOuterRadius2;\n"
"  float fDet = max(0.0, B*B - 4.0 * C);\n"
"  float fNear = 0.5 * (-B - sqrt(fDet));\n"
"\n"
"   vec3 v3Start = u_v3CameraPos + v3Ray * fNear;\n"
"   fFar -= fNear;\n"
"\n"
"   float fStartAngle = dot(v3Ray, v3Start) / u_fOuterRadius;\n"
"   float fStartDepth = exp(-1.0/u_fScaleDepth);\n"
"   float fStartOffset = fStartDepth * scale(fStartAngle);\n"
"\n"
"   float fSampleLength = fFar / fSamples;\n"
"   float fScaledLength = fSampleLength * u_fScale;\n"
"   vec3 v3SampleRay = v3Ray * fSampleLength;\n"
"   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;\n"
"\n"
"   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"
"   vec3 v3Attenuate;\n"
"   for (int i=0; i<nSamples; i++)\n"
"   {\n"
"     float fHeight = length(v3SamplePoint);\n"
"     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));\n"
"     float fLightAngle = dot(u_v3LightPos, v3SamplePoint) / fHeight;\n"
"     float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;\n"
"     float fScatter = (fStartOffset + fDepth *(scale(fLightAngle) - scale(fCameraAngle)));\n"
"     v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));\n"
"     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"
"     v3SamplePoint += v3SampleRay;\n"
"   }\n"
"\n"
"   v3MieColor = v3FrontColor * fKmESun;\n"
"   v3RayleighColor = v3FrontColor * (v3InvWavelength * fKrESun);\n"
"   v3Direction = u_v3CameraPos - v3Pos;\n"
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
"uniform vec3 u_v3LightPos;\n"
"\n"
"varying highp vec3 v3Direction;"
"varying highp vec3 v3RayleighColor;\n"
"varying highp vec3 v3MieColor;\n"
"\n"
"void main()\n"
"{\n"
"  float fCos = dot(u_v3LightPos, normalize(v3Direction)) / length(v3Direction);\n"
"  float fCos2 = fCos*fCos;\n"
"  float rayPhase = 0.75 + 0.75*fCos2;\n"
"  float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);\n"
"  vec3 color = rayPhase * v3RayleighColor + miePhase * v3MieColor;\n"
"  color = 1.0 - exp(color * -fExposure);"
"  gl_FragColor = vec4(color,color.b);\n"
"}\n"
;

static const double AtmosphereHeight = 1.05;
#define kAtmosphereShader @"Atmosphere Shader"

@interface SunUpdater : MaplyActiveObject
@end

@implementation SunUpdater
{
    bool changed;
    bool started;
    MaplyCoordinate3d sunPos;
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

- (void)setSunPosition:(MaplyCoordinate3d)inSunPos
{
    sunPos = inSunPos;
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
    if (!started)
    {
        shader.program->setUniform(k_fInnerRadius, (float)1.0);
        shader.program->setUniform(k_fInnerRadius2, (float)1.0);
        shader.program->setUniform(k_fOuterRadius, (float)AtmosphereHeight);
        shader.program->setUniform(k_fOuterRadius2, (float)(AtmosphereHeight*AtmosphereHeight));
        float scale = 1.0f / (AtmosphereHeight - 1.0);
        shader.program->setUniform(k_fScale, scale);
        float scaleDepth = 0.25;
        shader.program->setUniform(k_fScaleDepth, scaleDepth);
        shader.program->setUniform(k_fScaleOverScaleDepth, scale / scaleDepth);
    }
    
    Vector3d cameraPos = frameInfo.eyePos;
    Vector4d sunDir4d = Vector4d(sunPos.x,sunPos.y,sunPos.z,1.0);
    sunDir4d /= sunDir4d.w();
    Vector3d sunDir3d(sunDir4d.x(),sunDir4d.y(),sunDir4d.z());
    sunDir3d.normalize();

    shader.program->setUniform(k_v3CameraPos, Vector3f(cameraPos.x(),cameraPos.y(),cameraPos.z()));
    double cameraHeight = cameraPos.norm();
    shader.program->setUniform(k_fCameraHeight, (float)cameraHeight);
    shader.program->setUniform(k_fCameraHeight2, (float)(cameraHeight*cameraHeight));
    shader.program->setUniform(k_v3LightPos, Vector3f(sunDir3d.x(),sunDir3d.y(),sunDir3d.z()));
    
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
    
    [self complexAtmosphere];
    
    return self;
}

- (void)complexAtmosphere
{
    // Make a sphere for the outer atmosphere
    MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
    sphere.center = MaplyCoordinateMake(0, 0);
    sphere.height = -1.0;
    sphere.radius = AtmosphereHeight;
    compObj = [viewC addShapes:@[sphere] desc:@{kMaplyZBufferRead: @(NO),
                                                kMaplyZBufferWrite: @(NO),
                                                kMaplyShapeSampleX: @(120),
                                                kMaplyShapeSampleY: @(60),
                                                kMaplyShapeInsideOut: @(YES),
                                                kMaplyShapeCenterX: @(0.0),
                                                kMaplyShapeCenterY: @(0.0),
                                                kMaplyShapeCenterZ: @(0.0),
                                                kMaplyDrawPriority: @(0),
                                                kMaplyShader: kAtmosphereShader}];
    
    sunUpdater = [[SunUpdater alloc] initWithShader:shader viewC:viewC];
    [viewC addActiveObject:sunUpdater];
}

- (void)simpleAtmosphere
{
    
}

- (void)setSunPosition:(MaplyCoordinate3d)sunPos
{
    if (sunUpdater)
        [sunUpdater setSunPosition:sunPos];
}

- (MaplyShader *)setupShader
{
//    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTri] viewC:viewC];
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
