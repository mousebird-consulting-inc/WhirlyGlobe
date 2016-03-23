/*
 *  FlatMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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
    
/** The Plate Careee just unrolls lat/lon in radians and
    represents the map as a flat non-projection of that.
    This is plate carree: http://en.wikipedia.org/wiki/Equirectangular_projection
  */
class PlateCarreeCoordSystem : public CoordSystem
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
    /// Convert from WGS84 geocentric to local coordinates
    Point3f geocentricToLocal(Point3f);
    Point3d geocentricToLocal(Point3d);
        
    /// Return true if the other coordinate system is also Plate Carree
    bool isSameAs(CoordSystem *coordSys);
};
    
/** Flat Earth refers to the MultiGen flat earth coordinate system.
    This is a scaled unrolling from a center point.
 */
class FlatEarthCoordSystem : public CoordSystem
{
public:
    FlatEarthCoordSystem(const GeoCoord &origin);
    
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
    /// Convert from WGS84 geocentric to local coordinates
    Point3f geocentricToLocal(Point3f);
    Point3d geocentricToLocal(Point3d);
    
    /// Return true if the other coordinate system is Flat Earth with the same origin
    bool isSameAs(CoordSystem *coordSys);

    /// Return the origin
    GeoCoord getOrigin() const;
    
protected:
    GeoCoord origin;
    float converge;
};
    
/// A representative earth radius value.  We use this for scaling, not accurate geolocation.
static const float EarthRadius = 6371000;

}
