/*
 *  GlobeView.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/14/11.
 *  Copyright 2011-2016 mousebird consulting
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
#import "GlobeMath.h"

//namespace WhirlyGlobe
//{
//    class GlobeView;
//}
//
// Note: Porting
///// Animation callback
//@protocol WhirlyGlobeAnimationDelegate
///// Called every tick to update the globe position
//- (void)updateView:(WhirlyGlobe::GlobeView *)globeView;
//@end

namespace WhirlyGlobe
{

/** Parameters associated with viewing the globe.
    Modify the rotation quaternion to change the current
    view location.  Set the delegate to smoothly change
    location over time.
 */
class GlobeView : public WhirlyKit::View
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    GlobeView(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter);
    /// Copy constructor
    GlobeView(const GlobeView &that);
    virtual ~GlobeView();
    
    /// Used to update position based on time (or whatever other factor you like)
    // Note: Porting
//    NSObject<WhirlyGlobeAnimationDelegate> *delegate;

    /// Return min/max valid heights above globe
    double minHeightAboveGlobe();
    double maxHeightAboveGlobe();

    /// Set the height above globe, taking constraints into account
    void setHeightAboveGlobe(double newH);

    /// This version allows you to not update the watchers, if you're doing a bunch of updates at once
    void setHeightAboveGlobe(double newH,bool updateWatchers);

    /// This version avoids the limit calculations (Kind of a hack)
    void setHeightAboveGlobeNoLimits(double newH,bool updateWatchers);

    /// Update the quaternion
    void setRotQuat(Eigen::Quaterniond rotQuat);

    /// This version allows you to not update the watchers.
    void setRotQuat(Eigen::Quaterniond rotQuat,bool updateWatchers);
    
    /// Return the current quaternion
    Eigen::Quaterniond getRotQuat() { return rotQuat; }
    
    /// Return the tilt
    double getTilt() { return tilt; }
    
    /// Set the tilt
    void setTilt(double tilt);
    
    /// Set the far clipping plane
    void setFarClippingPlane(double farClip);

    /// Calculate the z offset to make the earth appear where we want it
    double calcEarthZOffset();
    
    /// Calculate model matrix
    Eigen::Matrix4d calcModelMatrix();
    
    /// Calculate view matrix
    Eigen::Matrix4d calcViewMatrix();

    /// Return where up (0,0,1) is after model rotation
    Eigen::Vector3d currentUp();

    /// Calculate where the eye is in model coordinates
    Eigen::Vector3d eyePos();

    /// Given a rotation, where would (0,0,1) wind up
    static Eigen::Vector3d prospectiveUp(Eigen::Quaterniond &prospectiveRot);

    /** Given a location on the screen and the screen size, figure out where we touched the sphere
        Returns true if we hit and where
        Returns false if not and the closest point on the sphere
     */
    bool pointOnSphereFromScreen(const WhirlyKit::Point2f &pt,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize,WhirlyKit::Point3d *hit,bool normalized);

    /** Given a location on the screen and the screen size, figure out where we touched the sphere
     Returns true if we hit and where
     Returns false if not and the closest point on the sphere
     */
    bool pointOnSphereFromScreen(const WhirlyKit::Point2f &pt,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize,WhirlyKit::Point3d *hit,bool normalized,double radius);

    /** From a world location (3D), figure out the projection to the screen
        Returns a point within the frame
      */
    WhirlyKit::Point2f pointOnScreenFromSphere(const WhirlyKit::Point3d &worldLoc,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize);

    /** Construct a rotation to the given location
        and return it.  Doesn't actually do anything yet.
     */
    Eigen::Quaterniond makeRotationToGeoCoord(const WhirlyKit::GeoCoord &worldLoc,bool northUp);

    // Construct a rotation to given location and heading
    Eigen::Quaterniond makeRotationToGeoCoord(const WhirlyKit::Point2d &worldCoord,bool northUp);

    // Construct a rotation to given location and heading
    Eigen::Quaterniond makeRotationToGeoCoord(const WhirlyKit::GeoCoord &worldCoord,double heading);

    /// Cancel any outstanding animation
    void cancelAnimation();

    /// Renderer calls this every update
    void animate();

    /// Calculate the Z buffer resolution
    float calcZbufferRes();

    /// Height above the globe
    double heightAboveSurface();
    
    /// Set the change delegate
    // Note: Porting
//    void setDelegate(NSObject<WhirlyGlobeAnimationDelegate> *delegate);
    
    // These are all for continuous zoom mode
    double absoluteMinHeight;
    double heightInflection;
    double defaultNearPlane;
    double absoluteMinNearPlane;
    double defaultFarPlane;
    double absoluteMinFarPlane;
    
    double getHeightAboveGlobe();
    
protected:
    void privateSetHeightAboveGlobe(double newH,bool updateWatchers);
    
    /// The globe has a radius of 1.0 so 1.0 + heightAboveGlobe is the offset from the middle of the globe
    double heightAboveGlobe;
    /// Quaternion used for rotation from origin state
    Eigen::Quaterniond rotQuat;
    /// The view can have a tilt.  0 is straight down.  PI/2 is looking to the horizon.
    double tilt;
    WhirlyKit::FakeGeocentricDisplayAdapter fakeGeoC;
};

}
