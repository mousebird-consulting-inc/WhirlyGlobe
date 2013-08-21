/*
 *  WGCoordinate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/17/12.
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

#import "MaplyCoordinate.h"

/// WhirlyGlobe just takes geo coordinates.
/// This contains lon and lat values in the x and y fields.
typedef MaplyCoordinate WGCoordinate;

/// Construct a WGCoordinate with longitude and latitude values in degrees
#if __cplusplus
extern "C" {
#endif
    WGCoordinate WGCoordinateMakeWithDegrees(float degLon,float degLat);
#if __cplusplus
}
#endif
