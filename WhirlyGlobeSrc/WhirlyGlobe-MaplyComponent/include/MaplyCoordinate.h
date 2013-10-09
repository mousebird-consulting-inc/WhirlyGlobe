/*
 *  MaplyCoordinate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

/** @typedef struct MaplyCoordinate
    @brief A 2D coordinate representation.
    @details The Maply Coordinate is a simple 2 dimensional coordinate
    passed around to numerous methods.  For geo-coordinates x
    maps to longitude and y to latitude and the values are
    in radians.
  */
typedef struct
{
    float x,y;
} MaplyCoordinate;

/** @typedef struct MaplyCoordinate3d
    @brief A 3D coordinate representation.
    @details The 3D version of the Maply Coordinate adds a z values, often
    in meters, but not always.  Consult the appropriate method to
    be sure.
  */
typedef struct
{
    float x,y,z;
} MaplyCoordinate3d;

/** @typedef struct MaplyBoundingBox
    @brief Represents a bounding box in a particular coordinate system.
    @details ll is the lower left and ur is the upper right.
  */
typedef struct
{
    MaplyCoordinate ll;
    MaplyCoordinate ur;
} MaplyBoundingBox;

#if __cplusplus
extern "C" {
#endif
    
/** @brief Construct a MaplyGeoCoordinate with longitude and latitude values in degrees.
    @details MaplyCoordinate's are in radians when they represent lon/lat values.  This function does that conversion for you.
    @param degLon The longitude value (east to west) in degrees.
    @param degLat The latitude value (north to south) in degrees.
    @return A 2D MaplyCoordinate in radians (if representing a lon/lat value).
  */
MaplyCoordinate MaplyCoordinateMakeWithDegrees(float degLon,float degLat);
    
/** @brief Construct a MaplyCoordinat3d from the values given.
    @param x The x value, or longitude in radians if we're making geo coordinates.
    @param y The y value, or latitude in radians if we're making geo coordinates.
    @param z The z value, sometimes this is display coordinates (radius == 1.0 for a sphere)
              and sometimes this is meters.  It depends on how you're using it.
    @return A 3D MaplyCorodinate3d in radians + other (if representing a lon/lat value).
  */
MaplyCoordinate3d MaplyCoordinate3dMake(float x,float y,float z);
    
#if __cplusplus
}
#endif

