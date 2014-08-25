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

/** @brief A protocol to fill out for selection and tap messages from the MaplyViewController.
    @details Fill out the protocol when you want to get back selection and tap messages.  All the methods are optional.
  */
@protocol MaplyViewControllerDelegate <NSObject>

@optional

/** @brief Called when the user taps on or near an object.
    @details You're given the object you passed in originally, such as a MaplyScreenMarker.  You can set a userObject on most of these to put your own data in there for tracking.
  */
- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj;

/** @brief User selected a given object and tapped at a given location.
    @details This is called when the user selects an object.  It differs from maplyViewController:didSelect: in that it passes on the location (in the local coordinate system) and the position on screen.
    @param viewC View Controller that saw the selection.
    @param selectedObj The object selected.  Probably one of MaplyVectorObject or MaplyScreenLabel or so on.
    @param coord Location in the local coordinate system where the user tapped.
    @param screenPt Location on screen where the user tapped.
 */
- (void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj atLoc:(WGCoordinate)coord onScreen:(CGPoint)screenPt;

/** @brief User tapped at a given location.
    @details This is a tap at a specific location on the map.  This won't be called if they tapped and selected, just for taps.
  */
- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord;

/** @brief Called when the map starts moving.
 @param viewC The map view controller.
 @param userMotion Set if this is motion being caused by the user, rather than a call to set location.
 @details This is called when something (probably the user) starts moving the map.
 */
- (void)maplyViewControllerDidStartMoving:(MaplyViewController *)viewC userMotion:(bool)userMotion;

/** @brief Called when the map stops moving.
 @details This is called when the map stops moving.  It passes in the corners of the current viewspace.
 @param viewC The globe view controller.
 @param userMotion Set if this is motion being caused by the user, rather than a call to set location.
 @param corners An array of length 4 containing the corners of the view space (lower left, lower right, upper right, upper left).  If any of those corners does not intersect the map (think zoomed out), its values are set to MAXFLOAT.
 */
- (void)maplyViewController:(MaplyViewController *)viewC didStopMoving:(MaplyCoordinate *)corners userMotion:(bool)userMotion;

/** @brief Called when the user taps on one of your annotations.
    @details This is called when the user taps on an annotation.
    @param annotation Which annotation they tapped on.
  */
- (void)maplyViewController:(MaplyViewController *)viewC didTapAnnotation:(MaplyAnnotation*)annotation;

/// Note: Deprecated
- (void)maplyViewController:(MaplyViewController *)viewC didClickAnnotation:(MaplyAnnotation*)annotation;

@end

/** @brief This view controller implements a map.
    @details This is the main entry point for displaying a 2D or 3D map.  Create one of these, fill it with data and let your users mess around with it.
    @details You can display a variety of features on the map, including tile base maps (MaplyQuadImageTilesLayer), vectors (MaplyVectorObject), shapes (MaplyShape), and others.  Check out the add calls in the MaplyBaseViewController for details.
    @details The Maply View Controller can be initialized in 3D map, 2D map mode.  The 2D mode can be tethered to a UIScrollView if you want to handle gestures that way.  That mode is very specific at the moment.
    @details To get selection and tap callbacks, fill out the MaplyViewControllerDelegate and assign the delegate.
    @details Most of the functionality is shared with MaplyBaseViewController.  Be sure to look in there first.
  */
@interface MaplyViewController : MaplyBaseViewController

/// @brief Initialize as a 3D map.
- (id)init;

/// @brief Initialize as a 2D map.
- (id)initAsFlatMap;

/** @brief Initialize as a 2D map tied to a UIScrollView.
    @details In this mode we disable all the the gestures.
    @param scrollView The UIScrollView to track.
    @param tetherView If set, we assume the scroll view is manipulating a blank UIView which we'll watch.
  */
- (id)initAsTetheredFlatMap:(UIScrollView *)scrollView tetherView:(UIView *)tetherView;

/** @brief Reset the UIScrollView for tethered mode.
    @details Occasionally we need to reset the UIScrollView and tether view.  This will do that.
    @param scrollView The UIScrollView to track.
    @param tetherView If set, we assume the scroll view is manipulating a blank UIView which we'll watch.
  */
- (void)resetTetheredFlatMap:(UIScrollView *)scrollView tetherView:(UIView *)tetherView;

/// @brief Set if we're in 2D mode.
@property (nonatomic,readonly) bool flatMode;

/// @brief If we're in tethered flat map mode, this is the view we're monitoring for size and offset changes.
@property(nonatomic,weak) UIView *tetherView;

/// @brief If set before startup (after init), we'll turn off all gestures and work only in tethered mode.
@property(nonatomic,assign) bool tetheredMode;

/// @brief Set the coordinate system to use in display.
/// @details The coordinate system needs to be valid in flat mode.  The extents, if present, will be used to define the coordinate system origin.
/// @details nil is the default and will result in a full web style Spherical Mercator.
@property(nonatomic) MaplyCoordinateSystem *coordSys;

/** @brief Set the center of the display coordinate system.
    @details This is (0,0,0) by default.  If you set it to something else all display coordinates will be offset from that origin.
    @details The option is useful when displaying small maps (of a city, say) at very high resolution.
  */
@property(nonatomic) MaplyCoordinate3d displayCenter;

/** @brief Turn the pinch (zoom) gesture recognizer on and off
    @details On by default.
  */
@property(nonatomic,assign) bool pinchGesture;

/** @brief Turn the rotate gesture recognizer on and off
    @details On by default.
 */
@property(nonatomic,assign) bool rotateGesture;

/** @brief Turn the pan gesture on and off
    @details Pan gesture is on by default
  */
@property(nonatomic,assign) bool panGesture;

/** @brief Turn the double tap to zoom gesture recognizer on and off
    @details On by default.
 */
@property(nonatomic,assign) bool doubleTapZoomGesture;

/** @brief Turn the 2 finger tap to zoom out gesture recognizer on and off
    @details On by default.
 */
@property(nonatomic,assign) bool twoFingerTapGesture;

/** @brief Turn on the double tap and drag gesture to zoom in and out.
    @details On by default.
  */
@property(nonatomic,assign) bool doubleTapDragGesture;

/** @brief turn the touch to cancel animation gesture on and off
    @details off by default
 */
@property(nonatomic,assign) bool cancelAnimationOnTouch;

/** @brief The current rotation away from north.
 */
@property(nonatomic,assign) float heading;

/** @brief If set, we'll automatically move to wherever the user tapped.
    @details When on we'll move the current location to wherever the user tapped if they tapped the globe.  That's true for selection as well.  On by default.
 */
@property(nonatomic,assign) bool autoMoveToTap;

/** @brief Delegate for selection and location tapping.
    @details Fill in the MaplyViewControllerDelegate and assign it here to get callbacks for object selection and tapping.
  */
@property(nonatomic,weak) NSObject<MaplyViewControllerDelegate> *delegate;

/** @brief Current height above terrain.
    @details In 3D map mode this is the height from which the user is viewing the map.  Maps are usually -PI to +PI along their horizontal edges.
  */
@property (nonatomic,assign) float height;

/** @brief 2D visual views can do some simple wrapping.  This turns that on and off (off by default).
    @details On some 2D visual views we're allowed to wrap across the edge of the world.  This will attempt to do that.
  */
@property (nonatomic,assign) bool viewWrap;

/** @brief The box the view point can be in.
    @details This is the box the view point is allowed to be within.  The view controller will constrain it to be within that box.  Coordinates are in geographic (radians).
  */
- (void)getViewExtentsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur;

/** @brief The box the view point can be in.
    @details This is the box the view point is allowed to be within.  The view controller will constrain it to be within that box. Coordinates are in geographic (radians).
 */
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur;

/** @brief Animate to the given position over time.
    @param newPos A coordinate in geographic (lon/lat radians)
    @param howLong A time in seconds.
  */
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong;

/** @brief Animate to new window extents over time.
    @details This method is similar to animateToPosition:time: but only works in 2D flat map mode.
    @param windowSize The new window size we're matching.
    @param contentOffset The contentOffset from the UIScrollView.
    @param howLong The length of time to take getting there.
  */
- (void)animateToExtentsWindowSize:(CGSize)windowSize contentOffset:(CGPoint)contentOffset time:(NSTimeInterval)howLong;

/** @brief Animate the given position to the screen position over time.
    @details This is similar to animateToPosition:time: except that it will attempt to match up the screen position and the geographic position.  This is how you offset the location you're looking at.
    @details If it's impossible to move newPos to loc, then nothing happens.
    @param newPos The geographic position (lon/lat in radians) to move to.
    @param loc The location on the screen where we'd like it to go.
    @param howLong How long in seconds to take getting there.
  */
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong;

/** @brief Set the center of the screen to the given position immediately.
    @param newPos The geographic position (lon/lat in radians) to move to.
  */
- (void)setPosition:(MaplyCoordinate)newPos;

/** @brief Set the center of the screen and the height offset immediately.
    @param newPos The geographic position (lon/lat in radians) to move to.
    @param height Height the view point above the map.
  */
- (void)setPosition:(MaplyCoordinate)newPos height:(float)height;

/** @brief Return the current center position and height.
    @param pos The center of the screen in geographic (lon/lat in radians).
    @param height The current view point's height above the map.
  */
- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height;

/** @brief Return the zoom limits for 3D map mode.
    @param minHeight The closest a viewer is allowed to get to the map surface.
    @param maxHeight The farthest away a viewer is allowed to get from the map surface.
  */
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight;

/** @brief Set the zoom limits for 3D map mode.
    @param minHeight The closest a viewer is allowed to get to the map surface.
    @param maxHeight The farthest away a viewer is allowed to get from the map surface.
 */
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight;

/** @brief Return the geographic (lon/lat radians) coordinate in radians for a given screen point.
    @return Returns the geo coordinate corresponding to a given screen point in radians.
 */
- (MaplyCoordinate)geoFromScreenPoint:(CGPoint)point;

/** @brief Find a height that shows the given bounding box.
    @details This method will search for a height that shows the given bounding box within the view.  The search is inefficient, so don't call this a lot.
    @param The bounding box (in radians) we're trying to view.
    @param pos Where the view will be looking.
  */
- (float)findHeightToViewBounds:(MaplyBoundingBox *)bbox pos:(MaplyCoordinate)pos;

/**
 @brief Return the extents of the current view
 @return Returns the Bounding Box (in radians) corresponding to the current view
 */
- (MaplyBoundingBox) getCurrentExtents;

@end
