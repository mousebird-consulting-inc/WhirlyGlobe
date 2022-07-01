/*  GlobeMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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
 */

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/** Geographic coordinate system represents a coordinate system that uses
    lat/lon/elevation.
  */
struct GeoCoordSystem : public CoordSystem
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(const Point3f &p) const override { return GeoCoord(p.x(),p.y()); }
    virtual GeoCoord localToGeographic(const Point3d &p) const override { return GeoCoord(p.x(),p.y()); }
    virtual Point2d localToGeographicD(const Point3d &p) const override { return Point2d(p.x(),p.y()); }
    
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(const GeoCoord &p) const override { return {p.lon(),p.lat(),0.0}; }
    virtual Point3d geographicToLocal3d(const GeoCoord &p) const override { return {p.lon(),p.lat(),0.0}; }
    virtual Point3d geographicToLocal(const Point2d &p) const override { return {p.x(),p.y(),0.0}; }
    virtual Point2d geographicToLocal2(const Point2d &p) const override { return {p.x(),p.y()}; }

    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(const Point3f &p) const override { return LocalToGeocentric(p); }
    virtual Point3d localToGeocentric(const Point3d &p) const override { return LocalToGeocentric(p); }
    /// Static version for convenience
    static Point3f LocalToGeocentric(const Point3f &);
    static Point3d LocalToGeocentric(const Point3d &);
    
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(const Point3f &p) const override { return GeocentricToLocal(p); }
    virtual Point3d geocentricToLocal(const Point3d &p) const override { return GeocentricToLocal(p); }
    /// Static version for convenience
    static Point3f GeocentricToLocal(const Point3f &);
    static Point3d GeocentricToLocal(const Point3d &);
    
    /// Convenience routine to convert a whole MBR to local coordinates
    static Mbr GeographicMbrToLocal(const GeoMbr &);

    /// Return true if the other coordinate system is also Geographic
    virtual bool isSameAs(const CoordSystem *coordSys) const override;
};

/** The Fake Geocentric Display Adapter is used by WhirlyGlobe to represent
    a scene that's nominally in lat/lon + elevation but displayed in a fake
    geocentric.  Fake geocentric is just a projection onto a sphere of radius 1.0.
    This is the one used by WhirlyGlobe, unless you're doing something tricky.
    Maply uses flat coordinate systems.
 */
struct FakeGeocentricDisplayAdapter : public CoordSystemDisplayAdapter
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    FakeGeocentricDisplayAdapter() : CoordSystemDisplayAdapter(&geoCoordSys,Point3d(0,0,0)) { }
    virtual ~FakeGeocentricDisplayAdapter() = default;

    /// There are no bounds in fake geocentric since it's not a flat system
    virtual bool getBounds(Point3f &ll,Point3f &ur) const override { return false; }
    
    /// Convert from geographic+height to fake display geocentric
    virtual Point3f localToDisplay(const Point3f &p) const override { return LocalToDisplay(p); }
    virtual Point3d localToDisplay(const Point3d &p) const override { return LocalToDisplay(p); }
    /// Static version
    static Point3f LocalToDisplay(const Point3f&);
    static Point3d LocalToDisplay(const Point3d&);

    /// Convert from fake display geocentric to geographic+height
    virtual Point3f displayToLocal(const Point3f &p) const override { return DisplayToLocal(p); }
    virtual Point3d displayToLocal(const Point3d &p) const override { return DisplayToLocal(p); }
    /// Static version
    static Point3f DisplayToLocal(const Point3f&);
    static Point3d DisplayToLocal(const Point3d&);
    
    /// Return a normal for the given point
    virtual Point3f normalForLocal(const Point3f &p) const override { return LocalToDisplay(p); }
    virtual Point3d normalForLocal(const Point3d &p) const override { return LocalToDisplay(p); }
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() const override { return &geoCoordSys; }
    
    /// This system is round
#if !MAPLY_MINIMAL
    bool isFlat() const override { return false; }
#endif //!MAPLY_MINIMAL

protected:
    mutable GeoCoordSystem geoCoordSys;
};

/** The (real) geocentric display adapter is used to represent a globe with
    a WGS84 ellipsoid.  Only use this one if you've got your data set up to
    deal with it.  We scale to EarthRadius to approximate the radius of 1.0
    we're more or less expecting in other places.
  */
struct GeocentricDisplayAdapter : public CoordSystemDisplayAdapter
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    GeocentricDisplayAdapter() : CoordSystemDisplayAdapter(&geoCoordSys,Point3d(0,0,0)) { }

    /// There are no bounds in fake geocentric since it's not a flat system
    virtual bool getBounds(Point3f &ll,Point3f &ur) const override { return false; }
    
    /// Convert from geographic+height to fake display geocentric
    virtual Point3f localToDisplay(const Point3f &p) const override { return LocalToDisplay(p); }
    virtual Point3d localToDisplay(const Point3d &p) const override { return LocalToDisplay(p); }
    /// Static version
    static Point3f LocalToDisplay(const Point3f &p);
    static Point3d LocalToDisplay(const Point3d &p);
    
    /// Convert from fake display geocentric to geographic+height
    virtual Point3f displayToLocal(const Point3f &p) const override { return DisplayToLocal(p); }
    virtual Point3d displayToLocal(const Point3d &p) const override { return DisplayToLocal(p); }
    /// Static version
    static Point3f DisplayToLocal(const Point3f &p);
    static Point3d DisplayToLocal(const Point3d &p);
    
    /// Return a normal for the given point
    virtual Point3f normalForLocal(const Point3f &p) const override { return LocalToDisplay(p); }
    virtual Point3d normalForLocal(const Point3d &p) const override { return LocalToDisplay(p); }
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() const override { return &geoCoordSys; }
    
    /// This system is round
#if !MAPLY_MINIMAL
    bool isFlat() const override { return false; }
#endif //!MAPLY_MINIMAL

protected:
    mutable GeoCoordSystem geoCoordSys;
};


// Returns negative if the given location (with its normal) is currently facing away from the viewer
float CheckPointAndNormFacing(const Point3f &dispLoc,const Point3f &norm,
                              const Eigen::Matrix4f &viewAndModelMat,
                              const Eigen::Matrix4f &viewModelNormalMat);

// Returns negative if the given location (with its normal) is currently facing away from the viewer
double CheckPointAndNormFacing(const Point3d &dispLoc,const Point3d &norm,
                               const Eigen::Matrix4d &viewAndModelMat,
                               const Eigen::Matrix4d &viewModelNormalMat);

}
