/*
 *  PinchDelegate.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/17/11.
 *  Copyright 2011-2012 mousebird consulting
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

/** WhirlyGlobe Pinch Gesture Delegate
	Responds to pinches on a UIView and manipulates the globe view
	accordingly.
 */
@interface WhirlyGlobePinchDelegate : NSObject <UIGestureRecognizerDelegate>
{
    /// If we're in the process of zooming in, where we started
	float startZ;
	WhirlyGlobeView *globeView;
}

/// Create a pinch gesture and a delegate and wire them up to the given UIView
/// Also need the view parameters in WhirlyGlobeView
+ (WhirlyGlobePinchDelegate *)pinchDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

@end
