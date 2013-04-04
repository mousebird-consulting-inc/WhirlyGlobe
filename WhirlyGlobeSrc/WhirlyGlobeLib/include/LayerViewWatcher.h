/*
 *  LayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/12.
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
#import "WhirlyKitView.h"

/// @cond
@class WhirlyKitLayerThread;
@class WhirlyKitViewState;
/// @endcond

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject<WhirlyKitViewWatcherDelegate>
{
    /// Layer we're attached to
    WhirlyKitLayerThread * __weak layerThread;
    /// The view we're following for upates
    WhirlyKitView * __weak view;
    /// Watchers we'll call back for updates
    NSMutableArray *watchers;
    
    /// When the last update was run
    NSTimeInterval lastUpdate;

    /// You should know the type here.  A globe or a map view state.
    WhirlyKitViewState *lastViewState;
    
    /// The sublcass of WhirlyKitViewState we'll use
    Class viewStateClass;
}

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKitView *)view thread:(WhirlyKitLayerThread *)layerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

@end

/** Representation of the view state.  This is the base
 class for specific view state info for the various view
 types.
 */
@interface WhirlyKitViewState : NSObject
{
@public
    Eigen::Matrix4f modelMatrix,viewMatrix,fullMatrix;
	float fieldOfView;
	float imagePlaneSize;
	float nearPlane;
	float farPlane;
    WhirlyKit::Point3f eyeVec;
    WhirlyKit::Point3f eyeVecModel;
    WhirlyKit::Point2f ll,ur;
    float near,far;
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
}

/// Called by the subclasses
- (id)initWithView:(WhirlyKitView *)view;

/// Calculate where the eye is in model coordinates
- (Eigen::Vector3f)eyePos;

/// Calculate the viewing frustum (which is also the image plane)
/// Need the framebuffer size in pixels as input
/// This will cache the values in the view state for later use
- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight;

/// From a screen point calculate the corresponding point in 3-space
- (WhirlyKit::Point3f)pointUnproject:(WhirlyKit::Point2f)screenPt width:(unsigned int)frameWidth height:(unsigned int)frameHeight clip:(bool)clip;

/// From a world location (3D), figure out the projection to the screen
///  Returns a point within the frame
- (CGPoint)pointOnScreenFromDisplay:(const WhirlyKit::Point3f &)worldLoc transform:(const Eigen::Matrix4f *)transform frameSize:(const WhirlyKit::Point2f &)frameSize;

/// Compare this view state to the other one.  Returns true if they're identical.
- (bool)isSameAs:(WhirlyKitViewState *)other;

@end
