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

// Sent when a WhirlyKit::View animation starts
#define kWKViewAnimationStarted @"WKViewAnimationStarted"
// Sent when a WhirlyKit::View animation is cancelled
#define kWKViewAnimationEnded @"WKViewAnimationEnded"

namespace WhirlyKit
{
    class View;
}

/// Watcher Callback
@protocol WhirlyKitViewWatcherDelegate
/// Called when the view changes position
- (void)viewUpdated:(WhirlyKit::View *)view;
@end

typedef std::set<NSObject<WhirlyKitViewWatcherDelegate> * __weak> WhirlyKitViewWatcherDelegateSet;

namespace WhirlyKit
{

/** Whirly Kit View is the base class for the views
    used in WhirlyGlobe and Maply.  It contains the general purpose
    methods and parameters related to the model and view matrices used for display.
 */
class View : DelayedDeletable
{
public:
    View();
    virtual ~View();

    /// Calculate the viewing frustum (which is also the image plane)
    /// Need the framebuffer size in pixels as input
    virtual void calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight,WhirlyKit::Point2d &ll,WhirlyKit::Point2d &ur,double &near,double &far);
    
    /// Cancel any outstanding animation.  Filled in by subclass.
    virtual void cancelAnimation();
    
    /// Renderer calls this every update.  Filled in by subclass.
    virtual void animate();
    
    /// Calculate the Z buffer resolution.  Filled in by subclass.
    virtual float calcZbufferRes();
    
    /// Generate the model view matrix for use by OpenGL.  Filled in by subclass.
    virtual Eigen::Matrix4d calcModelMatrix();
    
    /// An optional matrix used to calculate where we're looking
    ///  as a second step from where we are
    virtual Eigen::Matrix4d calcViewMatrix();
    
    /// Return the combination of model and view matrix
    virtual Eigen::Matrix4d calcFullMatrix();
    
    /// Calculate the projection matrix, given the size of the frame buffer
    virtual Eigen::Matrix4d calcProjectionMatrix(WhirlyKit::Point2f frameBufferSize,float margin);
    
    /// Return the nominal height above the surface of the data
    virtual double heightAboveSurface();
    
    /// From a screen point calculate the corresponding point in 3-space
    virtual WhirlyKit::Point3d pointUnproject(WhirlyKit::Point2f screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// Return the ray running from eye through the given screen point in display space
    //- (WhirlyKit::Ray3f)displaySpaceRayFromScreenPt:(WhirlyKit::Point2f)screenPt width:(float)frameWidth height:(float)frameHeight;
    
    /// Add a watcher delegate.  Call this on the main thread.
    virtual void addWatcherDelegate(NSObject<WhirlyKitViewWatcherDelegate> * delegate);
    
    /// Remove the given watcher delegate.  Call this on the main thread
    virtual void removeWatcherDelegate(NSObject<WhirlyKitViewWatcherDelegate> *delegate);
    
    /// Used by subclasses to notify all the watchers of updates
    virtual void runViewUpdates();
    
    double fieldOfView,imagePlaneSize,nearPlane,farPlane;
    /// The last time the position was changed
    CFTimeInterval lastChangedTime;
    /// Display adapter and coordinate system we're working in
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    /// If set, we'll scale the near and far clipping planes as we get closer
    bool continuousZoom;
    
    /// Called when positions are updated
    WhirlyKitViewWatcherDelegateSet watchDelegates;
};

}
