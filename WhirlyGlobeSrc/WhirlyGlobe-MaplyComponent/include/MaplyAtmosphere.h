/*
 *  MaplyAtmosphere.h
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

#import <UIKit/UIKit.h>
#import "MaplyComponentObject.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyLight.h"

/** @brief Sets up the objects and shaders to implement an atmosphere.
    @details This object sets up a shader implementation of the simple atmosphere from GPU Gems 2  http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
  */
@interface MaplyAtmosphere : NSObject

/// @brief Initialize the view controller.  Will place objects in that view controller.
- (id)initWithViewC:(WhirlyGlobeViewController *)viewC;

/// @brief Rayleigh scattering constant (0.0025 by default)
@property (nonatomic) float Kr;

/// @brief Mie scattering constant (0.0010 by default)
@property (nonatomic) float Km;

/// @brief Brightness of the sun (20.0 by default)
@property (nonatomic) float ESun;

/// @brief Number of samples for the ray through the atmosphere (3 by default)
@property (nonatomic) int numSamples;

/// @brief Outer radius of the atmosphere (1.05 by default).  Earth is radius 1.0.
@property (nonatomic) float outerRadius;

/// @brief Constant used in the fragment shader.  Default is -0.95.
@property (nonatomic) float g;

/// @brief Exposure constant in fragment shader.  Default is 2.0.
@property (nonatomic) float exposure;

/// @brief The ground shader we set up.  You need to apply it yourself.
@property (nonatomic) MaplyShader *groundShader;

/// @brief Wavelengths of the light (RGB).  Three floats, defaults are: 0.650, 0.570, 0.475
- (void)setWavelength:(float *)wavelength;

/// @brief Return the current wavelength settings (RGB)
- (void)getWavelength:(float *)wavelength;

/// @brief Set the sun's position relative to the earth.  This is what comes out of MaplySun.
- (void)setSunPosition:(MaplyCoordinate3d)sunDir;

/// @brief Remove objects from the view controller we set it up in.
- (void)removeFromViewC;

@end
