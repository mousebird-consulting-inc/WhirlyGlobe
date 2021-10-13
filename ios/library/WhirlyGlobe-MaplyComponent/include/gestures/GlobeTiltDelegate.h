/*
 *  TiltDelegate.h
 *
 *  Created by Stephen Gifford on 1/5/15.
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

#import <Foundation/Foundation.h>
#import "WhirlyGlobe.h"
#import "GlobePinchDelegate.h"
#import "GlobeView_iOS.h"

// Sent out when the tilt delegate takes control
#define kTiltDelegateDidStart @"WKTiltDelegateStarted"
// Sent out when the tilt delegate finished (but hands off to momentum)
#define kTiltDelegateDidEnd @"WKTiltDelegateEnded"

// The tilt delegate handle the 3D camera tilt
@interface WhirlyGlobeTiltDelegate : NSObject<UIGestureRecognizerDelegate>

+ (WhirlyGlobeTiltDelegate *)tiltDelegateForView:(UIView *)view globeView:(WhirlyGlobe::GlobeView_iOS *)globeView;

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

// Set so we can turn off the pinch delegate when we're working
@property (nonatomic,weak) WhirlyGlobePinchDelegate *pinchDelegate;

// Calculator delegate for tilt constraints
@property (nonatomic) WhirlyGlobe::TiltCalculatorRef tiltCalcDelegate;

@end
