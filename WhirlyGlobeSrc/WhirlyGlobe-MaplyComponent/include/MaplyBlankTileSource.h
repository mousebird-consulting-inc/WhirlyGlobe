/*
 *  MaplyBlankTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/15/15.
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

/** @brief A blank tile sources just generates a single colored image for each tile.
  */
@interface MaplyBlankTileSource : NSObject<MaplyTileSource>

/// Initialize with the coordinate system, min and max zoom levels, and the number of images to return
- (id)initWithCoordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Coordinate system we're pretending to be in
@property (nonatomic,readonly) MaplyCoordinateSystem *coordSys;

/// How big the images we generate are
@property (nonatomic) int pixelsPerSide;

/// Color for the generated tiles
@property (nonatomic,strong) UIColor *color;

@end
