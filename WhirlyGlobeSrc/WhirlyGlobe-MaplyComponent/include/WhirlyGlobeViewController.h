/*
 *  GlobeViewController.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import <UIKit/UIKit.h>
#import "MaplyBaseViewController.h"

@class WGViewControllerLayer;
@class WhirlyGlobeViewController;

/** Fill in this protocol to get called when the user taps on or near an object for
    selection. 
 */
@protocol WhirlyGlobeViewControllerDelegate <NSObject>

@optional
/// Called when the user taps on or near an object.
/// You're given the object you passed in originally, such as a WGScreenMarker.
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didSelect:(NSObject *)selectedObj;

/// Called when the user taps outside the globe.
/// Passes in the location on the view.
- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC;

/// The user tapped at the given location.
/// This won't be called if they tapped and selected, just if they tapped.
- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(WGCoordinate)coord;

/// This is called when a given layer loads.
/// Not all layers support this callback.  Those that load immediately (which is most of them)
///  won't trigger this.
- (void)globeViewController:(WhirlyGlobeViewController *)viewC layerDidLoad:(WGViewControllerLayer *)layer;

@end

/** This is the main object in the WhirlyGlobe Component.  You fire up one
    of these, add its view to your view hierarchy and start tossing in data.
    At the very least you'll want a base image layer and then you can put
    markers, labels, and vectors on top of that.
 */
@interface WhirlyGlobeViewController : MaplyBaseViewController
{
    NSObject<WhirlyGlobeViewControllerDelegate> * __weak delegate;
}

/// Set this to keep the north pole facing upward when moving around.
/// Off by default.
@property(nonatomic,assign) bool keepNorthUp;

/// Set this to trun on/off the pinch (zoom) gesture recognizer
/// On by default
@property(nonatomic,assign) bool pinchGesture;

/// Set this to turn on or off the rotation gesture recognizer.
/// On by default.
@property(nonatomic,assign) bool rotateGesture;

/// Set this to get callbacks for various events.
@property(nonatomic,weak) NSObject<WhirlyGlobeViewControllerDelegate> *delegate;

/// Get/set the current height above terrain.
/// The radius of the earth is 1.0.  Height above terrain is relative to that.
@property (nonatomic,assign) float height;

/// This is is radians.  0 is looking straight down (the default)
///  PI/2 is looking toward the horizon.
@property(nonatomic,assign) float tilt;

/// Return the min and max heights above the globe for zooming
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight;

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight;

/// Set this to something other than zero and it will autorotate
///  after that interval the given number of degrees per second
- (void)setAutoRotateInterval:(float)autoRotateInterval degrees:(float)autoRotateDegrees;

/// Animate to the given position over the given amount of time
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong;

/// Animate the given position to the given screen location over time.
/// If this isn't physically possible, it will just do nothing
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong;

/// Set the view to the given position immediately
- (void)setPosition:(MaplyCoordinate)newPos;

/// Set position and height at the same time
- (void)setPosition:(MaplyCoordinate)newPos height:(float)height;

/// Get the current position and height
- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height;

/// Set heading about current position
- (void) setHeading:(CGFloat) rotationHeading position:(WGCoordinate)newPos;

/// Add a spherical earth layer with the given set of base images
- (WGViewControllerLayer *)addSphericalEarthLayerWithImageSet:(NSString *)name;

/// This utility routine returns the on screen location for a coordinate in lat/lon
- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord;

/// Set the direction the sun is coming from.  This makes lighting independent of the viewing
///  angle as well.
- (void)setSunDirection:(MaplyCoordinate3d)sunDir;

/// Turn off the sun direction and go back to view dependent lighting (the default).
- (void)clearSunDirection;

@end
