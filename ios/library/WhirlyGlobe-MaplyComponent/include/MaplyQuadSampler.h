/*
 *  MaplyQuadSampler.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 3/27/18.
 *  Copyright 2011-2018 Saildrone Inc
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

#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"
#import "MaplyRenderController.h"

/**
    Sampling parameters.
 
    These are used to describe how we want to break down the globe or
    flat projection onto the globe.
  */
@interface MaplySamplingParams : NSObject

/// The coordinate system we'll be sampling from.
@property (nonatomic,nonnull) MaplyCoordinateSystem *coordSys;

/// Min zoom level for sampling
@property (nonatomic) int minZoom;

/// Max zoom level for sampling
@property (nonatomic) int maxZoom;

/// Generate geometry to cover the north and south poles
/// Only works for world-wide projections
@property (nonatomic) bool coverPoles;

/// If set, generate skirt geometry to hide the edges between levels
@property (nonatomic) bool edgeMatching;

/// Tesselation values per level for breaking down the coordinate system (e.g. globe)
@property (nonatomic) int tessX,tessY;

/// Decide if these sampling params are the same as others
- (bool)isEqualTo:(MaplySamplingParams *__nonnull)other;

@end
