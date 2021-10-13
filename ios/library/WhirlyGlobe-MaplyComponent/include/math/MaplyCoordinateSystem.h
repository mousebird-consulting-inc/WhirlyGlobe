/*
 *  MaplyCoordinateSystem.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import <Foundation/Foundation.h>
#import "MaplyCoordinate.h"

/** 
    Coordinate system for tiling systems and data sources and such.
    
    This object represents the spatial reference system and bounding box for objects like the MaplyTileSource or vectors or other things.  Not all data is in lat/lon geographic (actually MaplyPlateCaree) nor is it always in MaplySphericalMercator.  Sometimes it's in one or the other, or a bit of both.
    
    We use this base class to express the coordinate system and we threw in the bounding box, which we will surely come to regret.  Oh well, it's in there.
    
    This object gets passed around to tell us what coordinate system data is in or how we're displaying things.  Right now only MaplySphericalMercator and MaplyPlateCarree are represented.  In the future, there will be more.
   */
@interface MaplyCoordinateSystem : NSObject

/** 
    Set the bounding box in the local coordinate system.
 
    This is the bounding box, for things like display coordinates.  If the extents would normally be in degrees, use radians.  Otherwise, the values are in the local system.
 */
- (void)setBounds:(MaplyBoundingBox)bounds;

/**
    Set the bounding box in the local coordinate system.
 
    This is the bounding box, for things like display coordinates.  If the extents would normally be in degrees, use radians.  Otherwise, the values are in the local system.
 */
- (void)setBoundsD:(MaplyBoundingBoxD)boundsD;

/** 
    Set the bounding box in the local coordinate system.
    
    This is the bounding box, for things like display coordinates.  If the extents would normally be in degrees, use radians.  Otherwise, the values are in the local system.
  */
- (void)setBoundsLL:(MaplyCoordinate * __nonnull)ll ur:(MaplyCoordinate * __nonnull)ll;

/** 
    Return the bounding box in local coordinates.
 
    This is the bounding box in local coordinates, or if the extents would normally be expressed in degrees, it's radians.
 */
- (MaplyBoundingBox)getBounds;

/** 
    Return the bounding box in local coordinates.
    
    This is the bounding box in local coordinates, or if the extents would normally be expressed in degrees, it's radians.
  */
- (void)getBoundsLL:(MaplyCoordinate * __nonnull)ret_ll ur:(MaplyCoordinate * __nonnull)ret_ur;

/** 
    Convert a coordinate from geographic to the local coordinate system.
    
    Take a geo coordinate (lon/lat in radians) and convert to the local space.
  */
- (MaplyCoordinate)geoToLocal:(MaplyCoordinate)coord;

/** 
    Convert a coordinate from the local space to geographic.
    
    This takes a coordinate in this coordinate system and converts it to geographic (lat/lon in radians).
  */
- (MaplyCoordinate)localToGeo:(MaplyCoordinate)coord;

/** 
    Convert a 3D coordinate from the local space to geocentric.
    
    This takes a 3D coordinate (including height) and converts it to geocentric in WGS84.
  */
- (MaplyCoordinate3dD)localToGeocentric:(MaplyCoordinate3dD)coord;

/** 
    Convert a 3D coordinate from geocentric to the local space.
    
    This takes a 3D geocentric coordinate (WGS84) and converts it to the local space, including height;
  */
- (MaplyCoordinate3dD)geocentricToLocal:(MaplyCoordinate3dD)coord;

/** 
    Express the coordinate system in an SRS compatible string.
  */
- (NSString * __nonnull)getSRS;

/** 
    Can this coordinate system be expressed in degrees?
    
    This is set for coordinate systems that express their extents in degrees.  This is useful for cases where we need to construct some metadata to go along with the system and you'd normally expect it to be in degrees rather than radians.
  */
- (bool)canBeDegrees;

@end

/** 
    Plate Carree is lat/lon stretched out to its full extents.
    
    This coordinate system is used when we're tiling things over the whole earth, from -180 to +180 and from -90 to +90.  Use this if you chopped up your data in that way.
  */
@interface MaplyPlateCarree : MaplyCoordinateSystem

/// Initialize with Plate Carree covering the whole globe.
- (nonnull instancetype)init;

/// Initialize with Plate Carree covering the whole globe.
- (nullable instancetype)initFullCoverage;

/// Initialize with the given bounding box (in radians)
- (nullable instancetype)initWithBoundingBox:(MaplyBoundingBox)bbox;

/// Initialize with the given bounding box (in radians)
- (nullable instancetype)initWithBoundingBoxD:(MaplyBoundingBoxD)bbox;

@end

/** 
    Spherical Mercator is what you'll most commonly see on web maps.
    
    The Spherical Mercator system, with web extents is what MapBox, Google, Bing, etc. use for their maps.  If you ever want to annoy a cartographer, suggest that spherical mercator is all you ever really need.
    
    The drawback with Spherical Mercator is that it doesn't cover the poles and it distorts (and how) its north and south extents.  Web Standard refers to the extents you'll find in most online maps.  This is probably want you want.
  */
@interface MaplySphericalMercator : MaplyCoordinateSystem

/// Initialize with the -85...,+85... extents to match most comm only used online maps
- (nonnull instancetype)init;

/// Initialize with the -85...,+85... extents to match most commonly used online maps
- (nonnull instancetype)initWebStandard;

@end

/** 
    A generic coordinate system wrapper around proj4.
    
    You create one of these with a proj4 string.  It'll act like a normal MaplyCoordinateSysterm after that.
    
    Be sure to check that the system is valid.  The proj4 string could be wrong.
  */
@interface MaplyProj4CoordSystem : MaplyCoordinateSystem

/** 
    Initialize with a proj4 compatible string
    
    Since this is just a proj.4 wrapper, we need an initialization string that it can parse.
  */
- (nonnull instancetype)initWithString:(NSString * __nonnull)proj4Str;

/// True if the proj.4 string was valid and the coordinate system can work.
- (bool)valid;

@end

/** 
    Generate the correct coordinate system from a standard EPSG.
    
    This returns the correct coordinate system from a standard EPSG string.
    
    The list of available coordinate systems is very short.
  */
MaplyCoordinateSystem * __nullable MaplyCoordinateSystemFromEPSG(NSString * __nonnull crs);
