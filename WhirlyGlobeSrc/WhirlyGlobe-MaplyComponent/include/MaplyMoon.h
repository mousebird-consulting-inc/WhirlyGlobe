/*
 *  MaplyMoon.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/2/15.
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

/** @brief Utility for calculating moon position.
    @details This is a utility class that figures out where the moon is at a given data and provides the position.
  */
@interface MaplyMoon : NSObject

/** @brief Initialize with a date.
    @details Initialize with the given date.  The moon position will correspond to that.  Must be after 2000.
  */
- (id)initWithDate:(NSDate *)date;

/// @brief Location on the globe where the moon would land if it fell straight down.  Ouch.
- (MaplyCoordinate)asCoordinate;

/// @brief Return the location above the globe in lon/lat/distance.  Yay geocentric!
- (MaplyCoordinate3d)asPosition;

/// @brief Illuminated fraction of the moon
@property (readonly) double illuminatedFraction;

/// @brief Phase of the moon.
@property (readonly) double phase;

@end
