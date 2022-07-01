/*  MaplyZoomGestureDelegate_private.h
 *
 *  Created by Jesse Crocker on 2/4/14.
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
#import "MaplyView.h"
#import "SceneRenderer.h"
#import "MapView_iOS.h"

#define kZoomGestureDelegateDidStart @"WKZoomDelegateStarted"
// Sent out when the pan delegate finished (but hands off to momentum)
#define kZoomGestureDelegateDidEnd @"WKZoomDelegateEnded"

@interface MaplyZoomGestureDelegate ()
{
/// Boundary quad that we're to stay within
  WhirlyKit::Point2dVector bounds;
}

@property (nonatomic) Maply::MapView_iOSRef mapView;

- (const WhirlyKit::Point2dVector &)getBounds;

- (void)setBounds:(const WhirlyKit::Point2d *)bounds;

- (instancetype)initWithMapView:(Maply::MapView_iOSRef)inView;
@end
