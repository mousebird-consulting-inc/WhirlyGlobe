/*
 *  FlatMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import "FlatMath.h"
#import "GlobeMath.h"

namespace WhirlyKit
{
        
GeoCoord PlateCarreeCoordSystem::localToGeographic(Point3f pt)
{
    return GeoCoord(pt.x(),pt.y());
}

Point3f PlateCarreeCoordSystem::geographicToLocal(GeoCoord geo)
{
    return Point3f(geo.lon(),geo.lat(),0.0);
}
    
Point3f PlateCarreeCoordSystem::localToGeocentric(Point3f localPt)
{
    return GeoCoordSystem::LocalToGeocentric(Point3f(localPt.x(),localPt.y(),localPt.z()));
}

/// Convert from WGS84 geocentric to local coordinates
Point3f PlateCarreeCoordSystem::geocentricToLocal(Point3f geocPt)
{
    return GeoCoordSystem::GeocentricToLocal(geocPt);
}
    
bool PlateCarreeCoordSystem::isSameAs(CoordSystem *coordSys)
{
    PlateCarreeCoordSystem *other = dynamic_cast<PlateCarreeCoordSystem *>(coordSys);
    
    return (other != NULL);
}

        
FlatEarthCoordSystem::FlatEarthCoordSystem(const GeoCoord &origin)
    : origin(origin)
{
    converge = cosf(origin.lat());    
}
    
// Note: This is completely bogus
static const float MetersPerRadian = 111120.0 * 180.0 / M_PI;

GeoCoord FlatEarthCoordSystem::localToGeographic(Point3f pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() / (MetersPerRadian * converge) + origin.lon();
    coord.lat() = pt.y() / MetersPerRadian + origin.lat();
    
    return coord;
}

Point3f FlatEarthCoordSystem::geographicToLocal(GeoCoord geo)
{
    Point3f pt;
    pt.x() = (geo.lon() - origin.lon()) * converge * MetersPerRadian;
    pt.y() = (geo.lat() - origin.lat()) * MetersPerRadian;
    pt.z() = 0.0;
    
    return pt;
}

// Note: This needs to be turned into a display adapter
//Point3f FlatEarthCoordSystem::localToGeocentricish(Point3f inPt)
//{
//    // Note: This is entirely bogus.  We need to use proj4 and take the elipsoid into account
//    GeoCoord coord = localToGeographic(inPt);
//    Point3f pt = GeoCoordSystem::LocalToGeocentricish(Point3f(coord.lon(),coord.lat(),0.0));
//    
//    // And don't forget the Z
//    pt *= 1.0 + inPt.z() / EarthRadius;
//    
//    return pt;
//}
//
//Point3f FlatEarthCoordSystem::geocentricishToLocal(Point3f inPt)
//{
//    // Note: Entirely bogus. Pull in proj4 for the elipsoid
//    float len = inPt.norm() - 1.0;
//    inPt.normalize();
//
//    Point3f coord = GeoCoordSystem::GeocentricishToLocal(inPt);
//    Point3f pt = geographicToLocal(GeoCoord(coord.x(),coord.y()));
//    pt.z() = len * EarthRadius;
//    
//    return pt;
//}
    
/// Convert from local coordinates to WGS84 geocentric
Point3f FlatEarthCoordSystem::localToGeocentric(Point3f localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}

/// Convert from WGS84 geocentric to local coordinates
Point3f FlatEarthCoordSystem::geocentricToLocal(Point3f geocPt)
{
    Point3f geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3f(localPt.x(),localPt.y(),geoCoordPlus.z());
}

bool FlatEarthCoordSystem::isSameAs(CoordSystem *coordSys)
{
    FlatEarthCoordSystem *other = dynamic_cast<FlatEarthCoordSystem *>(coordSys);
    
    if (!other)
        return false;
    
    return other->origin == origin;
}
    
GeoCoord FlatEarthCoordSystem::getOrigin() const
{
    return origin;
}

}
