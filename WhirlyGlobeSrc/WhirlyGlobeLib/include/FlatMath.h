/*
 *  FlatMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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
    
/** The Plate Careee just unrolls lat/lon in radians and
    represents the map as a flat non-projection of that.
    This is plate carree: http://en.wikipedia.org/wiki/Equirectangular_projection
  */
class PlateCarreeCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    
    /// Convert from the local coordinate system to spherical display (WhirlyGlobe) coordinates (geocentric-ish)
    Point3f localToGeocentricish(Point3f);    
    /// Convert from spherical (WhirlyGlobe) display coordinates to the local coordinate system
    Point3f geocentricishToLocal(Point3f);
    
    /// Working in a flat space
    bool isFlat() { return true; }
};
    
/** Flat Earth refers to the MultiGen flat earth coordinate system.
    This is a scaled unrolling from a center point.
 */
class FlatEarthCoordSystem : public WhirlyKit::CoordSystem
{
public:
    FlatEarthCoordSystem(const GeoCoord &origin);
    
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    
    /// Convert from the local coordinate system to spherical display (WhirlyGlobe) coordinates (geocentric-ish)
    Point3f localToGeocentricish(Point3f);    
    /// Convert from spherical (WhirlyGlobe) display coordinates to the local coordinate system
    Point3f geocentricishToLocal(Point3f);
    
    /// Return the origin
    GeoCoord getOrigin() const;
    
    /// Working in a flat space
    bool isFlat() { return true; }        
    
protected:
    GeoCoord origin;
    float converge;
};

/// A representative earth radius value.  We use this for scaling, not accurate geolocation.
static const float EarthRadius = 6371000;

}
