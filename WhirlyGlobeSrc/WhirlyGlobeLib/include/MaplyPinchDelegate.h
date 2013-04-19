/*
 *  MaplyPinchDelegateMap.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import <Foundation/Foundation.h>
#import "MaplyView.h"

@interface MaplyPinchDelegate : NSObject <UIGestureRecognizerDelegate>
{
    /// Minimum allowable zoom level
    float minZoom;
    /// Maximum allowable zoom level
    float maxZoom;
    /// If we're zooming, where we started
    float startZ;
    MaplyView *mapView;
    /// Boundary quad that we're to stay within
    std::vector<WhirlyKit::Point2f> bounds;
}

@property (nonatomic,assign) float minZoom,maxZoom;

/// Create a pinch gesture and a delegate and wire them up to the given UIView
+ (MaplyPinchDelegate *)pinchDelegateForView:(UIView *)view mapView:(MaplyView *)mapView;

/// Set the bounding rectangle
- (void)setBounds:(WhirlyKit::Point2f *)bounds;

@end
