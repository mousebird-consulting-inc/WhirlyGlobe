/*
 *  PinchDelegate.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/17/11.
 *  Copyright 2011-2013 mousebird consulting
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

/** Protocol for a delegate that handles tilt calculation.
 */
@protocol WGTiltCalculatorDelegate <NSObject>

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

/** WhirlyGlobe Pinch Gesture Delegate
	Responds to pinches on a UIView and manipulates the globe view
	accordingly.
 */
@interface WhirlyGlobePinchDelegate : NSObject <UIGestureRecognizerDelegate>

/// If set, we'll also handle rotation.  On by default.
@property (nonatomic,assign) bool doRotation;

/// If set, we'll zoom around the pinch, rather than the center of the view
@property (nonatomic,assign) bool zoomAroundPinch;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
/// Also need the view parameters in WhirlyGlobeView
+ (WhirlyGlobePinchDelegate *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

@end
