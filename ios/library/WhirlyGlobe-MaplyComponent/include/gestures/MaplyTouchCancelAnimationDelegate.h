/*
 *  MaplyTouchCancelAnimationDelegate.mm
 *  WhirlyGlobeLib
 *
 *  Created by Jesse Crocker on 7/15/14.
 *  Copyright 2011-2019 mousebird consulting
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
#import "MapView_iOS.h"

@interface MaplyTouchCancelAnimationDelegate : NSObject <UIGestureRecognizerDelegate>

/// The gesture recognizer
@property (nonatomic,strong) UIGestureRecognizer *gestureRecognizer;
@property (nonatomic) Maply::MapView_iOS *mapView;

/// Create a touch gesture and a delegate and wire them up to the given UIView
+ (MaplyTouchCancelAnimationDelegate*)touchDelegateForView:(UIView *)view mapView:(Maply::MapView_iOS *)mapView;

- (instancetype)initWithMapView:(Maply::MapView_iOS *)inView;

@end
