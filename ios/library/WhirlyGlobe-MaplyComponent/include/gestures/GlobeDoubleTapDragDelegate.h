/*  GlobeDoubleTapDragDelegate.h
 *
 *  Created by Steve Gifford on 2/7/14.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import <Foundation/Foundation.h>
#import <WhirlyGlobe/GlobePinchDelegate.h>

// Sent out when the double tap delegate takes control
#define kGlobeDoubleTapDragDidStart @"WKGlobeDoubleTapDragStarted"
// Sent out when the double tap delegate finished (but hands off to momentum)
#define kGlobeDoubleTapDragDidEnd @"WKGlobeDoubleTapDragEnded"

@interface WhirlyGlobeDoubleTapDragDelegate : NSObject<UIGestureRecognizerDelegate>

@property (nonatomic,weak) UIGestureRecognizer *gestureRecognizer;

/// Zoom limits
@property (nonatomic) float minZoom,maxZoom;

/// Change if you want a shorter or longer press duration
@property (nonatomic) float minimumPressDuration;

@end
