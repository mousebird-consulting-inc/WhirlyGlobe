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

<<<<<<< HEAD
#import <set>
#import "WhirlyTypes.h"
#import "CoordSystem.h"

// Sent when a WhirlyKit::View animation starts
#define kWKViewAnimationStarted @"WKViewAnimationStarted"
// Sent when a WhirlyKit::View animation is cancelled
#define kWKViewAnimationEnded @"WKViewAnimationEnded"


=======
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
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

namespace WhirlyKit
{
    
class View;

/// Watcher Callback
class ViewWatcher
{
public:
    virtual ~ViewWatcher() { }
    /// Called when the view changes position
    virtual void viewUpdated(View *view) = 0;
};

typedef std::set<ViewWatcher *> ViewWatcherSet;

<<<<<<< HEAD
/** Whirly Kit View is the base class for the views
    used in WhirlyGlobe and Maply.  It contains the general purpose
    methods and parameters related to the model and view matrices used for display.
 */
class View : public DelayedDeletable
{
public:
    View();
    View(const View &);
    virtual ~View();

    /// Calculate the viewing frustum (which is also the image plane)
    /// Need the framebuffer size in pixels as input
    virtual void calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight,Point2d &ll,Point2d &ur,double &near,double &far);
    
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
    virtual Eigen::Matrix4d calcProjectionMatrix(Point2f frameBufferSize,float margin);
    
    /// Return the nominal height above the surface of the data
    virtual double heightAboveSurface();
    
    /// Put together one or more offset matrices to express wrapping
    virtual void getOffsetMatrices(std::vector<Eigen::Matrix4d> &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize);

    /// If we're wrapping, we may need a non-wrapped coordinate
    WhirlyKit::Point2f unwrapCoordinate(const WhirlyKit::Point2f &pt);
    
    /// From a screen point calculate the corresponding point in 3-space
    virtual WhirlyKit::Point3d pointUnproject(Point2f screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// Return the ray running from eye through the given screen point in display space
    //- (WhirlyKit::Ray3f)displaySpaceRayFromScreenPt:(WhirlyKit::Point2f)screenPt width:(float)frameWidth height:(float)frameHeight;
    
    /// Calculate a map scale
    double currentMapScale(const WhirlyKit::Point2f &frameSize);

    /// Calculate the height for a given scale.  Probably for minVis/maxVis
    double heightForMapScale(double scale,WhirlyKit::Point2f &frameSize);

    /// Calculate map zoom
    double currentMapZoom(const WhirlyKit::Point2f &frameSize,double latitude);
    
    /// Add a watcher delegate.  Call this on the main thread.
    virtual void addWatcher(ViewWatcher *delegate);
    
    /// Remove the given watcher delegate.  Call this on the main thread
    virtual void removeWatcher(ViewWatcher *delegate);
    
    /// Used by subclasses to notify all the watchers of updates
    virtual void runViewUpdates();
    
    double fieldOfView,imagePlaneSize,nearPlane,farPlane;
    std::vector<Eigen::Matrix4d> offsetMatrices;
    /// The last time the position was changed
    TimeInterval lastChangedTime;
    /// Display adapter and coordinate system we're working in
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    /// If set, we'll scale the near and far clipping planes as we get closer
    bool continuousZoom;
    
    /// Called when positions are updated
    ViewWatcherSet watchers;
};

}
=======
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
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
