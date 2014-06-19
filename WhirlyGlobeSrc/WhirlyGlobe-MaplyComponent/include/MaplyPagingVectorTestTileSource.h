/*
 *  MaplyPagingVectorTestTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/19/14.
 *  Copyright 2011-2014 mousebird consulting
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

#import "MaplyQuadPagingLayer.h"
#import "MaplyCoordinateSystem.h"

/** @brief A data source used for testing paging layer.
    @details This data source puts up a colored and numbered rectangle for each tile loaded.  It's useful for debugging paging schemes.
  */
@interface MaplyPagingVectorTestTileSource : NSObject<MaplyPagingDelegate>

/// Initialize with the coordinate system, min and max zoom levels, and the number of images to return
- (id)initWithCoordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom;

/// Coordinate system we're pretending to be in
@property (nonatomic,readonly) MaplyCoordinateSystem *coordSys;

/// Minimum zoom level supported
@property (nonatomic) int minZoom;

/// Max zoom level supported
@property (nonatomic) int maxZoom;


@end
