/*
 *  MaplyViewController.h
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
 *  Copyright 2012 mousebird consulting
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
#import <MaplyCoordinate.h>
#import "MaplyScreenMarker.h"
#import "MaplyVectorObject.h"
#import "MaplyViewTracker.h"
#import "MaplyComponentObject.h"
#import "MaplySharedAttributes.h"
#import "MaplyBaseViewController.h"

@class MaplyViewControllerLayer;
@class MaplyViewController;

/** Fill in this protocol for callbacks when the user taps on or near something.
  */
@protocol MaplyViewControllerDelegate <NSObject>

@optional

/// Called when the user taps on or near an object.
/// You're given the object you passed in originally, such as a MaplyScreenMarker
- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj;

/// User tapped at a given location.
/// This won't be called if they tapped and selected, just for taps.
- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord;

@end

/** The main object for the Maply Component.  Create a view controller, add it to
    your view hirarchy and start tossing in data.  You'll want a base layer at the
    very least and then you can add markers, labels, and vectors on top.
  */
@interface MaplyViewController : MaplyBaseViewController
{
    NSObject<MaplyViewControllerDelegate> * __weak delegate;
}

/// Set this to trun on/off the pinch (zoom) gesture recognizer
/// On by default
@property(nonatomic,assign) bool pinchGesture;

/// Set this to turn on or off the rotation gesture recognizer.
/// On by default.
@property(nonatomic,assign) bool rotateGesture;

/// Set this to get callbacks for various events.
@property(nonatomic,weak) NSObject<MaplyViewControllerDelegate> *delegate;

/// Get/set the current height above terrain.
/// The radius of the earth is 1.0.  Height above terrain is relative to that.
@property (nonatomic,assign) float height;

/// Return the view extents.  This is the box the view point is allowed to be within.
- (void)getViewExtentsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur;

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur;

/// Animate to the given position over the given amount of time
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong;

/// Animate the given position to the given screen location over time.
/// If this isn't physically possible, it will do nothing
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong;

/// Set the view to the given position immediately
- (void)setPosition:(MaplyCoordinate)newPos;

/// Set position and height at the same time
- (void)setPosition:(MaplyCoordinate)newPos height:(float)height;

/// Get the current position and height
- (void)getPosition:(WGCoordinate *)pos height:(float *)height;

/// Return the min and max heights above the globe for zooming
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight;

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight;

@end
