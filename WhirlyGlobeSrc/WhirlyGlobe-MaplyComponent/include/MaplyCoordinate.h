/*
 *  MaplyCoordinate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2012 mousebird consulting
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

typedef struct
{
    float x,y;
} MaplyCoordinate;

/// The standard 3-space coordinate
typedef struct
{
    float x,y,z;
} MaplyCoordinate3d;

#if __cplusplus
extern "C" {
#endif
/// Construct a MaplyGeoCoordinate with longitude and latitude values in degrees
MaplyCoordinate MaplyCoordinateMakeWithDegrees(float degLon,float degLat);
/// Convenience function for creating a 3D Maply coordinate
MaplyCoordinate3d MaplyCoordinate3dMake(float x,float y,float z);
#if __cplusplus
}
#endif

