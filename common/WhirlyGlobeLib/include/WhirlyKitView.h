/*  WhirlyKitView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2012-2022 mousebird consulting
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
 */

#import "Platform.h"
#import "WhirlyTypes.h"
#import "WhirlyVector.h"
#import "CoordSystem.h"

#import <memory>
#import <mutex>
#import <set>

namespace WhirlyKit
{

class SceneRenderer;
class View;
class ViewState;
typedef std::shared_ptr<ViewState> ViewStateRef;

/// Watcher Callback
struct ViewWatcher
{
    virtual ~ViewWatcher() = default;

    /// Called when the view changes position
    virtual void viewUpdated(View *view) = 0;
};
using ViewWatcherRef = std::shared_ptr<ViewWatcher>;
using ViewWatcherWeakRef = std::weak_ptr<ViewWatcher>;

struct ViewAnimationDelegate
{
    virtual bool isUserMotion() const = 0;

    /// Called every tick to update the view position
    virtual void updateView(WhirlyKit::View *) = 0;
};

/** Whirly Kit View is the base class for the views
    used in WhirlyGlobe and Maply.  It contains the general purpose
    methods and parameters related to the model and view matrices used for display.
 */
class View : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    View();
    View(const View &);
    virtual ~View() = default;

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
    virtual Eigen::Matrix4d calcModelMatrix() const;
    
    /// An optional matrix used to calculate where we're looking
    ///  as a second step from where we are
    virtual Eigen::Matrix4d calcViewMatrix() const;
    
    /// Return the combination of model and view matrix
    virtual Eigen::Matrix4d calcFullMatrix() const;
    
    /// Calculate the projection matrix, given the size of the frame buffer
    virtual Eigen::Matrix4d calcProjectionMatrix(Point2f frameBufferSize,float margin) const;
    
    /// Return the nominal height above the surface of the data
    virtual double heightAboveSurface() const;

    /// Minimum valid height above plane
    virtual double minHeightAboveSurface() const { return 0; }

    /// Maximum valid height above plane
    virtual double maxHeightAboveSurface() const { return 0; }

    /// Calculate where the eye is in model coordinates
    virtual Eigen::Vector3d eyePos() const;
    
    /// Put together one or more offset matrices to express wrapping
    virtual void getOffsetMatrices(Matrix4dVector &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferX) const;

    /// If we're wrapping, we may need a non-wrapped coordinate
    virtual WhirlyKit::Point2f unwrapCoordinate(const WhirlyKit::Point2f &pt) const;
    
    /// From a screen point calculate the corresponding point in 3-space
    virtual WhirlyKit::Point3d pointUnproject(Point2f screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// Return the ray running from eye through the given screen point in display space
    //- (WhirlyKit::Ray3f)displaySpaceRayFromScreenPt:(WhirlyKit::Point2f)screenPt width:(float)frameWidth height:(float)frameHeight;
    
    /// Calculate a map scale
    double currentMapScale(const WhirlyKit::Point2f &frameSize) const;

    /// Calculate the height for a given scale.  Probably for minVis/maxVis
    double heightForMapScale(double scale,const WhirlyKit::Point2f &frameSize) const;

    /// Calculate map zoom
    double currentMapZoom(const WhirlyKit::Point2f &frameSize,double latitude) const;

    /// Return the screen size in display coordinates
    virtual WhirlyKit::Point2d screenSizeInDisplayCoords(const WhirlyKit::Point2f &frameSize);
    
    /// Generate a ViewState corresponding to this view
    virtual ViewStateRef makeViewState(SceneRenderer *renderer) = 0;

    /// Add a watcher delegate.  Call this on the main thread.
    virtual void addWatcher(const ViewWatcherRef &);
    
    /// Remove the given watcher delegate.  Call this on the main thread.
    virtual void removeWatcher(const ViewWatcherRef &);
    
    /// Used by subclasses to notify all the watchers of updates
    virtual void runViewUpdates();

    const CoordSystemDisplayAdapter *getCoordAdapter() const { return coordAdapter; }

    void setFieldOfView(float fov);
    float getFieldOfView() const { return fieldOfView; }

    void setNearPlane(float near);
    float getNearPlane() const { return nearPlane; }

    void setFarPlane(float far);
    float getFarPlane() const { return farPlane; }

    void setPlanes(float near, float far);

    void setContinuousZoom(bool cz) { continuousZoom = cz; }
    bool getContinuousZoom() const { return continuousZoom; }

    TimeInterval getLastChangedTime() const { return lastChangedTime; }

    /// Indicates that the view is currently being panned
    bool getIsPanning() const { return isPanning; }
    void setIsPanning(bool b) { isPanning = b; }

    /// Indicates that the view is currently being zoomed
    bool getIsZooming() const { return isZooming; }
    void setIsZooming(bool b) { isZooming = b; }

    /// Indicates that the view is currently being rotated
    bool getIsRotating() const { return isRotating; }
    void setIsRotating(bool b) { isRotating = b; }

    /// Indicates that the view is currently being tilted
    bool getIsTilting() const { return isTilting; }
    void setIsTilting(bool b) { isTilting = b; }

    /// Indicates that the view is currently animating
    bool getIsAnimating() const { return isAnimating; }
    void setIsAnimating(bool b) { isAnimating = b; }

    /// Indicates that the pan/zoom/animation is user-initiated
    bool getUserMotion() const { return userMotion; }
    void setUserMotion(bool b) { userMotion = b; }

    /// Indicates that the view center was changed directly using the toolkit (other than setViewState) since the last frame
    bool getHasMoved() const { return hasMoved; }
    void setHasMoved(bool b) { hasMoved = b; }

    /// Indicates that the view height was changed directly using the toolkit (other than setViewState) since the last frame
    bool getHasZoomed() const { return hasZoomed; }
    void setHasZoomed(bool b) { hasZoomed = b; }

    /// Indicates that the view heading was changed directly using the toolkit (other than setViewState) since the last frame
    bool getHasRotated() const { return hasRotated; }
    void setHasRotated(bool b) { hasRotated = b; }
    
    /// Indicates that the view heading was changed directly using the toolkit (other than setViewState) since the last frame
    bool getHasTilted() const { return hasTilted; }
    void setHasTilted(bool b) { hasRotated = b; }

protected:
    friend class ViewState;
    void updateParams();

    double getImagePlaneSize() const { return imagePlaneSize; }

protected:
    double fieldOfView = M_PI / 3;  // 60 degree field of view
    double imagePlaneSize = 0.0;
    double nearPlane = 0.001;
    double farPlane = 10.0;
    
    void removeWatcherLocked(const ViewWatcherRef &);
    
protected:
    Point2d centerOffset = { 0, 0 };
    Matrix4dVector offsetMatrices;
    /// The last time the position was changed
    TimeInterval lastChangedTime = 0.0;
    /// Display adapter and coordinate system we're working in
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter = nullptr;
    /// If set, we'll scale the near and far clipping planes as we get closer
    bool continuousZoom = false;
    
    bool isPanning = false;
    bool isZooming = false;
    bool isAnimating = false;
    bool isRotating = false;
    bool isTilting = false;
    bool userMotion = false;
    bool hasMoved = false;
    bool hasZoomed = false;
    bool hasRotated = false;
    bool hasTilted = false;

    /// Called when positions are updated
    // Can't use a set or unordered_set for things that can change, but there
    // should never be huge numbers of watchers, or rapid adds/removes.
    std::vector<ViewWatcherWeakRef> watchers;
    std::mutex watcherLock;
};
    
typedef std::shared_ptr<View> ViewRef;

/** Representation of the view state.  This is the base
 class for specific view state info for the various view
 types.
 */
class ViewState
{
public:
    ViewState() : near(0), far(0) { }
    ViewState(View *view,WhirlyKit::SceneRenderer *renderer);
    virtual ~ViewState() = default;
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Calculate the viewing frustum (which is also the image plane)
    /// Need the framebuffer size in pixels as input
    /// This will cache the values in the view state for later use
    void calcFrustumWidth(unsigned int frameWidth,unsigned int frameHeight);
    
    /// From a screen point calculate the corresponding point in 3-space
    Point3d pointUnproject(Point2d screenPt,unsigned int frameWidth,unsigned int frameHeight,bool clip);
    
    /// From a world location (3D), figure out the projection to the screen
    ///  Returns a point within the frame
    Point2f pointOnScreenFromDisplay(const Point3d &worldLoc,const Eigen::Matrix4d *transform,const Point2f &frameSize);
    
    /// Compare this view state to the other one.  Returns true if they're identical.
    bool isSameAs(const ViewState *other) const;
    
    /// Return true if the view state has been set to something
    bool isValid() const { return near != far; }
    
    /// Dump out info about the view state
    void log();
    
    Eigen::Matrix4d modelMatrix,projMatrix;
    Matrix4dVector viewMatrices,invViewMatrices,fullMatrices,fullNormalMatrices,invFullMatrices;
    Eigen::Matrix4d invModelMatrix,invProjMatrix;
    double fieldOfView;
    double imagePlaneSize;
    double nearPlane;
    double farPlane;
    Point3d eyeVec;
    Point3d eyeVecModel;
    Point2d ll,ur;
    double near,far;
    CoordSystemDisplayAdapter *coordAdapter;
    
    /// Calculate where the eye is in model coordinates
    Point3d eyePos;
};

}
