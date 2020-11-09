/*
 *  MaplyAtmosphere.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/30/15.
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

#import <WhirlyGlobe_iOS.h>
#import "rendering/MaplyAtmosphere.h"
#import "visual_objects/MaplyShape.h"
#import "MaplyShader_private.h"
#import "MaplyActiveObject_private.h"

using namespace WhirlyKit;
using namespace Eigen;

#if 0
static const char *vertexShaderAtmosTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform vec3 u_v3CameraPos;
uniform float u_fCameraHeight2;
uniform vec3 u_v3LightPos;

uniform float u_fInnerRadius;
uniform float u_fInnerRadius2;
uniform float u_fOuterRadius;
uniform float u_fOuterRadius2;
uniform float u_fScale;
uniform float u_fScaleDepth;
uniform float u_fScaleOverScaleDepth;

uniform float u_Kr;
uniform float u_Kr4PI;
uniform float u_Km;
uniform float u_Km4PI;
uniform float u_ESun;
uniform float u_KmESun;
uniform float u_KrESun;
uniform vec3 u_v3InvWavelength;
uniform float u_fSamples;
uniform int u_nSamples;

attribute vec3 a_position;

varying highp vec3 v3Direction;
varying highp vec3 v3RayleighColor;
varying highp vec3 v3MieColor;

float scale(float fCos)
{
  float x = 1.0 - fCos;
  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main()
{
   vec3 v3Pos = a_position.xyz;
   vec3 v3Ray = v3Pos - u_v3CameraPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

  float B = 2.0 * dot(u_v3CameraPos, v3Ray);
  float C = u_fCameraHeight2 - u_fOuterRadius2;
  float fDet = max(0.0, B*B - 4.0 * C);
  float fNear = 0.5 * (-B - sqrt(fDet));

   vec3 v3Start = u_v3CameraPos + v3Ray * fNear;
   fFar -= fNear;

   float fStartAngle = dot(v3Ray, v3Start) / u_fOuterRadius;
   float fStartDepth = exp(-1.0/u_fScaleDepth);
   float fStartOffset = fStartDepth * scale(fStartAngle);

   float fSampleLength = fFar / u_fSamples;
   float fScaledLength = fSampleLength * u_fScale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
   vec3 v3Attenuate;
   for (int i=0; i<u_nSamples; i++)
   {
     float fHeight = length(v3SamplePoint);
     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));
     float fLightAngle = dot(u_v3LightPos, v3SamplePoint) / fHeight;
     float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
     float fScatter = (fStartOffset + fDepth *(scale(fLightAngle) - scale(fCameraAngle)));
     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));
     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
     v3SamplePoint += v3SampleRay;
   }

   v3MieColor = v3FrontColor * u_KmESun;
   v3RayleighColor = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_Km4PI);
   v3Direction = u_v3CameraPos - v3Pos;

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
}
)";

static const char *fragmentShaderAtmosTri = R"(
precision highp float;

uniform float g;
uniform float g2;
uniform float fExposure;
uniform vec3 u_v3LightPos;

varying highp vec3 v3Direction;
varying highp vec3 v3RayleighColor;
varying highp vec3 v3MieColor;

void main()
{
  float fCos = dot(u_v3LightPos, normalize(v3Direction)) / length(v3Direction);
  float fCos2 = fCos*fCos;
  float rayPhase = 0.75 + 0.75*fCos2;
  float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
  vec3 color = rayPhase * v3RayleighColor + miePhase * v3MieColor;
  color = 1.0 - exp(color * -fExposure);
  gl_FragColor = vec4(color,color.b);
}
)";
#endif

#define kAtmosphereShader @"Atmosphere Shader"

#if 0
static const char *vertexShaderGroundTri = R"(
precision highp float;

uniform mat4  u_mvpMatrix;
uniform vec3 u_v3CameraPos;
uniform float u_fCameraHeight2;
uniform vec3 u_v3LightPos;

uniform float u_fInnerRadius;
uniform float u_fInnerRadius2;
uniform float u_fOuterRadius;
uniform float u_fOuterRadius2;
uniform float u_fScale;
uniform float u_fScaleDepth;
uniform float u_fScaleOverScaleDepth;

uniform float u_Kr;
uniform float u_Kr4PI;
uniform float u_Km;
uniform float u_Km4PI;
uniform float u_ESun;
uniform float u_KmESun;
uniform float u_KrESun;
uniform vec3 u_v3InvWavelength;
uniform float u_fSamples;
uniform int u_nSamples;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord0;
attribute vec2 a_texCoord1;

varying mediump vec3 v_color;
varying mediump vec3 v_v3attenuate;
varying mediump vec2 v_texCoord0;
varying mediump vec2 v_texCoord1;

float scale(float fCos)
{
  float x = 1.0 - fCos;
  return u_fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main()
{
   vec3 v3Pos = a_normal.xyz;
   vec3 v3Ray = v3Pos - u_v3CameraPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

  float B = 2.0 * dot(u_v3CameraPos, v3Ray);
  float C = u_fCameraHeight2 - u_fOuterRadius2;
  float fDet = max(0.0, B*B - 4.0 * C);
  float fNear = 0.5 * (-B - sqrt(fDet));

   vec3 v3Start = u_v3CameraPos + v3Ray * fNear;
   fFar -= fNear;

   float fDepth = exp((u_fInnerRadius - u_fOuterRadius) / u_fScaleDepth);
   float fCameraAngle = dot(-v3Ray, v3Pos) / length (v3Pos);
   float fLightAngle = dot(u_v3LightPos, v3Pos) / length(v3Pos);
   float fCameraScale = scale(fCameraAngle);
   float fLightScale = scale(fLightAngle);
   float fCameraOffset = fDepth*fCameraScale;
   float fTemp = (fLightScale + fCameraScale);

   float fSampleLength = fFar / u_fSamples;
   float fScaledLength = fSampleLength * u_fScale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
   vec3 v3Attenuate;
   for (int i=0; i<u_nSamples; i++)
   {
     float fHeight = length(v3SamplePoint);
     float fDepth = exp(u_fScaleOverScaleDepth * (u_fInnerRadius - fHeight));
     float fScatter = fDepth*fTemp - fCameraOffset;
     v3Attenuate = exp(-fScatter * (u_v3InvWavelength * u_Kr4PI + u_Km4PI));
     v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
     v3SamplePoint += v3SampleRay;
   }

   v_v3attenuate = v3Attenuate;
   v_color = v3FrontColor * (u_v3InvWavelength * u_KrESun + u_KmESun);
   v_texCoord0 = a_texCoord0;
   v_texCoord1 = a_texCoord1;

   gl_Position = u_mvpMatrix * vec4(a_position,1.0);
}
)";

// Note: Not finished with these

static const char *fragmentShaderGroundTri = R"(
precision highp float;

uniform sampler2D s_baseMap0;
uniform sampler2D s_baseMap1;

varying vec3      v_color;
varying vec2      v_texCoord0;
varying vec2      v_texCoord1;
varying vec3      v_v3attenuate;

void main()
{
  vec3 dayColor = texture2D(s_baseMap0, v_texCoord0).xyz * v_v3attenuate;
  vec3 nightColor = texture2D(s_baseMap1, v_texCoord1).xyz * (1.0 - v_v3attenuate);
  gl_FragColor = vec4(v_color, 1.0) + vec4(dayColor + nightColor, 1.0);
}
)";
#endif

#define kAtmosphereGroundShader @"Atmosphere Ground Shader"

@interface SunUpdater : MaplyActiveObject
@property (nonatomic) bool lockToCamera;
@end

@implementation SunUpdater
{
    bool changed;
    bool started;
    MaplyCoordinate3d sunPos;
    MaplyShader *shader,*groundShader;
    MaplyAtmosphere * __weak atm;
    Vector3d lastCameraPos;
}

- (instancetype)initWithShader:(MaplyShader *)inShader groundShader:(MaplyShader *)inGroundShader atm:(MaplyAtmosphere *)inAtm viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    self = [super initWithViewController:viewC];
    changed = true;
    started = false;
    shader = inShader;
    groundShader = inGroundShader;
    atm = inAtm;
    _lockToCamera = false;
    
    return self;
}

- (bool)hasUpdate
{
    return changed || !started;
}

- (void)setSunPosition:(MaplyCoordinate3d)inSunPos
{
    sunPos = inSunPos;
    changed = true;
}

- (void)setLockToCamera:(bool)lockToCamera
{
    _lockToCamera = lockToCamera;
    changed = true;
}

static bool nameIDsSetup = false;
static StringIdentity v3CameraPosNameID;
static StringIdentity v3LightPosNameID;
static StringIdentity v3InvWavelengthNameID;
static StringIdentity fCameraHeightNameID;
static StringIdentity fCameraHeight2NameID;
static StringIdentity fInnerRadiusNameID;
static StringIdentity fInnerRadius2NameID;
static StringIdentity fOuterRadiusNameID;
static StringIdentity fOuterRadius2NameID;
static StringIdentity fScaleNameID;
static StringIdentity fScaleDepthNameID;
static StringIdentity fScaleOverScaleDepthNameID;
static StringIdentity KrNameID;
static StringIdentity Kr4PINameID;
static StringIdentity KmNameID;
static StringIdentity Km4PINameID;
static StringIdentity ESunNameID;
static StringIdentity KmESunNameID;
static StringIdentity KrESunNameID;
static StringIdentity fSamplesNameID;
static StringIdentity nSamplesNameID;
static StringIdentity gNameID;
static StringIdentity g2NameID;
static StringIdentity fExposureNameID;

- (void)setupStringIndices
{
    if (nameIDsSetup)
        return;
    
    v3CameraPosNameID = StringIndexer::getStringID("u_v3CameraPos");
    v3LightPosNameID = StringIndexer::getStringID("u_v3LightPos");
    v3InvWavelengthNameID = StringIndexer::getStringID("u_v3InvWavelength");
    fCameraHeightNameID = StringIndexer::getStringID("u_fCameraHeight");
    fCameraHeight2NameID = StringIndexer::getStringID("u_fCameraHeight2");
    fInnerRadiusNameID = StringIndexer::getStringID("u_fInnerRadius");
    fInnerRadius2NameID = StringIndexer::getStringID("u_fInnerRadius2");
    fOuterRadiusNameID = StringIndexer::getStringID("u_fOuterRadius");
    fOuterRadius2NameID = StringIndexer::getStringID("u_fOuterRadius2");
    fScaleNameID = StringIndexer::getStringID("u_fScale");
    fScaleDepthNameID = StringIndexer::getStringID("u_fScaleDepth");
    fScaleOverScaleDepthNameID = StringIndexer::getStringID("u_fScaleOverScaleDepth");
    KrNameID = StringIndexer::getStringID("u_Kr");
    Kr4PINameID = StringIndexer::getStringID("u_Kr4PI");
    KmNameID = StringIndexer::getStringID("u_Km");
    Km4PINameID = StringIndexer::getStringID("u_Km4PI");
    ESunNameID = StringIndexer::getStringID("u_ESun");
    KmESunNameID = StringIndexer::getStringID("u_KmESun");
    KrESunNameID = StringIndexer::getStringID("u_KrESun");
    fSamplesNameID = StringIndexer::getStringID("u_fSamples");
    nSamplesNameID = StringIndexer::getStringID("u_nSamples");
    gNameID = StringIndexer::getStringID("g");
    g2NameID = StringIndexer::getStringID("g2");
    fExposureNameID = StringIndexer::getStringID("fExposure");

    nameIDsSetup = true;
}

// Thanks to: http://stainlessbeer.weebly.com/planets-9-atmospheric-scattering.html
//  for the parameter values.

- (void)updateForFrame:(void *)frameInfoVoid
{
    RendererFrameInfo *frameInfo = (RendererFrameInfo *)frameInfoVoid;
    [self setupStringIndices];
    
    if (!changed && started)
    {
        // Check the camera position
        Vector3d cameraPos = frameInfo->eyePos;
        if (cameraPos == lastCameraPos)
            return;
    }
    
    frameInfo->sceneRenderer->forceDrawNextFrame();
    
    Vector3d cameraPos = frameInfo->eyePos;
    Vector4d sunDir4d = Vector4d(sunPos.x,sunPos.y,sunPos.z,1.0);
    sunDir4d /= sunDir4d.w();
    Vector3d sunDir3d(sunDir4d.x(),sunDir4d.y(),sunDir4d.z());
    if (_lockToCamera)
        sunDir3d = cameraPos;
    sunDir3d.normalize();
    //double cameraHeight = cameraPos.norm();
    //float scale = 1.0f / (atm.outerRadius - 1.f);
    //float scaleDepth = 0.25;
    float wavelength[3];
    [atm getWavelength:wavelength];
    for (unsigned int ii=0;ii<3;ii++)
        wavelength[ii] = (float)(1.0/pow(wavelength[ii],4.0));
    
    //MaplyShader *shaders[2] = {shader,groundShader};
    for (unsigned int ii=0;ii<2;ii++)
    {
        //MaplyShader *thisShader = shaders[ii];
        // TODO: Update for Metal
        NSLog(@"MaplyAtmosphere not implemented for Metal.");
    }
    
    changed = false;
    started = true;
    lastCameraPos = cameraPos;    
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

- (instancetype)initWithViewC:(WhirlyGlobeViewController *)inViewC
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

- (void)setWavelengthRed:(float) redWavelength green:(float)greenWavelength blue:(float)blueWavelength
{
    wavelength[0] = redWavelength;
    wavelength[1] = greenWavelength;
    wavelength[2] = blueWavelength;
}

- (void)getWavelength:(float *)retVals
{
    retVals[0] = wavelength[0];
    retVals[1] = wavelength[1];
    retVals[2] = wavelength[2];
}

- (float)getWavelengthForComponent:(short)component
{
    return wavelength[component];
}

- (void)setLockToCamera:(bool)lockToCamera
{
    _lockToCamera = lockToCamera;
    if (sunUpdater)
        sunUpdater.lockToCamera = _lockToCamera;
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
    // TODO: Switch to Metal
//    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereGroundShader vertex:[NSString stringWithFormat:@"%s",vertexShaderGroundTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderGroundTri] viewC:viewC];
    MaplyShader *theShader = nil;
    if (!theShader.valid)
        return nil;
    if (theShader)
        [viewC addShaderProgram:theShader];
    
    return theShader;
}

- (void)setSunPosition:(MaplyCoordinate3d)sunPos
{
    if (sunUpdater)
        [sunUpdater setSunPosition:sunPos];
}

- (MaplyShader *)setupShader
{
// TODO: Switch to Metal
//    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderTri] viewC:viewC];
//    MaplyShader *theShader = [[MaplyShader alloc] initWithName:kAtmosphereShader vertex:[NSString stringWithFormat:@"%s",vertexShaderAtmosTri] fragment:[NSString stringWithFormat:@"%s",fragmentShaderAtmosTri] viewC:viewC];
    MaplyShader *theShader = nil;
    if (!theShader.valid)
        return nil;
    if (theShader)
        [viewC addShaderProgram:theShader];
    
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
