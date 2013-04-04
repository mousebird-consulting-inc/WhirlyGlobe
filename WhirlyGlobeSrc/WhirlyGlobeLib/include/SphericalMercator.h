/*
 *  SphericalMercator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/12.
 *  Copyright 2011-2012 mousebird consulting
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
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    
    /// Convert from the local coordinate system to geocentric
    Point3f localToGeocentric(Point3f);
    /// Convert from display coordinates to geocentric
    Point3f geocentricToLocal(Point3f);
    
    /// True if the other system is Spherical Mercator with the same origin
    virtual bool isSameAs(CoordSystem *coordSys);
        
protected:
    float originLon;
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
    
    /// Return the valid boundary in spherical mercator.  Z coordinate is ignored at present.
    virtual bool getBounds(Point3f &ll,Point3f &ur);
        
    /// Convert from the system's local coordinates to display coordinates
    virtual WhirlyKit::Point3f localToDisplay(WhirlyKit::Point3f);
    
    /// Convert from display coordinates to the local system's coordinates
    virtual WhirlyKit::Point3f displayToLocal(WhirlyKit::Point3f);
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(Point3f);
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem();
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() { return true; }    
    
protected:
    Point2f org,ll,ur;
    SphericalMercatorCoordSystem smCoordSys;
};
    
}
