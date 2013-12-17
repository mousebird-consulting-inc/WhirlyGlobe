/*
 *  PanDelegateFixed.h
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 4/28/11.
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

#import <Foundation/Foundation.h>
#import "WhirlyGlobe.h"

// Sent out when the pan delegate takes control
#define kPanDelegateDidStart @"WKPanDelegateStarted"
// Sent out when the pan delegate finished (but hands off to momentum)
#define kPanDelegateDidEnd @"WKPanDelegateEnded"

// Version of pan delegate specific to this app
// The pan delegate handles panning and rotates the globe accordingly
@interface PanDelegateFixed : NSObject<UIGestureRecognizerDelegate> 

@property(nonatomic,assign) bool northUp;

+ (PanDelegateFixed *)panDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

@end
