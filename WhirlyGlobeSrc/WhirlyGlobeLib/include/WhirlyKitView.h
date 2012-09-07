/*
 *  WhirlyKitView.h
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

#import <UIKit/UIKit.h>
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"
#import "CoordSystem.h"

/** Whirly Kit View is the base class for the views
    used in WhirlyGlobe and Maply.  It contains the general purpose
    methods and parameters related to the model and view matrices used for display.
 */
@interface WhirlyKitView : NSObject
{
	float fieldOfView;
	float imagePlaneSize;
	float nearPlane;
	float farPlane;
	
    /// The last time the position was changed
    CFTimeInterval lastChangedTime;
    
    /// Coordinate system we're working in for the view
    WhirlyKit::CoordSystem *coordSystem;
    
    /// If set, we'll scale the near and far clipping planes as we get closer
    bool continuousZoom;
}

@property (nonatomic,assign) float fieldOfView,imagePlaneSize,nearPlane,farPlane;
@property (nonatomic,readonly) CFTimeInterval lastChangedTime;
@property (nonatomic,readonly) WhirlyKit::CoordSystem *coordSystem;
@property (nonatomic,assign) bool continuousZoom;

/// Calculate the viewing frustum (which is also the image plane)
/// Need the framebuffer size in pixels as input
- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(WhirlyKit::Point2f &)ll ur:(WhirlyKit::Point2f &)ur near:(float &)near far:(float &)far;

/// Cancel any outstanding animation.  Filled in by subclass.
- (void)cancelAnimation;

/// Renderer calls this every update.  Filled in by subclass.
- (void)animate;

/// Calculate the Z buffer resolution.  Filled in by subclass.
- (float)calcZbufferRes;

/// Generate the model view matrix for use by OpenGL.  Filled in by subclass.
- (Eigen::Matrix4f)calcModelMatrix;

/// An optional matrix used to calculate where we're looking
///  as a second step from where we are
- (Eigen::Matrix4f)calcViewMatrix;

/// Return the nominal height above the surface of the data
- (float)heightAboveSurface;

/// From a screen point calculate the corresponding point in 3-space
- (WhirlyKit::Point3f)pointUnproject:(WhirlyKit::Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip;

@end

