/*
 *  MaplyQuadImageOfflineLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

@class MaplyQuadImageOfflineLayer;

@protocol MaplyQuadImageOfflineDelegate<NSObject>

/// Called to update the image when there's a new one
- (void)offlineLayer:(MaplyQuadImageOfflineLayer *)layer images:(NSArray *)image bbox:(MaplyBoundingBox)bbox;

@end

@interface MaplyQuadImageOfflineLayer : MaplyViewControllerLayer

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)coordSys tileSource:(NSObject<MaplyTileSource> *)tileSource;

@property (nonatomic,assign) bool on;

@property (nonatomic,assign) int numSimultaneousFetches;

@property (nonatomic,assign) unsigned int imageDepth;

@property (nonatomic) bool flipY;

@property (nonatomic) int maxTiles;

@property (nonatomic) CGSize textureSize;

@property (nonatomic,assign) bool asyncFetching;

/// How often we'll generate a new image
@property (nonatomic) float period;

/// Extents of the image (in local coordinates)
@property (nonatomic) MaplyBoundingBox bbox;

/// Delegate for image updates
@property (nonatomic,weak) NSObject<MaplyQuadImageOfflineDelegate> *delegate;

- (void)reload;

@end
