/*
 *  GlobeMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/** Geographic coordinate system represents a coordinate system that uses
    lat/lon/elevation.
  */
class GeoCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    GeoCoord localToGeographic(Point3d);
    Point2d localToGeographicD(Point3d);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    Point3d geographicToLocal3d(GeoCoord);
    Point3d geographicToLocal(Point2d);

    /// Convert from local coordinates to WGS84 geocentric
    Point3f localToGeocentric(Point3f);
    Point3d localToGeocentric(Point3d);
    /// Static version for convenience
    static Point3f LocalToGeocentric(Point3f);
    static Point3d LocalToGeocentric(Point3d);
    /// Convert from WGS84 geocentric to local coordinates
    Point3f geocentricToLocal(Point3f);
    Point3d geocentricToLocal(Point3d);
    /// Static version for convenience
    static Point3f GeocentricToLocal(Point3f);
    static Point3d GeocentricToLocal(Point3d);
    
    /// Convenience routine to convert a whole MBR to local coordinates
    static Mbr GeographicMbrToLocal(GeoMbr);

    /// Return true if the other coordinate system is also Geographic
    bool isSameAs(CoordSystem *coordSys);
};

/** The Fake Geocentric Display Adapter is used by WhirlyGlobe to represent
    a scene that's nominally in lat/lon + elevation but displayed in a fake
    geocentric.  Fake geocentric is just a projection onto a sphere of radius 1.0.
    This is the one used by WhirlyGlobe, unless you're doing something tricky.
    Maply uses flat coordinte systems.
 */
class FakeGeocentricDisplayAdapter : public WhirlyKit::CoordSystemDisplayAdapter
{
public:
    FakeGeocentricDisplayAdapter() : CoordSystemDisplayAdapter(&geoCoordSys,Point3d(0,0,0)) { }
    virtual ~FakeGeocentricDisplayAdapter() { }

    /// There are no bounds in fake geocentric since it's not a flat system
    virtual bool getBounds(Point3f &ll,Point3f &ur) { return false; }
    
    /// Convert from geographic+height to fake display geocentric
    virtual Point3f localToDisplay(Point3f);
    virtual Point3d localToDisplay(Point3d);
    /// Static version
    static Point3f LocalToDisplay(Point3f);
    static Point3d LocalToDisplay(Point3d);

    /// Convert from fake display geocentric to geographic+height
    virtual Point3f displayToLocal(Point3f);
    virtual Point3d displayToLocal(Point3d);
    /// Static version
    static Point3f DisplayToLocal(Point3f);
    static Point3d DisplayToLocal(Point3d);
    
    /// Return a normal for the given point
    virtual Point3f normalForLocal(Point3f);
    virtual Point3d normalForLocal(Point3d);
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() { return &geoCoordSys; }
    
    /// This system is round
    bool isFlat() { return false; }
    
protected:
    GeoCoordSystem geoCoordSys;
};

/** The (real) geocentric display adapter is used to represent a globe with
    a WGS84 elipsoid.  Only use this one if you've got your data set up to
    deal with it.  We scale to EarthRadius to approximate the radius of 1.0
    we're more or less expecting in other places.
  */
class GeocentricDisplayAdapter : public WhirlyKit::CoordSystemDisplayAdapter
{
public:
    GeocentricDisplayAdapter() : CoordSystemDisplayAdapter(&geoCoordSys,Point3d(0,0,0)) { }

    /// There are no bounds in fake geocentric since it's not a flat system
    virtual bool getBounds(Point3f &ll,Point3f &ur) { return false; }
    
    /// Convert from geographic+height to fake display geocentric
    virtual Point3f localToDisplay(Point3f);
    virtual Point3d localToDisplay(Point3d);
    /// Static version
    static Point3f LocalToDisplay(Point3f);
    static Point3d LocalToDisplay(Point3d);
    
    /// Convert from fake display geocentric to geographic+height
    virtual Point3f displayToLocal(Point3f);
    virtual Point3d displayToLocal(Point3d);
    /// Static version
    static Point3f DisplayToLocal(Point3f);
    static Point3d DisplayToLocal(Point3d);
    
    /// Return a normal for the given point
    virtual Point3f normalForLocal(Point3f);
    virtual Point3d normalForLocal(Point3d);
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() { return &geoCoordSys; }
    
    /// This system is round
    bool isFlat() { return false; }

protected:
    GeoCoordSystem geoCoordSys;
};
    

// Returns negative if the given location (with its normal) is currently facing away from the viewer
float CheckPointAndNormFacing(const Point3f &dispLoc,const Point3f &norm,const Eigen::Matrix4f &viewAndModelMat,const Eigen::Matrix4f &viewModelNormalMat);

// Returns negative if the given location (with its normal) is currently facing away from the viewer
double CheckPointAndNormFacing(const Point3d &dispLoc,const Point3d &norm,const Eigen::Matrix4d &viewAndModelMat,const Eigen::Matrix4d &viewModelNormalMat);

}
