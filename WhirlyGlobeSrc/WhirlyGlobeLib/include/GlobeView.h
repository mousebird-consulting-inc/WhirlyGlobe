/*
 *  GlobeView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/14/11.
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
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"

/// @cond
@class WhirlyGlobeView;
/// @endcond

/// Animation callback
@protocol WhirlyGlobeAnimationDelegate
/// Called every tick to update the globe position
- (void)updateView:(WhirlyGlobeView *)globeView;
@end

/** Parameters associated with viewing the globe.
    Modify the rotation quaternion to change the current
    view location.  Set the delegate to smoothly change
    location over time.
 */
@interface WhirlyGlobeView : WhirlyKitView

/// Initialize with an existing globe view
- (id)initWithGlobeView:(WhirlyGlobeView *)inGlobeView;

/// The globe has a radius of 1.0 so 1.0 + heightAboveGlobe is the offset from the middle of the globe
@property (nonatomic,assign) double heightAboveGlobe;
/// Quaternion used for rotation from origin state
@property (nonatomic,assign) Eigen::Quaterniond rotQuat;
/// Used to update position based on time (or whatever other factor you like)
@property (nonatomic,weak) NSObject<WhirlyGlobeAnimationDelegate> *delegate;
/// The view can have a tilt.  0 is straight down.  PI/2 is looking to the horizon.
@property (nonatomic,assign) double tilt;
/// Set the far clipping plane (be careful)
- (void)setFarClippingPlane:(double)farClip;

/// Return min/max valid heights above globe
- (double)minHeightAboveGlobe;
- (double)maxHeightAboveGlobe;

/// Set the height above globe, taking constraints into account
- (void)setHeightAboveGlobe:(double)newH;

/// This version allows you to not update the watchers, if you're doing a bunch of updates at once
- (void)setHeightAboveGlobe:(double)newH updateWatchers:(bool)updateWatchers;

/// This version avoids the limit calculations (Kind of a hack)
- (void)setHeightAboveGlobeNoLimits:(double)newH updateWatchers:(bool)updateWatchers;

/// This version allows you to not update the watchers.
- (void)setRotQuat:(Eigen::Quaterniond)rotQuat updateWatchers:(bool)updateWatchers;

/// Calculate the z offset to make the earth appear where we want it
- (double)calcEarthZOffset;

/// Return where up (0,0,1) is after model rotation
- (Eigen::Vector3d)currentUp;

/// Calculate where the eye is in model coordinates
- (Eigen::Vector3d)eyePos;

/// Given a rotation, where would (0,0,1) wind up
+ (Eigen::Vector3d)prospectiveUp:(Eigen::Quaterniond &)prospectiveRot;

/** Given a location on the screen and the screen size, figure out where we touched the sphere
    Returns true if we hit and where
    Returns false if not and the closest point on the sphere
 */
- (bool)pointOnSphereFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const WhirlyKit::Point2f &)frameSize hit:(WhirlyKit::Point3d *)hit normalized:(bool)normalized;

/** From a world location (3D), figure out the projection to the screen
    Returns a point within the frame
  */
- (CGPoint)pointOnScreenFromSphere:(const WhirlyKit::Point3d &)worldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const WhirlyKit::Point2f &)frameSize;

/** Construct a rotation to the given location
    and return it.  Doesn't actually do anything yet.
 */
- (Eigen::Quaterniond) makeRotationToGeoCoord:(const WhirlyKit::GeoCoord &)worldLoc keepNorthUp:(BOOL)northUp;

- (Eigen::Quaterniond) makeRotationToGeoCoordD:(const WhirlyKit::Point2d &)worldLoc keepNorthUp:(BOOL)northUp;

// Construct a rotation to given location and heading
- (Eigen::Quaterniond) makeRotationToGeoCoord:(const WhirlyKit::GeoCoord &)worldCoord heading:(double)heading;

/// Cancel any outstanding animation
- (void)cancelAnimation;

/// Renderer calls this every update
- (void)animate;

/// Calculate the Z buffer resolution
- (float)calcZbufferRes;

/// Height above the globe
- (double)heightAboveSurface;

@end


