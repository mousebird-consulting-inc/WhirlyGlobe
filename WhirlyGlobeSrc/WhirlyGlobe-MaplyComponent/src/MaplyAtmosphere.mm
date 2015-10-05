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
#define k_Kr "u_Kr"
#define k_Kr4PI "u_Kr4PI"
#define k_Km "u_Km"
#define k_Km4PI "u_Km4PI"
#define k_ESun "u_ESun"
#define k_KmESun "u_KmESun"
#define k_KrESun "u_KrESun"
#define k_v3InvWavelength "u_v3InvWavelength"
#define k_fSamples "u_fSamples"
#define k_nSamples "u_nSamples"
#define k_g "g"
#define k_g2 "g2"
#define k_fExposure "fExposure"

using namespace WhirlyKit;
using namespace Eigen;

static const char *vertexShaderAtmosTri =
"precision highp float;\n"
"\n"
"uniform mat4  u_mvpMatrix;\n"
"uniform vec3 u_v3CameraPos;\n"
"uniform float u_fCameraHeight2;\n"
"uniform vec3 u_v3LightPos;\n"
"\n"
"uniform float u_fInnerRadius;\n"
"uniform float u_fInnerRadius2;\n"
"uniform float u_fOuterRadius;\n"
"uniform float u_fOuterRadius2;\n"
"uniform float u_fScale;\n"
"uniform float u_fScaleDepth;\n"
"uniform float u_fScaleOverScaleDepth;\n"
"\n"
"uniform float u_Kr;\n"
"uniform float u_Kr4PI;\n"
"uniform float u_Km;\n"
"uniform float u_Km4PI;\n"
"uniform float u_ESun;\n"
"uniform float u_KmESun;\n"
"uniform float u_KrESun;\n"
"uniform vec3 u_v3InvWavelength ;\n"
"uniform float u_fSamples;\n"
"uniform int u_nSamples;\n"
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
"   float fSampleLength = fFar / u_fSamples;\n"
"   float fScaledLength = fSampleLength * u_fScale;\n"
"   vec3 v3SampleRay = v3Ray * fSampleLength;\n"
"   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;\n"
"\n"
"   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"
"   vec3 v3Attenuate;\n"
"   for (int i=0; i<u_nSamples; i++)\n"
"   {\n"
"     float fHeight = length(v3SamplePoint);\n"
"     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));\n"
"     float fLightAngle = dot(u_v3LightPos, v3SamplePoint) / fHeight;\n"
"     float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;\n"
"     float fScatter = (fStartOffset + fDepth *(scale(fLightAngle) - scale(fCameraAngle)));\n"
"     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));\n"
"     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"
"     v3SamplePoint += v3SampleRay;\n"
"   }\n"
"\n"
"   v3MieColor = v3FrontColor * u_KmESun;\n"
"   v3RayleighColor = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_Km4PI);\n"
"   v3Direction = u_v3CameraPos - v3Pos;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;


static const char *fragmentShaderAtmosTri =
"precision highp float;\n"
"\n"
"uniform float g;\n"
"uniform float g2;\n"
"uniform float fExposure;\n"
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

#define kAtmosphereShader @"Atmosphere Shader"

static const char *vertexShaderGroundTri =
"precision highp float;\n"
"\n"
"uniform mat4  u_mvpMatrix;\n"
"uniform vec3 u_v3CameraPos;\n"
"uniform float u_fCameraHeight2;\n"
"uniform vec3 u_v3LightPos;\n"
"\n"
"uniform float u_fInnerRadius;\n"
"uniform float u_fInnerRadius2;\n"
"uniform float u_fOuterRadius;\n"
"uniform float u_fOuterRadius2;\n"
"uniform float u_fScale;\n"
"uniform float u_fScaleDepth;\n"
"uniform float u_fScaleOverScaleDepth;\n"
"\n"
"uniform float u_Kr;\n"
"uniform float u_Kr4PI;\n"
"uniform float u_Km;\n"
"uniform float u_Km4PI;\n"
"uniform float u_ESun;\n"
"uniform float u_KmESun;\n"
"uniform float u_KrESun;\n"
"uniform vec3 u_v3InvWavelength ;\n"
"uniform float u_fSamples;\n"
"uniform int u_nSamples;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec3 a_normal;\n"
"attribute vec2 a_texCoord0;\n"
"attribute vec2 a_texCoord1;\n"
"\n"
"varying mediump vec3 v_color;\n"
"varying mediump vec3 v_v3attenuate;\n"
"varying mediump vec2 v_texCoord0;"
"varying mediump vec2 v_texCoord1;\n"
"\n"
"float scale(float fCos)\n"
"{\n"
"  float x = 1.0 - fCos;\n"
"  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));\n"
"}\n"
"\n"
"void main()\n"
"{"
"   vec3 v3Pos = a_normal.xyz;\n"
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
"   float fDepth = exp((u_fInnerRadius - u_fOuterRadius) / u_fScaleDepth);\n"
"   float fCameraAngle = dot(-v3Ray, v3Pos) / length (v3Pos);\n"
"   float fLightAngle = dot(u_v3LightPos, v3Pos) / length(v3Pos);\n"
"   float fCameraScale = scale(fCameraAngle);\n"
"   float fLightScale = scale(fLightAngle);\n"
"   float fCameraOffset = fDepth*fCameraScale;\n"
"   float fTemp = (fLightScale + fCameraScale);\n"
"\n"
"   float fSampleLength = fFar / u_fSamples;\n"
"   float fScaledLength = fSampleLength * u_fScale;\n"
"   vec3 v3SampleRay = v3Ray * fSampleLength;\n"
"   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;\n"
"\n"
"   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);\n"
"   vec3 v3Attenuate;\n"
"   for (int i=0; i<u_nSamples; i++)\n"
"   {\n"
"     float fHeight = length(v3SamplePoint);\n"
"     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));\n"
"     float fScatter = fDepth*fTemp - fCameraOffset;\n"
"     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));\n"
"     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);\n"
"     v3SamplePoint += v3SampleRay;\n"
"   }\n"
"\n"
"   v_v3attenuate = v3Attenuate;\n"
"   v_color = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_KmESun);\n"
"   v_texCoord0 = a_texCoord0;\n"
"   v_texCoord1 = a_texCoord1;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);\n"
"}\n"
;

// Note: Not finished with these

static const char *fragmentShaderGroundTri =
"precision mediump float;\n"
"\n"
"uniform sampler2D s_baseMap0;\n"
"uniform sampler2D s_baseMap1;\n"
"\n"
"varying vec3      v_color;\n"
"varying vec2      v_texCoord0;\n"
"varying vec2      v_texCoord1;\n"
"varying vec3      v_v3attenuate;\n"
"\n"
"void main()\n"
"{\n"
"  vec3 dayColor = texture2D(s_baseMap0, v_texCoord0).xyz * v_v3attenuate;\n"
"  vec3 nightColor = texture2D(s_baseMap1, v_texCoord1).xyz * (1.0 - v_v3attenuate);\n"
"  gl_FragColor = vec4(v_color, 1.0) + vec4(dayColor + nightColor, 1.0);\n"
"}\n"
;

#define kAtmosphereGroundShader @"Atmosphere Ground Shader"

@interface SunUpdater : MaplyActiveObject
@end

@implementation SunUpdater
{
    bool changed;
    bool started;
    MaplyCoordinate3d sunPos;
    MaplyShader *shader,*groundShader;
    MaplyAtmosphere * __weak atm;
}

- (id)initWithShader:(MaplyShader *)inShader groundShader:(MaplyShader *)inGroundShader atm:(MaplyAtmosphere *)inAtm viewC:(MaplyBaseViewController *)viewC
{
    self = [super initWithViewController:viewC];
    changed = true;
    started = false;
    shader = inShader;
    groundShader = inGroundShader;
    atm = inAtm;
    
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
    
    Vector3d cameraPos = frameInfo.eyePos;
    Vector4d sunDir4d = Vector4d(sunPos.x,sunPos.y,sunPos.z,1.0);
    sunDir4d /= sunDir4d.w();
    Vector3d sunDir3d(sunDir4d.x(),sunDir4d.y(),sunDir4d.z());
    sunDir3d.normalize();
    double cameraHeight = cameraPos.norm();
    float scale = 1.0f / (atm.outerRadius - 1.f);
    float scaleDepth = 0.25;
    float wavelength[3];
    [atm getWavelength:wavelength];
    for (unsigned int ii=0;ii<3;ii++)
        wavelength[ii] = (float)(1.0/pow(wavelength[ii],4.0));
    
    MaplyShader *shaders[2] = {shader,groundShader};
    for (unsigned int ii=0;ii<2;ii++)
    {
        MaplyShader *thisShader = shaders[ii];
        glUseProgram(thisShader.program->getProgram());
        thisShader.program->setUniform(k_v3CameraPos, Vector3f(cameraPos.x(),cameraPos.y(),cameraPos.z()));
        thisShader.program->setUniform(k_fCameraHeight, (float)cameraHeight);
        thisShader.program->setUniform(k_fCameraHeight2, (float)(cameraHeight*cameraHeight));
        thisShader.program->setUniform(k_v3LightPos, Vector3f(sunDir3d.x(),sunDir3d.y(),sunDir3d.z()));

        thisShader.program->setUniform(k_fInnerRadius, 1.f);
        thisShader.program->setUniform(k_fInnerRadius2, 1.f);
        thisShader.program->setUniform(k_fOuterRadius, atm.outerRadius);
        thisShader.program->setUniform(k_fOuterRadius2, atm.outerRadius*atm.outerRadius);
        thisShader.program->setUniform(k_fScale, scale);
        thisShader.program->setUniform(k_fScaleDepth, scaleDepth);
        thisShader.program->setUniform(k_fScaleOverScaleDepth, scale / scaleDepth);
        
        thisShader.program->setUniform(k_Kr, atm.Kr);
        thisShader.program->setUniform(k_Kr4PI, (float)(atm.Kr * 4.0 * M_PI));
        thisShader.program->setUniform(k_Km, atm.Km);
        thisShader.program->setUniform(k_Km4PI, (float)(atm.Km * 4.0 * M_PI));
        thisShader.program->setUniform(k_ESun, atm.ESun);
        thisShader.program->setUniform(k_KmESun, atm.Km * atm.ESun);
        thisShader.program->setUniform(k_KrESun, atm.Kr * atm.ESun);
        thisShader.program->setUniform(k_v3InvWavelength, Vector3f(wavelength[0],wavelength[1],wavelength[2]));
        thisShader.program->setUniform(k_fSamples, (float)atm.numSamples);
        thisShader.program->setUniform(k_nSamples, atm.numSamples);
        
        thisShader.program->setUniform(k_g, atm.g);
        thisShader.program->setUniform(k_g2, atm.g * atm.g);
        thisShader.program->setUniform(k_fExposure, atm.exposure);
    }
    
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
    float wavelength[3];
}

- (id)initWithViewC:(WhirlyGlobeViewController *)inViewC
{
    self = [super init];
    
    viewC = inViewC;
    
    _Kr = 0.0025;
    _Km = 0.0010;
    _ESun = 20.0;
    _numSamples = 3;
    _outerRadius = 1.05;
    _g = -0.95;
    _exposure = 2.0;
    wavelength[0] = 0.650;
    wavelength[1] = 0.570;
    wavelength[2] = 0.475;

    // Atmosphere shader
    shader = [self setupShader];
    
    if (!shader)
        return nil;
    
    _groundShader = [self setupGroundShader];

    [self complexAtmosphere];
    
    return self;
}

- (void)setWavelength:(float *)inVals
{
    wavelength[0] = inVals[0];
    wavelength[1] = inVals[1];
    wavelength[2] = inVals[2];
}

- (void)getWavelength:(float *)retVals
{
    retVals[0] = wavelength[0];
    retVals[1] = wavelength[1];
    retVals[2] = wavelength[2];
}

- (void)complexAtmosphere
{
    // Make a sphere for the outer atmosphere
    MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
    sphere.center = MaplyCoordinateMake(0, 0);
    sphere.height = -1.0;
    sphere.radius = _outerRadius;
    compObj = [viewC addShapes:@[sphere] desc:@{kMaplyZBufferRead: @(NO),
                                                kMaplyZBufferWrite: @(NO),
                                                kMaplyShapeSampleX: @(120),
                                                kMaplyShapeSampleY: @(60),
                                                kMaplyShapeInsideOut: @(YES),
                                                kMaplyShapeCenterX: @(0.0),
                                                kMaplyShapeCenterY: @(0.0),
                                                kMaplyShapeCenterZ: @(0.0),
                                                kMaplyDrawPriority: @(kMaplyAtmosphereDrawPriorityDefault),
                                                kMaplyShader: kAtmosphereShader}];
    
    sunUpdater = [[SunUpdater alloc] initWithShader:shader groundShader:_groundShader atm:self viewC:viewC];
    [viewC addActiveObject:sunUpdater];
}

- (MaplyShader *)setupGroundShader
{
    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereGroundShader vertex:[NSString stringWithFormat:@"%s",vertexShaderGroundTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderGroundTri] viewC:viewC];
    if (!theShader.valid)
        return nil;
    if (theShader)
        [viewC addShaderProgram:theShader sceneName:kAtmosphereGroundShader];
    
    return theShader;
}

- (void)setSunPosition:(MaplyCoordinate3d)sunPos
{
    if (sunUpdater)
        [sunUpdater setSunPosition:sunPos];
}

- (MaplyShader *)setupShader
{
//    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTri] viewC:viewC];
    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderAtmosTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderAtmosTri] viewC:viewC];
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
