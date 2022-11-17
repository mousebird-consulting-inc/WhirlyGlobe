/*
 *  MaplyView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2022 mousebird consulting
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

#import "WhirlyKitView.h"

namespace WhirlyKit
{
class View;
class SceneRenderer;
}

namespace Maply
{

class MapView;

/// Animation callback
struct MapViewAnimationDelegate : public WhirlyKit::ViewAnimationDelegate
{
    virtual bool isUserMotion() const = 0;

    /// Called every tick to update the map position
    virtual void updateView(WhirlyKit::View *) = 0;
};
typedef std::shared_ptr<MapViewAnimationDelegate> MapViewAnimationDelegateRef;

/** Parameters associated with viewing the map.
    Modify the location to change the current view location.
    Set the delegate to smoothly change location over time.
 */
class MapView : public WhirlyKit::View
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    MapView(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter);
    /// Copy constructor
    MapView(const MapView &that);
    virtual ~MapView() = default;

    /// Calculate the Z buffer resolution
    virtual float calcZbufferRes() override;

    /// Generate the model view matrix for use by OpenGL.
    virtual Eigen::Matrix4d calcModelMatrix() const override;
    
    /// Generate the view matrix for use by OpenGL
    virtual Eigen::Matrix4d calcViewMatrix() const override;
    
    /// Generate the whole matrix (minus projection)
    virtual Eigen::Matrix4d calcFullMatrix() const override;

    /// Height above the plane
    virtual double heightAboveSurface() const override;
    
    /// Put together one or more offset matrices to express wrapping
    virtual void getOffsetMatrices(WhirlyKit::Matrix4dVector &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize,float bufferX) const override;

    /// Minimum valid height above plane
    virtual double minHeightAboveSurface() const override;

    /// Maximum valid height above plane
    virtual double maxHeightAboveSurface() const override;

    /// Set the location, but we may or may not run updates
    void setLoc(const WhirlyKit::Point3d &loc,bool runUpdates);

    /// Set the location we're looking from.  Always runs updates
    void setLoc(WhirlyKit::Point3d newLoc);
    
    /// Return the current location
    WhirlyKit::Point3d getLoc() const { return loc; }
    
    /// Return the current rotation
    double getRotAngle() const { return rotAngle; }
    
    /// Return the wrap status
    bool getWrap() const { return wrap; }
    
    /// Set whether we're wrapping across world boundaries
    void setWrap(bool inWrap)  { wrap = inWrap; }

    /** Given a location on the screen and the screen size, figure out where we touched
        the plane.  Returns true if we hit and where.
        Returns false if we didn't, which can only happened if we're turned away.
     */
    bool pointOnPlaneFromScreen(WhirlyKit::Point2f pt,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize,WhirlyKit::Point3d *hit,bool clip);

    /** From a world location in 3D, figure the projection to the screen.
        Returns a point within the frame.
      */
    WhirlyKit::Point2f pointOnScreenFromPlane(const WhirlyKit::Point3d &worldLoc,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize);

    /// Set the rotation angle
    void setRotAngle(double newRotAngle,bool viewUpdates);
    
    /// Turn a wrapped coordinate (e.g. may be outside -M_PI,M_PI) back into a real world coordinate
    virtual WhirlyKit::Point2f unwrapCoordinate(const WhirlyKit::Point2f &pt) const override;
    
    /// Eye position in model coordinates
    virtual Eigen::Vector3d eyePos() const override;
    
    /// Return the coord adapter
    WhirlyKit::CoordSystemDisplayAdapter *getCoordAdapter() { return coordAdapter; }
    
    /// Make a map view state from the current globe view
    virtual WhirlyKit::ViewStateRef makeViewState(WhirlyKit::SceneRenderer *renderer) override;

    /// Set the change delegate
    virtual void setDelegate(MapViewAnimationDelegateRef delegate);
    
    /// Return the delegate for testing
    virtual MapViewAnimationDelegateRef getDelegate();
    
    /// Cancel any outstanding animation
    virtual void cancelAnimation() override;
    
    /// Renderer calls this every update
    virtual void animate() override;

protected:
    /// Viewer location
    WhirlyKit::Point3d loc;
    /// Viewer rotation angle
    double rotAngle;
    /// If set, we're wrapping across the date line
    bool wrap;
    /// Used to update position based on time
    MapViewAnimationDelegateRef delegate;

    // These are all for continuous zoom mode
    double absoluteMinHeight;
    double heightInflection;
    double defaultNearPlane;
    double absoluteMinNearPlane;
    double defaultFarPlane;
    double absoluteMinFarPlane;
};
    
typedef std::shared_ptr<MapView> MapViewRef;

/** View State related to the map view.
 */
class MapViewState : public WhirlyKit::ViewState
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    MapViewState(MapView *mapView,WhirlyKit::SceneRenderer *renderer);
    
    /// Height above globe at this view state
    double heightAboveSurface;
    
    bool pointOnPlaneFromScreen(const WhirlyKit::Point2f &pt,const Eigen::Matrix4d &transform,const WhirlyKit::Point2f &frameSize, WhirlyKit::Point3d &hit, bool clip);
};
    
typedef std::shared_ptr<MapViewState> MapViewStateRef;

}
