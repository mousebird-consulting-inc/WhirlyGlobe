/*
 *  GlobeViewController.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2013 mousebird consulting
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
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj;

/** @brief Called when the user taps on or near an object.
    @details You're given the object you passed in originally, such as a MaplyScreenMarker.
    @details This version is called preferentially if it exists.  Otherwise globeViewController:didSelect: is called if it exists.
    @param viewC The view controller where the user selected something.
    @param selectedObj The Maply object they selected.
    @param coord The location (geographic lon/lat in radians) where the user tapped.
    @param screenPt The location on screen where the user tapped.
  */
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt;

/** @brief Called when the user taps outside the globe.
  */
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC;

/** @brief Called when the user taps the globe but doesn't select anything.
    @param viewC The view controller where the user selected something.
    @param coord The location (geographic lon/lat in radians) where the user tapped.
  */
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(WGCoordinate)coord;

/** @brief This is an older method called when some layers load.
    @details Certain image layers call this method when they finish loading.  More modern layers don't, so don't rely on this.
  */
- (void)globeViewController:(WhirlyGlobeViewController *)viewC layerDidLoad:(WGViewControllerLayer *)layer;

/** @brief Called when the globe starts moving.
    @details This is called when something (probably the user) starts moving the globe.
  */
- (void)globeViewControllerDidStartMoving:(WhirlyGlobeViewController *)viewC;

/** @brief Called when the globe stops moving.
    @details This is called when the globe stops moving.  It passes in the corners of the current viewspace.
    @param viewC The globe view controller.
    @param corners An array of length 4 containing the corners of the view space (lower left, lower right, upper right, upper left).  If any of those corners does not intersect the globe (think zoomed out), its values are set to MAXFLOAT.
  */
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didStopMoving:(MaplyCoordinate *)corners;

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

/** @brief If set, we'll automatically move to wherever the user tapped.
    @details When on we'll move the current location to wherever the user tapped if they tapped the globe.  That's true for selection as well.  On by default.
  */
@property(nonatomic,assign) bool autoMoveToTap;

/** @brief Delegate for the selection and tap events.
    @details Fill in the WhirlyGlobeViewControllerDelegate protocol, assign the object here and you'll get selection and tap events.
  */
@property(nonatomic,weak) NSObject<WhirlyGlobeViewControllerDelegate> *delegate;

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
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight;

/** @brief Set the zoom limits for the globe.
    @param minHeight The closest a viewer is allowed to get to the globe surface.
    @param maxHeight The farthest away a viewer is allowed to get from the globe surface.
 */
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight;

/** @brief Set the simplified tilt mode.  We'll tilt toward the horizon as the user gets closer to the ground.
    @details This implements a simplified mode for tilting.  As the user gets closer to the ground we tilt more toward the horizon.
    @param minHeight The minimum height corresponding to minTilt.
    @param maxHeight The height at which to start interoplating tilt.
    @param minTilt The most tilt toward the horizon.  Invoked when the user is at minHeight or below.
    @param maxTilt The tilt at the maximum height and over.  The tilt will never be less than this, so typically 0.
  */
/// Set the height range over which to modify tilt.  We'll
///  vary the tilt between the given values over the given height range, if set.
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
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong;

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
- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height;

/** @brief Find a selectable object at or near the given location.
    @details This runs immediately and looks for a Maply object at the given location.  It differs from the WhirlyGlobeViewControllerDelegate in that it doesn't require user interaction.
    @param screenPt The location on screen where we're looking for an object.
    @return Returns a Maply object such as MaplyScreenLabel or MaplyShape or nil if it failed to find anything.
  */
- (id)findObjectAtLocation:(CGPoint)screenPt;

/** @brief An old style method to add a spherical earth layer.
    @details Image sets and this layer have been superceeded by MaplyQuadImageTilesLayer.  This is here for backwards compatibility.
  */
- (WGViewControllerLayer *)addSphericalEarthLayerWithImageSet:(NSString *)name;

/** @brief Return the location on screen for a given geographic (lon/lat radians) coordinate.
    @return Returns the screen point corresponding to a given geo coordinate.
  */
- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord;

/** @brief Set the direction the sun is coming from.  
    @details This turns on lighting independent viewing mode and moves one of the lights to the given sun direction. When you rotate the globe, lighting will now rotate as well.
    @details You can use this to implement plausible day time/night time lighting.
    @param sunDir The direction in the display system the sun is coming from.  Up is (0,0,1), the date line is (-1,0,0).
  */
- (void)setSunDirection:(MaplyCoordinate3d)sunDir;

/// @brief Turn off sun direction setup by setSunDirection:
- (void)clearSunDirection;

@end
