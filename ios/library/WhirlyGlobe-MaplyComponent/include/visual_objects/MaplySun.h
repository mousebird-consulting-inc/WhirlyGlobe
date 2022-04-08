/*  MaplySun.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/15.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <UIKit/UIKit.h>
#import <WhirlyGlobe/MaplyComponentObject.h>
#import <WhirlyGlobe/WhirlyGlobeViewController.h>
#import <WhirlyGlobe/MaplyLight.h>

/** 
    Utility for calculating sun position and shading info.
    
    This is a utility class that figures out where the sun is at a given date and provides positional information for lighting calculations.
  */
@interface MaplySun : NSObject

/** 
    Initialize with a date.
    
    Initialize with the given date/time (UTC).  The sun position will correspond to that.
  */
- (_Nullable instancetype)initWithDate:(NSDate *__nonnull)date;

/// Return the vector corresponding to the sun location from the earth.
@property (nonatomic, readonly) MaplyCoordinate3d direction;

/// Returns the location above the globe in lon/lat.  Yay geocentrism!
@property (nonatomic, readonly) MaplyCoordinate3d position;

/// Makes up a light that corresponds to the sun's location at a given time
- (MaplyLight * _Nullable)makeLight;

/// Makes up a light that corresponds to the sun's location at a given time
- (MaplyLight * _Nullable)makeLightWithAmbient:(float)ambient diffuse:(float)diffuse;

@end
