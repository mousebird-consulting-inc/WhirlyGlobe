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
#import "PinchDelegate.h"

@class WhirlyGlobeRotateDelegate;

// Sent out when the pinch delegate takes control
#define kPinchDelegateDidStart @"WKPinchDelegateStarted"
// Sent out when the pinch delegate finished (but hands off to momentum)
#define kPinchDelegateDidEnd @"WKPinchDelegateEnded"

/** A simple tilt delegate.
  */
@interface WGStandardTiltDelegate : NSObject<WGTiltDelegate>
@end

/** WhirlyGlobe Pinch Gesture Delegate
 Responds to pinches on a UIView and manipulates the globe view
 accordingly.
 */
@interface WGPinchDelegateFixed : NSObject <UIGestureRecognizerDelegate>

/// Min and max height to allow the user to change
@property (nonatomic,assign) float minHeight,maxHeight;

/// If set we're cooperating with the rotation delegate (HACK!)
@property (nonatomic,weak) WhirlyGlobeRotateDelegate *rotateDelegate;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
/// Also need the view parameters in WhirlyGlobeView
+ (WGPinchDelegateFixed *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

/// If set, we'll zoom around the pinch, rather than the center of the view
@property (nonatomic,assign) bool zoomAroundPinch;

/// If set, we'll rotate around the pinch
@property (nonatomic,assign) bool doRotation;

/// If set, we'll maintain north as up
@property (nonatomic,assign) bool northUp;

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

// If set, we calculate the tilt every time we update
@property (nonatomic,weak) NSObject<WGTiltDelegate> *tiltDelegate;


@end
