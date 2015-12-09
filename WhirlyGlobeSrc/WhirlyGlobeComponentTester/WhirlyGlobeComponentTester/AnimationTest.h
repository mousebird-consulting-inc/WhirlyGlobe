/*
 *  AnimationTest.h
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/31/13.
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
#import "WhirlyGlobeComponent.h"

/** The animation test object runs a sphere around the globe
    over a defined time period.
  */
@interface AnimatedSphere : MaplyActiveObject

/// Initialize with period (amount of time for one orbit), radius and color of the sphere and a starting point
- (id)initWithPeriod:(float)period radius:(float)radius color:(UIColor *)color viewC:(MaplyBaseViewController *)viewC;

@end
