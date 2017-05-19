/*
 *  MaplyAtmosphere.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/30/15.
 *  Copyright 2011-2017 mousebird consulting
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

#import <UIKit/UIKit.h>
#import "MaplyComponentObject.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyLight.h"

/** 
    Sets up the objects and shaders to implement an atmosphere.
    
    This object sets up a shader implementation of the simple atmosphere from GPU Gems 2  http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
  */
@interface MaplyAtmosphere : NSObject

/// Initialize the view controller.  Will place objects in that view controller.
- (nullable instancetype)initWithViewC:(WhirlyGlobeViewController *__nonnull)viewC;

/// Rayleigh scattering constant (0.0025 by default)
@property (nonatomic) float Kr;

/// Mie scattering constant (0.0010 by default)
@property (nonatomic) float Km;

/// Brightness of the sun (20.0 by default)
@property (nonatomic) float ESun;

/// Number of samples for the ray through the atmosphere (3 by default)
@property (nonatomic) int numSamples;

/// Outer radius of the atmosphere (1.05 by default).  Earth is radius 1.0.
@property (nonatomic) float outerRadius;

/// Constant used in the fragment shader.  Default is -0.95.
@property (nonatomic) float g;

/// Exposure constant in fragment shader.  Default is 2.0.
@property (nonatomic) float exposure;

/// The ground shader we set up.  You need to apply it yourself.
@property (nonatomic,nullable) MaplyShader *groundShader;

/// If set we'll lock the sun direction to the camera position.  Permanent daylight.
@property (nonatomic) bool lockToCamera;

/// Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
- (void)setWavelength:(float *__nonnull)wavelength;

/// Wavelengths of the light (RGB).  Defaults are: 0.650, 0.570, 0.475
- (void)setWavelengthRed:(float) redWavelength green:(float)greenWavelength blue:(float)blueWavelength;

/// Return the current wavelength settings (RGB)
- (void)getWavelength:(float *__nonnull)wavelength;

/// Return the current wavelength settings (RGB). The component is 0 for red, 1 for green and  2 for blue
- (float)getWavelengthForComponent:(short)component;

/// Set the sun's position relative to the earth.  This is what comes out of MaplySun.
- (void)setSunPosition:(MaplyCoordinate3d)sunDir;

/// Remove objects from the view controller we set it up in.
- (void)removeFromViewC;

@end
