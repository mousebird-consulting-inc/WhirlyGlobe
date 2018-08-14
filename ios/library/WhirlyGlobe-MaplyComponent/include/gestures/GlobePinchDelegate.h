/*
 *  PinchDelegateFixed.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 8/22/12.
 *  Copyright 2012-2017 mousebird consulting
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
#import "GlobePinchDelegate.h"

@class WhirlyGlobeRotateDelegate;

/** Protocol for a delegate that handles tilt calculation.
 */
@protocol WhirlyGlobeTiltCalculatorDelegate <NSObject>

/// If this is called, the pan delegate will vary the tilt between the given values for the
///  given height range.
- (void)setMinTilt:(float)minTilt maxTilt:(float)maxTilt minHeight:(float)minHeight maxHeight:(float)maxHeight;

/// Returns true if the tilt zoom mode is set and the appropriate values
- (void)getMinTilt:(float *)retMinTilt maxTilt:(float *)retMaxTilt minHeight:(float *)retMinHeight maxHeight:(float *)retMaxHeight;

/// Return a calculated tilt
- (double)tiltFromHeight:(double)height;

/// Return the maximum allowable tilt
- (double)maxTilt;

/// Called by an actual tilt gesture.  We're setting the tilt as given
- (void)setTilt:(double)newTilt;

@end

// Sent out when the pinch delegate takes control
#define kPinchDelegateDidStart @"WKPinchDelegateStarted"
// Sent out when the pinch delegate finished (but hands off to momentum)
#define kPinchDelegateDidEnd @"WKPinchDelegateEnded"

/** A simple tilt delegate.
  */
@interface WhirlyGlobeStandardTiltDelegate : NSObject<WhirlyGlobeTiltCalculatorDelegate>

// Initialize with a globe view
- (instancetype)initWithGlobeView:(WhirlyGlobeView *)globeView;

@end

/** WhirlyGlobe Pinch Gesture Delegate
 Responds to pinches on a UIView and manipulates the globe view
 accordingly.
 */
@interface WhirlyGlobePinchDelegate : NSObject <UIGestureRecognizerDelegate>

/// Min and max height to allow the user to change
@property (nonatomic,assign) float minHeight,maxHeight;

/// If set we're cooperating with the rotation delegate (HACK!)
@property (nonatomic,weak) WhirlyGlobeRotateDelegate *rotateDelegate;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
/// Also need the view parameters in WhirlyGlobeView
+ (WhirlyGlobePinchDelegate *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

/// If set, we'll zoom around the pinch, rather than the center of the view
@property (nonatomic,assign) bool zoomAroundPinch;

/// If set, we'll rotate around the pinch
@property (nonatomic,assign) bool doRotation;

/// If set, we'll pan around the center point.  If not, we just zoom.
@property (nonatomic,assign) bool allowPan;

/// If set, we'll maintain north as up
@property (nonatomic,assign) bool northUp;

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

// If set, we calculate the tilt every time we update
@property (nonatomic,weak) NSObject<WhirlyGlobeTiltCalculatorDelegate> *tiltDelegate;

// If set, we'll keep track up rather than north up
- (void)setTrackUp:(double)trackUp;

// Turn track up back off
- (void)clearTrackUp;


@end
