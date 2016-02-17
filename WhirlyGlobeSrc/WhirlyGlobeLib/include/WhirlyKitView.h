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

#import <set>
#import "WhirlyTypes.h"
#import "CoordSystem.h"

// Sent when a WhirlyKit::View animation starts
#define kWKViewAnimationStarted @"WKViewAnimationStarted"
// Sent when a WhirlyKit::View animation is cancelled
#define kWKViewAnimationEnded @"WKViewAnimationEnded"



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
    
    /// Return the screen size in display coordinates
    virtual WhirlyKit::Point2d screenSizeInDisplayCoords(WhirlyKit::Point2f &frameSize);

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
