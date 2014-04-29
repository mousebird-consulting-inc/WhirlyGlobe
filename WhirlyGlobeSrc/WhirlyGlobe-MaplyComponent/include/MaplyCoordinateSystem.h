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

/** @brief Coordinate system for tiling systems and data sources and such.
    @details This object represents the spatial reference system and bounding box for objects like the MaplyTileSource or vectors or other things.  Not all data is in lat/lon geographic (actually MaplyPlateCaree) nor is it always in MaplySphericalMercator.  Sometimes it's in one or the other, or a bit of both.
    @details We use this base class to express the coordinate system and we threw in the bounding box, which we will surely come to regret.  Oh well, it's in there.
    @details This object gets passed around to tell us what coordinate system data is in or how we're displaying things.  Right now only MaplySphericalMercator and MaplyPlateCarree are represented.  In the future, there will be more.
   */
@interface MaplyCoordinateSystem : NSObject

/** @brief Set the bounding box in the local coordinate system.
    @details This is the bounding box, for things like display coordinates.  If the extents would normally be in degrees, use radians.  Otherwise, the values are in the local system.
  */
- (void)setBoundsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ll;

/** @brief Return the bounding box in local coordinates.
    @details This is the bounding box in local coordinates, or if the extents would normally be expressed in degrees, it's radians.
  */
- (void)getBoundsLL:(MaplyCoordinate *)ret_ll ur:(MaplyCoordinate *)ret_ur;

/** @brief Convert a coordinate from geographic to the local coordinate system.
    @details Take a geo coordinate (lon/lat in radians) and convert to the local space.
  */
- (MaplyCoordinate)geoToLocal:(MaplyCoordinate)coord;

/** @brief Convert a coordinate from the local space to geographic.
    @details This takes a coordinate in this coordinate system and converts it to geographic (lat/lon in radians).
  */
- (MaplyCoordinate)localToGeo:(MaplyCoordinate)coord;

/** @brief Express the coordinate system in an SRS compatible string.
  */
- (NSString *)getSRS;

/** @brief Can this coordinate system be expressed in degrees?
    @details This is set for coordinate systems that express their extents in degrees.  This is useful for cases where we need to construct some metadata to go along with the system and you'd normally expect it to be in degrees rather than radians.
  */
- (bool)canBeDegrees;

@end

/** @brief Plate Carree is lat/lon stretched out to its full extents.
    @details This coordinate system is used when we're tiling things over the whole earth, from -180 to +180 and from -90 to +90.  Use this if you chopped up your data in that way.
  */
@interface MaplyPlateCarree : MaplyCoordinateSystem

/// @brief Initialize with Plate Carree covering the whole globe.
- (id)initFullCoverage;

/// @brief Initialize with the given bounding box (in radians)
- (id)initWithBoundingBox:(MaplyBoundingBox)bbox;

@end

/** @brief Spherical Mercator is what you'll most commonly see on web maps.
    @details The Spherical Mercator system, with web extents is what MapBox, Google, Bing, etc. use for their maps.  If you ever want to annoy a cartographer, suggest that spherical mercator is all you ever really need.
    @details The drawback with Spherical Mercator is that it doesn't cover the poles and it distorts (and how) its north and south extents.  Web Standard refers to the extents you'll find in most online maps.  This is probably want you want.
  */
@interface MaplySphericalMercator : MaplyCoordinateSystem

/// @brief Initialize with the -85...,+85... extents to match most commonly used online maps
- (id)initWebStandard;

@end
