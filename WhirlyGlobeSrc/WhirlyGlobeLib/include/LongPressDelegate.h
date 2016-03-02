/*
 *  LongPressDelegate.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/11.
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

/** Long press delegate
    responds to a long press by blasting out a notification.
 */
@interface WhirlyGlobeLongPressDelegate : NSObject <UIGestureRecognizerDelegate>

/// Create a long press geture recognizer and a delegate and wire them up to the UIView
+ (WhirlyGlobeLongPressDelegate *)longPressDelegateForView:(UIView *)view globeView:(WhirlyGlobeView *)globeView;

/// The init requires a globe view
- (id)initWithGlobeView:(WhirlyGlobeView *)inView;

@end
