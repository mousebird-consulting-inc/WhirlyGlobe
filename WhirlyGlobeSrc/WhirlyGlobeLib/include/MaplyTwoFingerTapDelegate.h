/*
 *  MaplyTwoFingerTapDelegate.h
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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
#import "MaplyView.h"
#import "MaplyZoomGestureDelegate.h"

@interface MaplyTwoFingerTapDelegate : MaplyZoomGestureDelegate

/// Create a 2 finger tap gesture and a delegate and wire them up to the given UIView
+ (MaplyTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view mapView:(MaplyView *)mapView;

// How long we animate from one place to the next
@property (nonatomic) float animTime;


@end
