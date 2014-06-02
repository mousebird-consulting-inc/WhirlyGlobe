/*
 *  MaplyView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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

#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"

/// @cond
@class MaplyView;
/// @endcond

/// Animation callback
@protocol MaplyAnimationDelegate
- (void)updateView:(MaplyView *)mapView;
@end

/** Parameters associated with viewing the map.
    Modify the location to change the current view location.
    Set the delegate to smoothly change location over time.
 */
@interface MaplyView : WhirlyKitView

/// Viewer location
@property(nonatomic,readonly) WhirlyKit::Point3d &loc;
/// Viewer rotation angle
@property(nonatomic,readonly) double &rotAngle;
/// Used to update position based on time (or whatever)
@property(nonatomic,weak) NSObject<MaplyAnimationDelegate> *delegate;

/// Initialize with the coordinate system we'll use
- (id)initWithCoordAdapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;

/// Cancel any outstanding animation
- (void)cancelAnimation;

/// Renderer calls this every update
- (void)animate;

/// Calculate the Z buffer resolution
- (float)calcZbufferRes;

/// Generate the model view matrix for use by OpenGL.
- (Eigen::Matrix4d)calcModelMatrix;

/// Height above the plane
- (double)heightAboveSurface;

/// Minimum valid height above plane
- (double)minHeightAboveSurface;

/// Maximum valid height above plane
- (double)maxHeightAboveSurface;

/// Set the location, but we may or may not run updates
- (void)setLoc:(WhirlyKit::Point3d &)loc runUpdates:(bool)runUpdates;

/** Given a location on the screen and the screen size, figure out where we touched
    the plane.  Returns true if we hit and where.
    Returns false if we didn't, which can only happened if we're turned away.
 */
- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Matrix4d *)transform frameSize:(const WhirlyKit::Point2f &)frameSize hit:(WhirlyKit::Point3d *)hit clip:(bool)clip;

/** From a world location in 3D, figure the projection to the screen.
    Returns a point within the frame.
  */
- (CGPoint)pointOnScreenFromPlane:(const WhirlyKit::Point3d &)worldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const WhirlyKit::Point2f &)frameSize;

/// Set the location we're looking from
- (void)setLoc:(WhirlyKit::Point3d)newLoc;

/// Set the rotation angle
- (void)setRotAngle:(double)newRotAngle;

@end
