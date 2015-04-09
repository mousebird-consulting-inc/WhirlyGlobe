/*
 *  MaplyZoomGestureDelegate.h
 *
 *
 *  Created by Jesse Crocker on 2/4/14.
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

#import <Foundation/Foundation.h>
#import "MaplyView.h"

#define kZoomGestureDelegateDidStart @"WKZoomDelegateStarted"
// Sent out when the pan delegate finished (but hands off to momentum)
#define kZoomGestureDelegateDidEnd @"WKZoomDelegateEnded"

@interface MaplyZoomGestureDelegate : NSObject <UIGestureRecognizerDelegate>
{
  /// Boundary quad that we're to stay within
  std::vector<WhirlyKit::Point2f> bounds;
}

/// Minimum allowable zoom level
@property (nonatomic,assign) float minZoom;
/// Maximum allowable zoom level
@property (nonatomic,assign) float maxZoom;
//The gesture recognizer
@property (nonatomic,strong) UIGestureRecognizer *gestureRecognizer;
@property (nonatomic,weak) MaplyView *mapView;

/// Set the bounding rectangle
- (void)setBounds:(WhirlyKit::Point2f *)bounds;

- (instancetype)initWithMapView:(MaplyView *)inView;

@end
