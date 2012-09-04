/*
 *  FlatMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
 *  Copyright 2011 mousebird consulting
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
    
Point3f PlateCarreeCoordSystem::localToGeocentricish(Point3f pt)
{
    return GeoCoordSystem::LocalToGeocentricish(localToGeographic(pt));
}
    
Point3f PlateCarreeCoordSystem::geocentricishToLocal(Point3f pt)
{
    Point3f coord = GeoCoordSystem::GeocentricishToLocal(pt);
    return geographicToLocal(GeoCoord(coord.x(),coord.y()));
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

Point3f FlatEarthCoordSystem::localToGeocentricish(Point3f inPt)
{
    // Note: This is entirely bogus.  We need to use proj4 and take the elipsoid into account
    GeoCoord coord = localToGeographic(inPt);
    Point3f pt = GeoCoordSystem::LocalToGeocentricish(Point3f(coord.lon(),coord.lat(),0.0));
    
    // And don't forget the Z
    pt *= 1.0 + inPt.z() / EarthRadius;
    
    return pt;
}

Point3f FlatEarthCoordSystem::geocentricishToLocal(Point3f inPt)
{
    // Note: Entirely bogus. Pull in proj4 for the elipsoid
    float len = inPt.norm() - 1.0;
    inPt.normalize();

    Point3f coord = GeoCoordSystem::GeocentricishToLocal(inPt);
    Point3f pt = geographicToLocal(GeoCoord(coord.x(),coord.y()));
    pt.z() = len * EarthRadius;
    
    return pt;
}
    
GeoCoord FlatEarthCoordSystem::getOrigin() const
{
    return origin;
}

}
