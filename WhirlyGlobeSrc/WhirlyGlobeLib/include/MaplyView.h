/*
 *  MaplyView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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

#import "WhirlyKitView.h"

//namespace Maply
//{
//class MapView;
//}

// Note: Porting
/// Animation callback
//@protocol MaplyAnimationDelegate
//- (void)updateView:(Maply::MapView *)mapView;
//@end

namespace Maply
{

/** Parameters associated with viewing the map.
    Modify the location to change the current view location.
    Set the delegate to smoothly change location over time.
 */
class MapView : public WhirlyKit::View
{
public:
    MapView(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter);
    /// Copy constructor
    MapView(const MapView &that);
    virtual ~MapView();

    /// Set the callback delegate
    // Note: Porting
//    void setDelegate(NSObject<MaplyAnimationDelegate> *delegate);

    /// Cancel any outstanding animation
    void cancelAnimation();

    /// Renderer calls this every update
    void animate();

    /// Calculate the Z buffer resolution
    float calcZbufferRes();

    /// Generate the model view matrix for use by OpenGL.
    virtual Eigen::Matrix4d calcModelMatrix();
    
    /// Generate the view matrix for use by OpenGL
    virtual Eigen::Matrix4d calcViewMatrix();
    
    /// Generate the whole matrix (minus projection)
    virtual Eigen::Matrix4d calcFullMatrix();

    /// Height above the plane
    double heightAboveSurface();
    
    /// Put together one or more offset matrices to express wrapping
    virtual void getOffsetMatrices(std::vector<Eigen::Matrix4d> &offsetMatrices,const WhirlyKit::Point2f &frameBufferSize);

    /// Minimum valid height above plane
    double minHeightAboveSurface();

    /// Maximum valid height above plane
    double maxHeightAboveSurface();

    /// Set the location, but we may or may not run updates
    void setLoc(WhirlyKit::Point3d &loc,bool runUpdates);

    /// Set the location we're looking from.  Always runs updates
    void setLoc(WhirlyKit::Point3d newLoc);
    
    /// Return the current location
    WhirlyKit::Point3d getLoc() { return loc; }
    
    /// Return the current rotation
    double getRotAngle() { return rotAngle; }
    
    /// Return the wrap status
    bool getWrap() { return wrap; }
    
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
    void setRotAngle(double newRotAngle);
    
    /// Turn a wrapped coordinate (e.g. may be outside -M_PI,M_PI) back into a real world coordinate
    WhirlyKit::Point2f unwrapCoordinate(const WhirlyKit::Point2f &pt);
    
protected:
    /// Viewer location
    WhirlyKit::Point3d loc;
    /// Viewer rotation angle
    double rotAngle;
    /// If set, we're wrapping across the date line
    bool wrap;
    /// Used to update position based on time (or whatever)
    // Note: Porting
//    NSObject<MaplyAnimationDelegate> *delegate;
};

}
