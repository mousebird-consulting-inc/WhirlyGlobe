/*
 *  FlatMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011-2015 mousebird consulting
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

GeoCoord PlateCarreeCoordSystem::localToGeographic(Point3d pt)
{
    return GeoCoord(pt.x(),pt.y());
}
    
Point2d PlateCarreeCoordSystem::localToGeographicD(Point3d pt)
{
    return Point2d(pt.x(),pt.y());
}

Point3f PlateCarreeCoordSystem::geographicToLocal(GeoCoord geo)
{
    return Point3f(geo.lon(),geo.lat(),0.0);
}

Point3d PlateCarreeCoordSystem::geographicToLocal3d(GeoCoord geo)
{
    return Point3d(geo.lon(),geo.lat(),0.0);
}
    
Point3d PlateCarreeCoordSystem::geographicToLocal(Point2d geo)
{
    return Point3d(geo.x(),geo.y(),0.0);
}

Point3f PlateCarreeCoordSystem::localToGeocentric(Point3f localPt)
{
    return GeoCoordSystem::LocalToGeocentric(Point3f(localPt.x(),localPt.y(),localPt.z()));
}

Point3d PlateCarreeCoordSystem::localToGeocentric(Point3d localPt)
{
    return GeoCoordSystem::LocalToGeocentric(Point3d(localPt.x(),localPt.y(),localPt.z()));
}
    
/// Convert from WGS84 geocentric to local coordinates
Point3f PlateCarreeCoordSystem::geocentricToLocal(Point3f geocPt)
{
    return GeoCoordSystem::GeocentricToLocal(geocPt);
}

Point3d PlateCarreeCoordSystem::geocentricToLocal(Point3d geocPt)
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

GeoCoord FlatEarthCoordSystem::localToGeographic(Point3d pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() / (MetersPerRadian * converge) + origin.lon();
    coord.lat() = pt.y() / MetersPerRadian + origin.lat();
    
    return coord;
}
    
Point2d FlatEarthCoordSystem::localToGeographicD(Point3d pt)
{
    Point2d coord;
    coord.x() = pt.x() / (MetersPerRadian * converge) + origin.lon();
    coord.y() = pt.y() / MetersPerRadian + origin.lat();
    
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

Point3d FlatEarthCoordSystem::geographicToLocal3d(GeoCoord geo)
{
    Point3d pt;
    pt.x() = (geo.lon() - origin.lon()) * converge * MetersPerRadian;
    pt.y() = (geo.lat() - origin.lat()) * MetersPerRadian;
    pt.z() = 0.0;
    
    return pt;
}
    
Point3d FlatEarthCoordSystem::geographicToLocal(Point2d geo)
{
    Point3d pt;
    pt.x() = (geo.x() - origin.lon()) * converge * MetersPerRadian;
    pt.y() = (geo.y() - origin.lat()) * MetersPerRadian;
    pt.z() = 0.0;
    
    return pt;
}
        
/// Convert from local coordinates to WGS84 geocentric
Point3f FlatEarthCoordSystem::localToGeocentric(Point3f localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}

Point3d FlatEarthCoordSystem::localToGeocentric(Point3d localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3d(geoCoord.x(),geoCoord.y(),localPt.z()));
}
    
/// Convert from WGS84 geocentric to local coordinates
Point3f FlatEarthCoordSystem::geocentricToLocal(Point3f geocPt)
{
    Point3f geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3f(localPt.x(),localPt.y(),geoCoordPlus.z());
}

Point3d FlatEarthCoordSystem::geocentricToLocal(Point3d geocPt)
{
    Point3d geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3d localPt = geographicToLocal3d(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3d(localPt.x(),localPt.y(),geoCoordPlus.z());
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
