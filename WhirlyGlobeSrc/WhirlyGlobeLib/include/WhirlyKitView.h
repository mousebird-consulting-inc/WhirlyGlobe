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
#import <set>
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "WhirlyKitView.h"
#import "CoordSystem.h"

// Sent when a WhirlyKitView animation starts
#define kWKViewAnimationStarted @"WKViewAnimationStarted"
// Sent when a WhirlyKitView animation is cancelled
#define kWKViewAnimationEnded @"WKViewAnimationEnded"

/// @cond
@class WhirlyKitView;
/// @endcond

/// Watcher Callback
@protocol WhirlyKitViewWatcherDelegate
/// Called when the view changes position
- (void)viewUpdated:(WhirlyKitView *)view;
@end

typedef std::set<NSObject<WhirlyKitViewWatcherDelegate> * __weak> WhirlyKitViewWatcherDelegateSet;

/** Whirly Kit View is the base class for the views
    used in WhirlyGlobe and Maply.  It contains the general purpose
    methods and parameters related to the model and view matrices used for display.
 */
@interface WhirlyKitView : NSObject

@property (nonatomic,assign) double fieldOfView,imagePlaneSize,nearPlane,farPlane;
/// The last time the position was changed
@property (nonatomic,assign) CFTimeInterval lastChangedTime;
/// Display adapter and coordinate system we're working in
@property (nonatomic,assign) WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
/// If set, we'll scale the near and far clipping planes as we get closer
@property (nonatomic,assign) bool continuousZoom;

/// Calculate the viewing frustum (which is also the image plane)
/// Need the framebuffer size in pixels as input
- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(WhirlyKit::Point2d &)ll ur:(WhirlyKit::Point2d &)ur near:(double &)near far:(double &)far;

/// Cancel any outstanding animation.  Filled in by subclass.
- (void)cancelAnimation;

/// Renderer calls this every update.  Filled in by subclass.
- (void)animate;

/// Calculate the Z buffer resolution.  Filled in by subclass.
- (float)calcZbufferRes;

/// Generate the model view matrix for use by OpenGL.  Filled in by subclass.
- (Eigen::Matrix4d)calcModelMatrix;

/// An optional matrix used to calculate where we're looking
///  as a second step from where we are
- (Eigen::Matrix4d)calcViewMatrix;

/// Return the combination of model and view matrix
- (Eigen::Matrix4d)calcFullMatrix;

/// Calculate the projection matrix, given the size of the frame buffer
- (Eigen::Matrix4d)calcProjectionMatrix:(WhirlyKit::Point2f)frameBufferSize margin:(float)margin;

/// Return the nominal height above the surface of the data
- (double)heightAboveSurface;

/// From a screen point calculate the corresponding point in 3-space
- (WhirlyKit::Point3d)pointUnproject:(WhirlyKit::Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip;

/// Return the ray running from eye through the given screen point in display space
//- (WhirlyKit::Ray3f)displaySpaceRayFromScreenPt:(WhirlyKit::Point2f)screenPt width:(float)frameWidth height:(float)frameHeight;

/// Add a watcher delegate.  Call this on the main thread.
- (void)addWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate;

/// Remove the given watcher delegate.  Call this on the main thread
- (void)removeWatcherDelegate:(NSObject<WhirlyKitViewWatcherDelegate> *)delegate;

/// Used by subclasses to notify all the watchers of updates
- (void)runViewUpdates;

@end
