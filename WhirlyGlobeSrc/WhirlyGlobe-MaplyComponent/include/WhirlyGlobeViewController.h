/*
 *  GlobeViewController.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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
#import "MaplyBaseViewController.h"

@class WGViewControllerLayer;
@class WhirlyGlobeViewController;

/** @brief Animation State used by the WhirlyGlobeViewControllerAnimationDelegate.
    @details You fill out one of these when you're implementing the animation delegate.  Return it and the view controller will set the respective values to match.
  */
@interface WhirlyGlobeViewControllerAnimationState : NSObject

/// @brief Heading is calculated from due north
/// @details If not set or set to MAXFLOAT, this is ignored
@property (nonatomic) float heading;

/// @brief Height above the globe
@property (nonatomic) float height;

/// @brief Tilt as used in the view controller
/// @details If not set or set to MAXFLOAT, we calculate tilt the regular way
@property (nonatomic) float tilt;

/// @brief Position to move to on the globe
@property (nonatomic) MaplyCoordinate pos;

/// @brief If set, this is a point on the screen where pos should be.
/// @details By default this is (-1,-1) meaning the screen position is just the middle.  Otherwise, this is where the position should wind up on the screen, if it can.
@property (nonatomic) CGPoint screenPos;

/** @brief Interpolate a new state between the given states A and B.
    @details This does a simple interpolation (lat/lon, not great circle) between the two animation states.
  */
+ (WhirlyGlobeViewControllerAnimationState * __nonnull)Interpolate:(double)t from:(WhirlyGlobeViewControllerAnimationState * __nonnull)stateA to:(WhirlyGlobeViewControllerAnimationState * __nonnull)stateB;

@end

/** @brief An animation delegate that can be set on a WhirlyGlobeViewController to control the view over time.
    @details Filling out these methods will get you animation callbacks at the proper time to control position, heading, tilt, and height on a frame basis.
    @details You pass the resulting object in to
  */
@protocol WhirlyGlobeViewControllerAnimationDelegate <NSObject>

/** @brief This method is called when the animation starts.
    @details At the animation start we collect up the various parameters of the current visual view state and pas them in via the startState.  You probably want to keep track of this for later.
    @param viewC The view controller doing the animation.
    @param startState The starting point for the visual view animation.  Cache this somewhere for your own interpolation.
    @param startTime When the animation starts (e.g. now)
    @param endTime When the animation ends.  This is an absolute value.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC startState:(WhirlyGlobeViewControllerAnimationState * __nonnull)startState startTime:(NSTimeInterval)startTime endTime:(NSTimeInterval)endTime;

/** @brief This method is called at the beginning of every frame draw to position the viewer.
    @details This is the method that does all the work.  You need to fill out the returned WhirlyGlobeViewControllerAnimationState according to whatever interpolation your'e doing based on the currentTime.
    @param viewC The view controller doing the animation.
    @param currentTime The time for this frame.  Use this rather than calculating the time yourself.
    @return The WhirlyGlobeViewControllerAnimationState expressing where you want the viewer to be and where they are looking.
  */
- (WhirlyGlobeViewControllerAnimationState * __nonnull)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC stateForTime:(NSTimeInterval)currentTime;

@optional

/** @brief This method is called at the end of the animation.
    @details The globe view controller calls this method when the animation is finished.  Do your cleanup here if need be.
    @param viewC The globe view controller.
  */
- (void)globeViewControllerDidFinishAnimation:(WhirlyGlobeViewController * __nonnull)viewC;

@end

/** @brief A simple animation delegate for moving the globe around.
    @details The animation delegate support provides a lot of flexibility.  This version just provides all the standard fields and interpolates from beginning to end.
  */
@interface WhirlyGlobeViewControllerSimpleAnimationDelegate : NSObject <WhirlyGlobeViewControllerAnimationDelegate>

/// @brief Initialize with an animation state to copy
- (id __nonnull)initWithState:(WhirlyGlobeViewControllerAnimationState * __nonnull)endState;

/// @brief Location at the end of the animation
@property (nonatomic) MaplyCoordinate loc;

/// @brief Heading at the end of the animation
@property (nonatomic) double heading;

/// @brief Height at the end of the animation
@property (nonatomic) double height;

/// @brief Tilt at the end of the animation
@property (nonatomic) double tilt;

@end

/** @brief Globe View Controller Delegate protocol for getting back selection and tap events.
    @details Fill out the methods in this protocol and assign yourself as a delegate in the WhirlyGlobeViewController to get selection and tap events.
  */
@protocol WhirlyGlobeViewControllerDelegate <NSObject>

@optional
/** @brief Called when the user taps on or near an object.
    @details You're given the object you passed in originally, such as a MaplyScreenMarker.  Most of those objects have userObject properties, which is a good place to stash your own data.
    @param viewC The view controller where the user selected something.
    @param selectedObj The Maply object they selected.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC didSelect:(NSObject * __nonnull)selectedObj;

/** @brief Called when the user taps on or near an object.
    @details This will call back with the closest object it finds near (or on) where the user tapped.
    @details You're given the object you passed in originally, such as a MaplyScreenMarker.
    @details This version is called preferentially if it exists.  Otherwise globeViewController:didSelect: is called if it exists.
    @param viewC The view controller where the user selected something.
    @param selectedObj The Maply object they selected.
    @param coord The location (geographic lon/lat in radians) where the user tapped.
    @param screenPt The location on screen where the user tapped.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC didSelect:(NSObject * __nonnull)selectedObj atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt;

/** @brief Called when the user taps on or near one or more objects.  Returns them all.
    @details This method is called when the
    @param viewC The view controller where the user selected something.
    @param selectedObjs A list of
    @param coord The location (geographic lon/lat in radians) where the user tapped.
    @param screenPt The location on screen where the user tapped.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC allSelect:(NSArray * __nonnull)selectedObjs atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt;

/** @brief Called when the user taps outside the globe.
  */
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController * __nonnull)viewC;

/** @brief Called when the user taps the globe but doesn't select anything.
    @param viewC The view controller where the user selected something.
    @param coord The location (geographic lon/lat in radians) where the user tapped.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC didTapAt:(WGCoordinate)coord;

/** @brief This is an older method called when some layers load.
    @details Certain image layers call this method when they finish loading.  More modern layers don't, so don't rely on this.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC layerDidLoad:(WGViewControllerLayer * __nonnull)layer;

/** @brief Called when the globe starts moving.
    @param viewC The globe view controller.
    @param userMotion Set if this is motion being caused by the user, rather than a call to set location.
    @details This is called when something (probably the user) starts moving the globe.
  */
- (void)globeViewControllerDidStartMoving:(WhirlyGlobeViewController * __nonnull)viewC userMotion:(bool)userMotion;

/** @brief Called when the globe stops moving.
    @details This is called when the globe stops moving.  It passes in the corners of the current viewspace.
    @param viewC The globe view controller.
    @param userMotion Set if this is motion being caused by the user, rather than a call to set location.
    @param corners An array of length 4 containing the corners of the view space (lower left, lower right, upper right, upper left).  If any of those corners does not intersect the globe (think zoomed out), its values are set to MAXFLOAT.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC didStopMoving:(MaplyCoordinate * __nonnull)corners userMotion:(bool)userMotion;

/** @brief Called when an animation that knows where it's going to stop start ups.
    @details This is called when we know where the globe will stop.  It passes in the corners of that future viewspace.
    @param viewC The globe view controller.
    @param corners An array of length 4 containing the corners of the view space (lower left, lower right, upper right, upper left).  If any of those corners does not intersect the globe (think zoomed out), its values are set to MAXFLOAT.
    @param userMotion Set if this is motion being caused by the user, rather than a call to set location.
 */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC willStopMoving:(MaplyCoordinate * __nonnull)corners userMotion:(bool)userMotion;

/** @brief Called whenever the viewpoint moves.
    @details This is called whenever the viewpoint moves.  That includes user motion as well as animations.
    @details It may be triggered as often as every frame.  If that's a problem, use the globeViewController:didStopMoving:userMotion: or globeViewController:willStopMoving:userMotion: calls.
    @param viewC The globe view controller.
    @param corners An array of length 4 containing the corners of the view space (lower left, lower right, upper right, upper left).  If any of those corners does not intersect the globe (think zoomed out), its values are set to MAXFLOAT.
  */
- (void)globeViewController:(WhirlyGlobeViewController * __nonnull)viewC didMove:(MaplyCoordinate * __nonnull)corners;

@end

/** @brief This view controller implements a 3D interactive globe.
    @details This is the main entry point for displaying a globe.  Create one of these, fill it with data and let your users mess around with it.
    @details You can display a variety of features on the globe, including tiled base maps (MaplyQuadImageTilesLayer), vectors (MaplyVectorObject), shapes (MaplyShape), and others.  Check out the add calls in the MaplyBaseViewController for details.
    @details To get selection and tap callbacks, fill out the WhirlyGlobeViewControllerDelegate and assign the delegate.
    @details Most of the functionality is shared with MaplyBaseViewController.  Be sure to look in there first.
 */
@interface WhirlyGlobeViewController : MaplyBaseViewController

/** @brief If set, keep north facing upward on the screen as the user moves around.
    @details Off by default.
  */
@property(nonatomic,assign) bool keepNorthUp;

/** @brief Turn the pinch (zoom) gesture recognizer on and off
    @details On by default.
 */
@property(nonatomic,assign) bool pinchGesture;

/** @brief Turn the rotate gesture recognizer on and off
    @details On by default.
 */
@property(nonatomic,assign) bool rotateGesture;

/** @brief Turn the tilte gesture recognizer on and off
    @details Off by default.
 */
@property(nonatomic,assign) bool tiltGesture;

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


/** @brief If set, we'll automatically move to wherever the user tapped.
    @details When on we'll move the current location to wherever the user tapped if they tapped the globe.  That's true for selection as well.  On by default.
  */
@property(nonatomic,assign) bool autoMoveToTap;

/** @brief Delegate for the selection and tap events.
    @details Fill in the WhirlyGlobeViewControllerDelegate protocol, assign the object here and you'll get selection and tap events.
  */
@property(nonatomic,weak) NSObject<WhirlyGlobeViewControllerDelegate> * __nullable delegate;

/** @brief Current viewer height above terrain.
    @details This is the height from with the viewer is viewing the globe.  Values range from minHeight to maxHeight.  Smaller is closer.  See getZoomLimitsMin:max: for values.  The display units are based on a globe with a radius of 1.0.
 */
@property (nonatomic,assign) float height;

/** @brief Tilt in radians.  0 is looking straight down (the default).  PI/2 is looking toward the horizon.
  */
@property(nonatomic,assign) float tilt;

/** @brief The current rotation away from north.
    @details If keepNorthUp is set this is always 0.
  */
@property(nonatomic,assign) float heading;

/** @brief Return the zoom limits for the globe.
    @param minHeight The closest a viewer is allowed to get to the globe surface.
    @param maxHeight The farthest away a viewer is allowed to get from the globe surface.
 */
- (void)getZoomLimitsMin:(float * __nonnull)minHeight max:(float * __nonnull)maxHeight;

/** @brief Set the zoom limits for the globe.
    @param minHeight The closest a viewer is allowed to get to the globe surface.
    @param maxHeight The farthest away a viewer is allowed to get from the globe surface.
 */
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight;

/** @brief How much we zoom in or out by when the user double taps or two finger taps.
    @details This sets the factor we'll use to zoom in by (e.g. *2.0) when the user double taps.  It also sets how much we zoom out by when the user two finger taps.  This will only have an effect if those gestures are active.
  */
@property (nonatomic) float zoomTapFactor;

/** @brief How long we take to zoom in or out when the user double taps or two finger taps.
    @details This controls the duration of the zoom animation.  You can set it to zero to avoid the animation entirely.
  */
@property (nonatomic) float zoomTapAnimationDuration;

/** @brief Set the simplified tilt mode.  We'll tilt toward the horizon as the user gets closer to the ground.
    @details This implements a simplified mode for tilting.  As the user gets closer to the ground we tilt more toward the horizon.
    @param minHeight The minimum height corresponding to minTilt.
    @param maxHeight The height at which to start interoplating tilt.
    @param minTilt The most tilt toward the horizon.  Invoked when the user is at minHeight or below.
    @param maxTilt The tilt at the maximum height and over.  The tilt will never be less than this, so typically 0.
  */
- (void)setTiltMinHeight:(float)minHeight maxHeight:(float)maxHeight minTilt:(float)minTilt maxTilt:(float)maxTilt;

/// @brief Turn off the varying tilt set up by setTiltMinHeight:maxHeight:minTilt:maxTilt:
- (void)clearTiltHeight;

/** @brief Turn on autorotate to rotate by the given amount every second.
    @details This turns on an auto-rotate mode.  The globe will start rotating after a delay by the given number of degrees per second.  Very pleasant.
    @param autoRotatInterval Wait this number of seconds after user interaction to auto rotate.
    @param autoRotateDegrees Rotate this number of degrees (not radians) per second.
  */
- (void)setAutoRotateInterval:(float)autoRotateInterval degrees:(float)autoRotateDegrees;

/** @brief Animate to the given position over time.
    @param newPos A coordinate in geographic (lon/lat radians)
    @param howLong A time interval in seconds.
 */
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong;

/** @brief Animate the given position to the screen position over time.
    @details This is similar to animateToPosition:time: except that it will attempt to match up the screen position and the geographic position.  This is how you offset the location you're looking at.
    @details If it's impossible to move newPos to loc, then nothing happens.
    @param newPos The geographic position (lon/lat in radians) to move to.
    @param loc The location on the screen where we'd like it to go.
    @param howLong How long in seconds to take getting there.
 */
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong;

/** @brief Animate to the given position, heading and height over time.
    @param newPos A coordinate in geographic (lon/lat radians)
    @param newHeight New height to animate to.
    @param newHeading New heading to finish on.
    @param howLong A time interval in seconds.
 */
- (bool)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight heading:(float)newHeading time:(NSTimeInterval)howLong;

/** @brief Animate with a delegate over time.
    @details Fill in the WhirlyGlobeViewControllerAnimationDelegate and you can control the visual view on a frame by frame basis.  You'll get called back at the appropriate time on the main thread over the time period.
    @details You'll also be called one at the end of the animation to establish the final position.
    @param animationDelegate The objects that implements the WhirlyGlobeViewControllerAnimationDelegate protocol.
    @param howLong How long the animation will run from the present time.
  */
- (void)animateWithDelegate:(NSObject<WhirlyGlobeViewControllerAnimationDelegate> * __nonnull)animationDelegate time:(NSTimeInterval)howLong;

/** @brief Set the center of the screen to the given position immediately.
    @param newPos The geographic position (lon/lat in radians) to move to.
 */
- (void)setPosition:(MaplyCoordinate)newPos;

/** @brief Set the center of the screen and the height offset immediately.
    @param newPos The geographic position (lon/lat in radians) to move to.
    @param height Height the view point above the globe.
 */
- (void)setPosition:(MaplyCoordinate)newPos height:(float)height;

/** @brief Return the current center position and height.
    @param pos The center of the screen in geographic (lon/lat in radians).
    @param height The current view point's height above the globe.
 */
- (void)getPosition:(MaplyCoordinate * __nonnull)pos height:(float * __nonnull)height;

/** @brief Set the viewing state all at once
    @details This sets the position, tilt, height, screen position and heading all at once.
  */
- (void)setViewState:(WhirlyGlobeViewControllerAnimationState * __nonnull)viewState;

/** @brief Make a WhirlyGlobeViewControllerAnimationState object from the current view state.
    @details This returns the current view parameters in a single WhirlyGlobeViewControllerAnimationState.
  */
- (WhirlyGlobeViewControllerAnimationState * __nullable)getViewState;

/** @brief Return a view state looking at the given location.
    @details Creates a view state that looks at the given location, taking tilt and heading into account.
    @param coord The location the user will be looking at.
    @param tilt Tilt off of vertical.
    @param heading Heading calculated from due north.
    @param alt Altitude of the point the user will be looking at (0, is a good value).
    @param range How far the user will be from the location they're looking at.
    @return The view state encapsulating the user location.  Will be nil if the parameters weren't valid.
  */
- (WhirlyGlobeViewControllerAnimationState * __nullable)viewStateForLookAt:(MaplyCoordinate)coord tilt:(float)tilt heading:(float)heading altitude:(float)alt range:(float)range;

/** @brief Apply viewing constraints to the given view state.
    @details This applies active viewing constraints, such as min and max height and calculated tilt, if it's on to the given view state. This is particularly useful when controlled tilt is on.
  */
- (void)applyConstraintsToViewState:(WhirlyGlobeViewControllerAnimationState * __nonnull)viewState;

/** @brief Find a selectable object at or near the given location.
    @details This runs immediately and looks for a Maply object at the given location.  It differs from the WhirlyGlobeViewControllerDelegate in that it doesn't require user interaction.
    @param screenPt The location on screen where we're looking for an object.
    @return Returns a Maply object such as MaplyScreenLabel or MaplyShape or nil if it failed to find anything.
  */
- (id __nullable)findObjectAtLocation:(CGPoint)screenPt;

/** @brief An old style method to add a spherical earth layer.
    @details Image sets and this layer have been superceeded by MaplyQuadImageTilesLayer.  This is here for backwards compatibility.
  */
- (WGViewControllerLayer * __nonnull)addSphericalEarthLayerWithImageSet:(NSString * __nonnull)name;

/** @brief Return a location on the screen for a given geographic coordinate or false if it's not on the screen.
    @param geoCoord Point on the earth in lat/lon radians you want a screen position for.
    @param screenPt Location on the screen.
    @return True if the geo coord was on the screen, false otherwise.
  */
- (bool)screenPointFromGeo:(MaplyCoordinate)geoCoord screenPt:(CGPoint * __nonnull)screenPt;

/** @brief Calculate a geo coordinate from a point on the screen.
    @param screenPt Location on the screen.
    @param geoCoord Point on the earth in lat/lon radians.
    @return True if the point was on the globe, false otherwise.
  */
- (bool)geoPointFromScreen:(CGPoint)screenPt geoCoord:(MaplyCoordinate * __nonnull)geoCoord;

/** @brief Calculate a geocentric coordinate from a point on the screen.
    @param screenPt Location on the screen.
    @param retCoords An array of 3 doubles.  The geocentric coordinate will be returned here.
    @return True if the point was on the globe, false otherwise.
 */
- (bool)geocPointFromScreen:(CGPoint)screenPt geocCoord:(double * __nonnull)retCoords;

/** @brief Find a height that shows the given bounding box.
    @details This method will search for a height that shows the given bounding box within the view.  The search is inefficient, so don't call this a lot.
    @param bbox The bounding box (in radians) we're trying to view.
    @param pos The position the viewer will be at.
 */
- (float)findHeightToViewBounds:(MaplyBoundingBox * __nonnull)bbox pos:(MaplyCoordinate)pos;

/**
 @brief Return the extents of the current view.
 @details When we're dealing with a globe the corners could be outside of the globe, in this case false is returned.
 @param bbox. The bbox will be returned here.
 @return Returns true if exists a bounding bbox for the current view, otherwise returns false
 */
- (bool) getCurrentExtents:(MaplyBoundingBox * __nonnull)bbox;

/**
  @brief From the current view figure out a usable geo bounding box.
  @details This is similar to the WhirlyGlobeViewControllerDelegate methods and getCurrentExtents except that it goes a little deeper.  It starts with the four corners of the screen and then tries to take tilt and orientation into account.  Ideally it produces a bounding box that covers everything the user is looking at as opposed to where the four corners are.
 @param bboxes The bounding boxes to fill in.  Pass in two.
 @param visualBoxes If set, we'll build bounding boxes you can display.  If not set, we'll build a single bounding box usable for math.
  */
- (int)getUsableGeoBoundsForView:(MaplyBoundingBox * __nonnull)bboxes visual:(bool)visualBoxes;

@end
