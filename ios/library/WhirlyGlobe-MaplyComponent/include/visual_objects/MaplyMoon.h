/*  MaplyMoon.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/2/15.
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
    Utility for calculating moon position.
    
    This is a utility class that figures out where the moon is at a given data and provides the position.
  */
@interface MaplyMoon : NSObject

/** 
    Initialize with a date.
    
    Initialize with the given date/time (UTC).  The moon position will correspond to that.  Must be after 2000.
  */
- (instancetype _Nullable)initWithDate:(NSDate *__nonnull)date;

/// Location on the globe where the moon would land if it fell straight down.  Ouch.
@property (nonatomic, readonly) MaplyCoordinate coordinate;

/// Return the location above the globe in lon/lat/distance.  Yay geocentric!
@property (nonatomic, readonly)  MaplyCoordinate3d position;

/// Illuminated fraction of the moon
@property (nonatomic, readonly) double illuminatedFraction;

/// Phase of the moon.
@property (nonatomic, readonly) double phase;

/// Makes up a light that corresponds to the location at a given time
- (MaplyLight * _Nullable )makeLight;

/// Makes up a light that corresponds to the location at a given time
- (MaplyLight * _Nullable)makeLightWithAmbient:(float)ambient diffuse:(float)diffuse;

@end
