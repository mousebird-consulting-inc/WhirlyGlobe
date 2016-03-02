/*
 *  MaplySun.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/15.
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

#import <UIKit/UIKit.h>
#import "MaplyComponentObject.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyLight.h"

/** @brief Utility for calculating sun position and shading info.
    @details This is a utility class that figures out where the sun is at a given date and provides positional information for lighting calculations.
  */
@interface MaplySun : NSObject

/** @brief Initialize with a date.
    @details Initialize with the given date.  The sun position will correspond to that.
  */
- (id)initWithDate:(NSDate *)date;

/// @brief Return the vector corresponding to the sun location from the earth.
- (MaplyCoordinate3d)getDirection;

/// @brief Makes up a light that corresponds to the sun's location at a given time
- (MaplyLight *)makeLight;

/// @brief Returns the location above the globe in lon/lat.  Yay geocentrism!
- (MaplyCoordinate)asPosition;

@end
