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
/// @endcond

namespace WhirlyKit
{
    class SceneRendererES;
    class ViewStateFactory;
    class ViewState;
}

/** The layer view watcher is a base class.  We subclass it for specific
    view types, such as globe and map.  Each of the subclasses determines
    the criteria for watcher updates.
 */
@interface WhirlyKitLayerViewWatcher : NSObject<WhirlyKitViewWatcherDelegate>

/// The sublcass of WhirlyKit::ViewState we'll use
@property (nonatomic) WhirlyKit::ViewStateFactory *viewStateFactory;

/// Initialize with a view and layer thread
- (id)initWithView:(WhirlyKit::View *)view thread:(WhirlyKitLayerThread *)layerThread;

/// Add the given target/selector combo as a watcher.
/// Will get called at most the given frequency.
- (void)addWatcherTarget:(id)target selector:(SEL)selector minTime:(NSTimeInterval)minTime minDist:(float)minDist maxLagTime:(NSTimeInterval)maxLagTime;

/// Remove the given target/selector combo
- (void)removeWatcherTarget:(id)target selector:(SEL)selector;

@end

namespace WhirlyKit
{
    
/// Generate a view state of the appropriate type
class ViewStateFactory
{
public:
    ViewStateFactory();
    virtual ~ViewStateFactory();
    virtual ViewState *makeViewState(WhirlyKit::View *,SceneRendererES *renderer) = 0;
};

/** Representation of the view state.  This is the base
 class for specific view state info for the various view
 types.
 */
class ViewState
{
public:
    ViewState(WhirlyKit::View *view,SceneRendererES *renderer);
    virtual ~ViewState();

    /// Calculate the viewing frustum (which is also the image plane)
    /// Need the framebuffer size in pixels as input
    /// This will cache the values in the view state for later use
    void calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight);
    
    /// From a screen point calculate the corresponding point in 3-space
    Point3d pointUnproject(Point2d screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// From a world location (3D), figure out the projection to the screen
    ///  Returns a point within the frame
    CGPoint pointOnScreenFromDisplay(const WhirlyKit::Point3d &worldLoc,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize);
    
    /// Compare this view state to the other one.  Returns true if they're identical.
    bool isSameAs(WhirlyKit::ViewState *other);
    
    /// Dump out info about the view state
    void log();
    
    Eigen::Matrix4d modelMatrix,viewMatrix,fullMatrix,projMatrix,fullNormalMatrix;
    Eigen::Matrix4d invModelMatrix,invViewMatrix,invFullMatrix,invProjMatrix;
    double fieldOfView;
    double imagePlaneSize;
    double nearPlane;
    double farPlane;
    WhirlyKit::Point3d eyeVec;
    WhirlyKit::Point3d eyeVecModel;
    WhirlyKit::Point2d ll,ur;
    double near,far;
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    
    /// Calculate where the eye is in model coordinates
    Point3d eyePos;
};

}
