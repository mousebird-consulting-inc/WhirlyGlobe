/*
 *  GlobeLayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011 mousebird consulting
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
#import "GlobeView.h"
#import "LayerViewWatcher.h"

/** Representation of the view state at a given point in time.
    This is enough information to figure out what we were looking
    at.
 */
@interface WhirlyGlobeViewState : NSObject
{
@public
    float heightAboveGlobe;
	Eigen::Quaternion<float> rotQuat;    
    Eigen::Affine3f modelMatrix;
	float fieldOfView;
	float imagePlaneSize;
	float nearPlane;
	float farPlane;
}

- (id)initWithView:(WhirlyGlobeView *)globeView;

/// Calculate the viewing frustum (which is also the image plane)
/// Need the framebuffer size in pixels as input
- (void)calcFrustumWidth:(unsigned int)frameWidth height:(unsigned int)frameHeight ll:(WhirlyKit::Point2f &)ll ur:(WhirlyKit::Point2f &)ur near:(float &)near far:(float &)far;

/** From a world location (3D), figure out the projection to the screen
 Returns a point within the frame
 */
- (CGPoint)pointOnScreenFromSphere:(const WhirlyKit::Point3f &)worldLoc transform:(const Eigen::Affine3f *)transform frameSize:(const WhirlyKit::Point2f &)frameSize;

@end

/** The Globe Layer View watcher is a subclass of the layer view
    watcher that takes globe specific parameters into account.
  */
@interface WhirlyGlobeLayerViewWatcher : WhirlyKitLayerViewWatcher<WhirlyGlobeViewWatcherDelegate>
{
}

/// Initialize with the globe view to watch and the layer thread
- (id)initWithView:(WhirlyGlobeView *)view thread:(WhirlyKitLayerThread *)inLayerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

@end
