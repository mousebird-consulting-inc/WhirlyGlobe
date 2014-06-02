/*
 *  PinchDelegateFixed.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/22/12.
 *  Copyright 2012 mousebird consulting
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
#import "GlobeView.h"

// Sent out when the pinch delegate takes control
#define kPinchDelegateDidStart @"WKPinchDelegateStarted"
// Sent out when the pinch delegate finished (but hands off to momentum)
#define kPinchDelegateDidEnd @"WKPinchDelegateEnded"

/** WhirlyGlobe Pinch Gesture Delegate
 Responds to pinches on a UIView and manipulates the globe view
 accordingly.
 */
@interface WGPinchDelegateFixed : NSObject <UIGestureRecognizerDelegate>

/// Min and max height to allow the user to change
@property (nonatomic,assign) float minHeight,maxHeight;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
/// Also need the view parameters in WhirlyGlobeView
+ (WGPinchDelegateFixed *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

/// If this is called, the pan delegate will vary the tilt between the given values for the
///  given height range.
- (void)setMinTilt:(float)minTilt maxTilt:(float)maxTilt minHeight:(float)minHeight maxHeight:(float)maxHeight;

/// Returns true if the tilt zoom mode is set and the appropriate values
- (bool)getMinTilt:(float *)retMinTilt maxTilt:(float *)retMaxTilt minHeight:(float *)retMinHeight maxHeight:(float *)retMaxHeight;

/// If set, we'll zoom around the pinch, rather than the center of the view
@property (nonatomic,assign) bool zoomAroundPinch;

/// If set, we'll rotate around the pinch
@property (nonatomic,assign) bool doRotation;

/// If set, we'll maintain north as up
@property (nonatomic,assign) bool northUp;

/// Turn off the tilt controlled by zoom height
- (void)clearTiltZoom;

/// Calculate the current tilt based on the tilt zoom values, if they're there
- (float)calcTilt;

@end
