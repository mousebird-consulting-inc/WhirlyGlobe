/*
 *  TapDelegate.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/3/11.
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

#import <UIKit/UIKit.h>
#import "GlobeView.h"
#import "TapMessage.h"

/** WhirlyGlobe tap gesture delegate
	responds to taps by blasting out a notification.
 */
@interface WhirlyGlobeTapDelegate : NSObject <UIGestureRecognizerDelegate>

/// Create a tap gesture recognizer and a delegate and wire them up to the given UIView
+ (WhirlyGlobeTapDelegate *)tapDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

@end
