/*
 *  WhirlyMapView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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

#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"

// 3D map view
@interface WhirlyMapView : WhirlyKitView
{
    // Viewer location
    WhirlyGlobe::Point3f loc;
}

@property(nonatomic,assign) WhirlyGlobe::Point3f loc;

/// Initialize with the coordinate system we'll use
- (id)initWithCoordSystem:(WhirlyKit::CoordSystem *)coordSys;

/// Cancel any outstanding animation
- (void)cancelAnimation;

/// Renderer calls this every update
- (void)animate;

/// Calculate the Z buffer resolution
- (float)calcZbufferRes;

/// Generate the model view matrix for use by OpenGL.
- (Eigen::Affine3f)calcModelMatrix;

/// Height above the plane
- (float)heightAboveSurface;

/** Given a location on the screen and the screen size, figure out where we touched
    the plane.  Returns true if we hit and where.
    Returns false if we didn't, which can only happened if we're turned away.
 */
- (bool)pointOnPlaneFromScreen:(CGPoint)pt transform:(const Eigen::Affine3f *)transform frameSize:(const WhirlyGlobe::Point2f &)frameSize hit:(WhirlyGlobe::Point3f *)hit;

/** From a world location in 3D, figure the projection to the screen.
    Returns a point within the frame.
  */
- (CGPoint)pointOnScreenFromPlane:(const WhirlyGlobe::Point2f &)worldLoc transform:(const Eigen::Affine3f *)transform frameSize:(const WhirlyGlobe::Point2f &)frameSize;

@end
