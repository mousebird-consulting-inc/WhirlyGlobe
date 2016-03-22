/*
 *  SphericalMercator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/12.
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit 
{

/** The Mercator Projection, bane of cartographers everywhere.
 It stretches out the world in a familiar way, making the US
 look almost as big as our collective ego.  And Greenland.  For some reason.
 */
class SphericalMercatorCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// Construct with an optional origin for the projection in radians
    /// The equator is default
    SphericalMercatorCoordSystem(float originLon=0.0);
    
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    GeoCoord localToGeographic(Point3d);
    Point2d localToGeographicD(Point3d);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    Point3d geographicToLocal3d(GeoCoord);
    Point3d geographicToLocal(Point2d);
    
    /// Convert from the local coordinate system to geocentric
    Point3f localToGeocentric(Point3f);
    Point3d localToGeocentric(Point3d);
    /// Convert from display coordinates to geocentric
    Point3f geocentricToLocal(Point3f);
    Point3d geocentricToLocal(Point3d);
    
    /// True if the other system is Spherical Mercator with the same origin
    virtual bool isSameAs(CoordSystem *coordSys);
        
protected:
    double originLon;
};
    
/** The Spherical Mercator Display adapter uses an origin and geo MBR
    to convert coordinates in and out of display space.
  */
class SphericalMercatorDisplayAdapter : public CoordSystemDisplayAdapter
{
public:
    /// Construct with the origin longitude for the projection and the
    ///  bounding box for the constraints.  The latter are in geographic (WGS84)
    SphericalMercatorDisplayAdapter(float originLon,GeoCoord ll,GeoCoord ur);
    
    /// Construct with the origin longitude for the projection and the
    ///  bounding box for the constraints.  The latter are in geographic (WGS84).
    /// Also pass in a display origin.  We'll offset everything from there.
    SphericalMercatorDisplayAdapter(float originLon,GeoCoord ll,GeoCoord ur,Point3d displayOrigin);
    
    /// Return the valid boundary in spherical mercator.  Z coordinate is ignored at present.
    virtual bool getBounds(Point3f &ll,Point3f &ur);
        
    /// Convert from the system's local coordinates to display coordinates
    virtual WhirlyKit::Point3f localToDisplay(WhirlyKit::Point3f);
    virtual WhirlyKit::Point3d localToDisplay(WhirlyKit::Point3d);
    
    /// Convert from display coordinates to the local system's coordinates
    virtual WhirlyKit::Point3f displayToLocal(WhirlyKit::Point3f);
    virtual WhirlyKit::Point3d displayToLocal(WhirlyKit::Point3d);
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(Point3f);
    virtual Point3d normalForLocal(Point3d);
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem();
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() { return true; }    
    
protected:
    Point2d org,ll,ur;
    SphericalMercatorCoordSystem smCoordSys;
};
    
}
