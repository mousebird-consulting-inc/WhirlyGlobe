/*
 *  GlobeTwoFingerTapDelegate.h
 *
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import "GlobeView.h"
#import "PinchDelegate.h"

@interface WhirlyGlobeTwoFingerTapDelegate : NSObject<UIGestureRecognizerDelegate>

/// Create a double tap gesture and a delegate and wire them up to the given UIView
+ (WhirlyGlobeTwoFingerTapDelegate *)twoFingerTapDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

// How much we zoom in by
@property (nonatomic) float zoomTapFactor;

// How long the zoom animation takes
@property (nonatomic) float zoomAnimationDuration;

/// Zoom limits
@property (nonatomic) float minZoom,maxZoom;

// If set, we calculate the tilt every time we update
@property (nonatomic,weak) NSObject<WGTiltCalculatorDelegate> *tiltDelegate;

@end
