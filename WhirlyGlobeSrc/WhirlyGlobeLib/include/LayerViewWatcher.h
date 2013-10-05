/*
 *  LayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
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
#import "WhirlyKitView.h"

/// @cond
@class WhirlyKitLayerThread;
@class WhirlyKitViewState;
@class WhirlyKitSceneRendererES;
/// @endcond

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject<WhirlyKitViewWatcherDelegate>

/// The sublcass of WhirlyKitViewState we'll use
@property (nonatomic) Class viewStateClass;

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKitView *)view thread:(WhirlyKitLayerThread *)layerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime minDist:(float)minDist maxLagTime:(NSTimeInterval)maxLagTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

@end

/** Representation of the view state.  This is the base
 class for specific view state info for the various view
 types.
 */
@interface WhirlyKitViewState : NSObject

@property(nonatomic,assign) Eigen::Matrix4d &modelMatrix,&viewMatrix,&fullMatrix,&projMatrix,&fullNormalMatrix;
@property(nonatomic,assign) Eigen::Matrix4d &invModelMatrix,&invViewMatrix,&invFullMatrix,&invProjMatrix;
@property(nonatomic,assign) double fieldOfView;
@property(nonatomic,assign) double imagePlaneSize;
@property(nonatomic,assign) double nearPlane;
@property(nonatomic,assign) double farPlane;
@property(nonatomic,assign) WhirlyKit::Point3d &eyeVec;
@property(nonatomic,assign) WhirlyKit::Point3d &eyeVecModel;
@property(nonatomic,assign) WhirlyKit::Point2d &ll,&ur;
@property(nonatomic,assign) double near,far;
@property(nonatomic,assign) WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
/// Calculate where the eye is in model coordinates
@property (nonatomic,readonly) WhirlyKit::Point3d eyePos;

/// Called by the subclasses
- (id)initWithView:(WhirlyKitView *)view renderer:(WhirlyKitSceneRendererES *)renderer;

/// Calculate the viewing frustum (which is also the image plane)
/// Need the framebuffer size in pixels as input
/// This will cache the values in the view state for later use
- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight;

/// From a screen point calculate the corresponding point in 3-space
- (WhirlyKit::Point3d)pointUnproject:(WhirlyKit::Point2d)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip;

/// From a world location (3D), figure out the projection to the screen
///  Returns a point within the frame
- (CGPoint)pointOnScreenFromDisplay:(const WhirlyKit::Point3d &)worldLoc transform:(const Eigen::Matrix4d *)transform frameSize:(const WhirlyKit::Point2f &)frameSize;

/// Compare this view state to the other one.  Returns true if they're identical.
- (bool)isSameAs:(WhirlyKitViewState *)other;

/// Dump out info about the view state
- (void)log;

@end
