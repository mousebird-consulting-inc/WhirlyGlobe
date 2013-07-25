/*
 *  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import <Foundation/Foundation.h>
#import "MaplyCoordinate.h"

/** The coordinate system is an opaque object representing a particular
    spatial coordinate system (e.g. Plate Caree or Spherical Mercator)
    and bounding box.
    At present most users just need a couple of different coordinate systems,
    even though WhirlyGlobe-Maply has much more flexible support.  Obviously,
    this can be expanded in the future.
  */
@interface MaplyCoordinateSystem : NSObject

/// Return the bounding box (in local coordinates)
- (void)getBoundsLL:(MaplyCoordinate *)ret_ll ur:(MaplyCoordinate *)ret_ur;

/// Express this coordinate system using SRS syntax
- (NSString *)getSRS;

@end

/** Plate Carree is just lat/lon stretched out to its full extents.
    Use this if your data source return tiles that square in lat/lon.
  */
@interface MaplyPlateCarree : MaplyCoordinateSystem

/// Plate Caree that covers the globe
- (id)initFullCoverage;

@end

/** Spherical Mercator is what a the most common
    remote tile sources use (e.g. MapBox, Google, Bing)
  */
@interface MaplySphericalMercator : MaplyCoordinateSystem

/// Create the standard extents used by MapBox, Google, Bing, etc.
- (id)initWebStandard;

@end
