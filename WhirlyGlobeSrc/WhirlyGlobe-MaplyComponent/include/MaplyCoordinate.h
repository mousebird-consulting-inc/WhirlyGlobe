/*
 *  MaplyCoordinate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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


static const MaplyCoordinate kMaplyNullCoordinate = {.x = FLT_MIN, .y = FLT_MIN};


/** @typedef struct MaplyCoordinateD
    @brief Double precision version of 2D coordinate.
    @details This works the same was as the MaplyCoordinate, but has
    more precision.
  */
typedef struct
{
    double x,y;
} MaplyCoordinateD;

static const MaplyCoordinateD kMaplyNullCoordinateD = {.x = DBL_MIN, .y = DBL_MIN};


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

/** @brief An NSObject based wrapper for 3D coordinates.
    @details This wrapper encapsulates a MaplyCoordinate3d so we can pass them around in NSDictionary objects.
  */
@interface MaplyCoordinate3dWrapper : NSObject

/// @brief Initialize with a 3D coordinate
- (instancetype)initWithCoord:(MaplyCoordinate3d)coord;

/// @brief 3D coordinate
@property (nonatomic,readonly) MaplyCoordinate3d coord;

@end

/** @typedef struct MaplyBoundingBox
    @brief Represents a bounding box in a particular coordinate system.
    @details ll is the lower left and ur is the upper right.
  */
typedef struct
{
    MaplyCoordinate ll;
    MaplyCoordinate ur;
} MaplyBoundingBox;

static const MaplyBoundingBox kMaplyNullBoundingBox = {
	.ll = {.x = FLT_MIN, .y = FLT_MIN},
	.ur = {.x = FLT_MIN, .y = FLT_MIN}
};

/** @typedef struct MaplyBoundingBox
 @brief Represents a bounding box in a particular coordinate system.
 @details ll is the lower left and ur is the upper right.
 */
typedef struct
{
	MaplyCoordinateD ll;
	MaplyCoordinateD ur;
} MaplyBoundingBoxD;

static const MaplyBoundingBoxD kMaplyNullBoundingBoxD = {
	.ll = {.x = DBL_MIN, .y = DBL_MIN},
	.ur = {.x = DBL_MIN, .y = DBL_MIN}
};

#if __cplusplus
extern "C" {
#endif

/** @brief Construct a MaplyCoordiante with longitude and latitude values in radians.
    @details MaplyCoordinate's are in radians when they represent lon/lat values.  This constructs one with radians as input.
    @return A 2D MaplyCoordinate in radians (if representing a lon/lat value).
  */
MaplyCoordinate MaplyCoordinateMake(float radLon,float radLat);
    
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
    @return A 3D MaplyCoordinate3d in radians + other (if representing a lon/lat value).
  */
MaplyCoordinate3d MaplyCoordinate3dMake(float x,float y,float z);

/** @brief Construct a MaplyBoundingBox from the values given.
    @details The inputs are in degrees and the order is longitude *then* latitude.
    @param degLon0 The left side of the bounding box in degrees.
    @param degLat0 The bottom of the bounding box in degrees.
    @param degLon1 The right side of the bounding box in degrees.
    @param degLat1 The top of the bounding box in degrees.
    @return A MaplyBoundingBox in radians.
  */
MaplyBoundingBox MaplyBoundingBoxMakeWithDegrees(float degLon0,float degLat0,float degLon1,float degLat1);
    
/** @brief Check if two bounding boxes overlap.
    @return Returns true if they did overlap, false otherwise.
 */
bool MaplyBoundingBoxesOverlap(MaplyBoundingBox bbox0,MaplyBoundingBox bbox1);

/** @brief Calculate the great circle distance between two geo coordinates.
    @details This calculates the distance on a sphere between one point and another.
    @param p0 The starting point, lon/lat in radians.
    @param p1 The end point, lon/lat in radians.
    @return The distance between p0 and p1 in meters.
  */
double MaplyGreatCircleDistance(MaplyCoordinate p0,MaplyCoordinate p1);
    
#if __cplusplus
}
#endif

