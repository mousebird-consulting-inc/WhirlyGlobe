/*  MaplyAtmosphere.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/30/15.
 *  Copyright 2011-2023 mousebird consulting
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
 */

#import <WhirlyGlobe_iOS.h>
#import "rendering/MaplyAtmosphere.h"
#import "visual_objects/MaplyShape.h"
#import "MaplyShader_private.h"
#import "MaplyActiveObject_private.h"
#import "MaplyRenderController_private.h"
#import "AtmosphereShadersMTL.h"

using namespace WhirlyKit;
using namespace Eigen;

@interface SunUpdater : MaplyActiveObject
@property (nonatomic) bool lockToCamera;
@end

@implementation SunUpdater
{
    bool changed;
    bool started;
    MaplyCoordinate3d sunPos;
    MaplyShader *shader,*groundShader;
    MaplyAtmosphere * __weak atmosphere;
    Vector3d lastCameraPos;
}

- (instancetype _Nullable)initWithShader:(MaplyShader *)inShader
                            groundShader:(MaplyShader *)inGroundShader
                                     atm:(MaplyAtmosphere *)inAtm
                                   viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!(self = [super initWithViewController:viewC]))
    {
        return nil;
    }

    changed = true;
    started = false;
    shader = inShader;
    groundShader = inGroundShader;
    atmosphere = inAtm;
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
    if (_lockToCamera != lockToCamera) {
        _lockToCamera = lockToCamera;
        changed = true;
    }
}

// Thanks to: http://stainlessbeer.weebly.com/planets-9-atmospheric-scattering.html
//  for the parameter values.

- (void)updateForFrame:(void *)frameInfoVoid
{
    RendererFrameInfo *frameInfo = (RendererFrameInfo *)frameInfoVoid;

    __strong MaplyAtmosphere *atm = atmosphere;
    if (!atm)
    {
        return;
    }
    
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
    const auto cameraHeight = (float)cameraPos.norm();
    float wavelength[3];
    [atm getWavelength:wavelength];
    for (unsigned int ii=0;ii<3;ii++)
    {
        wavelength[ii] = (float)(1.0/pow(wavelength[ii],4.0));
    }

    WhirlyKitAtmosphereShader::AtmosShaderVertUniforms vu;
    memset(&vu, 0, sizeof(vu));
    vu.cameraHeight = cameraHeight;
    vu.innerRadius = 1.0f;
    vu.outerRadius = atm.outerRadius;
    vu.c = cameraHeight * cameraHeight - vu.outerRadius * vu.outerRadius;
    vu.scale = 1 / (vu.outerRadius - vu.innerRadius);
    vu.scaleDepth = 0.25f;
    vu.scaleOverScaleDepth = vu.scale / vu.scaleDepth;
    vu.kr = atm.Kr;
    vu.km = atm.Km;
    vu.eSun = atm.ESun;
    vu.kmESun = vu.km * vu.eSun;
    vu.krESun = vu.kr * vu.eSun;
    vu.kr4PI = (float)(vu.kr * 4.0 * M_PI);
    vu.km4PI = (float)(vu.km * 4.0 * M_PI);
    vu.samples = atm.numSamples;
    CopyIntoMtlFloat3(vu.lightPos, sunDir3d);
    CopyIntoMtlFloat3(vu.invWavelength, Point3f(wavelength[0], wavelength[1], wavelength[2]));

    WhirlyKitAtmosphereShader::AtmosShaderFragUniforms fu;
    memset(&fu, 0, sizeof(fu));
    fu.g = atm.g;
    fu.g2 = fu.g * fu.g;
    fu.exposure = atm.exposure;
    CopyIntoMtlFloat3(fu.lightPos, sunDir3d);

    NSData *vBlock = [[NSData alloc] initWithBytes:&vu length:sizeof(vu)];
    NSData *fBlock = [[NSData alloc] initWithBytes:&fu length:sizeof(fu)];

    for (MaplyShader *shader : {shader, groundShader})
    {
        [shader setUniformBlock:vBlock buffer:WhirlyKitAtmosphereShader::AtmosUniformVertEntry];
        [shader setUniformBlock:fBlock buffer:WhirlyKitAtmosphereShader::AtmosUniformFragEntry];
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

- (instancetype _Nullable)initWithViewC:(WhirlyGlobeViewController *)inViewC
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
    viewC = inViewC;
    
    _Kr = 0.0025;
    _Km = 0.0010;
    _ESun = 20.0;
    _numSamples = 5;
    _outerRadius = 1.05;
    _g = -0.95;
    _exposure = 2.0;
    wavelength[0] = 0.650;
    wavelength[1] = 0.570;
    wavelength[2] = 0.475;

    id<MTLLibrary> lib = [inViewC getMetalLibrary];
    MaplyRenderController *control = [inViewC getRenderControl];
    
    // Atmosphere shader
    shader = [inViewC getShaderByName:kMaplyAtmosphereProgram];
    if (!shader)
    {
        auto air = std::make_shared<ProgramMTL>(
            [kMaplyAtmosphereProgram cStringUsingEncoding:NSASCIIStringEncoding],
            [lib newFunctionWithName:@"vertexTri_atmos"],
            [lib newFunctionWithName:@"fragmentTri_atmos"]);
        if (air->valid)
        {
            [control addShader:kMaplyAtmosphereProgram program:air];
            shader = [inViewC getShaderByName:kMaplyAtmosphereProgram];
        }
        if (!shader)
        {
            return nil;
        }
    }
    
    _groundShader = [inViewC getShaderByName:kMaplyAtmosphereGroundProgram];
    if (!_groundShader)
    {
        auto ground = std::make_shared<ProgramMTL>(
            [kMaplyAtmosphereProgram cStringUsingEncoding:NSASCIIStringEncoding],
            [lib newFunctionWithName:@"vertexTri_atmosGround"],
            [lib newFunctionWithName:@"fragmentTri_atmosGround"]);
        if (ground->valid)
        {
            [control addShader:kMaplyAtmosphereGroundProgram program:ground];
            _groundShader = [inViewC getShaderByName:kMaplyAtmosphereGroundProgram];
        }
        if (!_groundShader)
        {
            return nil;
        }
    }

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
    sunUpdater.lockToCamera = _lockToCamera;
}

- (void)complexAtmosphere
{
    const auto __strong vc = viewC;
    if (!vc)
    {
        return;
    }

    // Make a sphere for the outer atmosphere
    MaplyShapeSphere *sphere = [[MaplyShapeSphere alloc] init];
    sphere.center = MaplyCoordinateMake(0, 0);
    sphere.height = -1.0;
    sphere.radius = _outerRadius;

    NSDictionary *desc = @{
        kMaplyZBufferRead: @(NO),
        kMaplyZBufferWrite: @(NO),
        kMaplyShapeSampleX: @(120),
        kMaplyShapeSampleY: @(60),
        kMaplyShapeInsideOut: @(YES),
        kMaplyShapeCenterX: @(0.0),
        kMaplyShapeCenterY: @(0.0),
        kMaplyShapeCenterZ: @(0.0),
        kMaplyDrawPriority: @(kMaplyAtmosphereDrawPriorityDefault),
        kMaplyShader: kMaplyAtmosphereProgram,
        kMaplyFade: @(5.0),
    };

    compObj = [vc addShapes:@[sphere] desc:desc];
    if (compObj)
    {
        sunUpdater = [[SunUpdater alloc] initWithShader:shader groundShader:_groundShader atm:self viewC:vc];
        if (sunUpdater)
        {
            [vc addActiveObject:sunUpdater];
        }
    }
}

- (void)setSunPosition:(MaplyCoordinate3d)sunPos
{
    [sunUpdater setSunPosition:sunPos];
}

- (void)removeFromViewC
{
    if (const auto __strong vc = viewC)
    {
        if (compObj)
        {
            [vc removeObject:compObj];
            compObj = nil;
        }
        if (sunUpdater)
        {
            [vc removeActiveObject:sunUpdater];
            sunUpdater = nil;
        }
    }
}

@end
