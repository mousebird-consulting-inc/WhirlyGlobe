/*
 *  SphericalMercator.mm
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

#import "SphericalMercator.h"
#import "GlobeMath.h"

namespace WhirlyKit
{

SphericalMercatorCoordSystem::SphericalMercatorCoordSystem(float originLon)
    : originLon(originLon)
{
}

// Keep things right below/above the poles
const float PoleLimit = DegToRad(85.05113);
    
/// Convert from the local coordinate system to lat/lon
GeoCoord SphericalMercatorCoordSystem::localToGeographic(Point3f pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atanf(sinhf(pt.y()));
    
    return coord;    
}

/// Convert from lat/lon t the local coordinate system
Point3f SphericalMercatorCoordSystem::geographicToLocal(GeoCoord geo)
{
    Point3f coord;
    coord.x() = geo.lon() - originLon;
    float lat = geo.lat();
    if (lat < -PoleLimit) lat = -PoleLimit;
    if (lat > PoleLimit) lat = PoleLimit;
    coord.y() = logf((1.0f+sinf(lat))/cosf(lat));
    coord.z() = 0.0;
    
    return coord;    
}
    
/// Convert from the local coordinate system to geocentric
Point3f SphericalMercatorCoordSystem::localToGeocentric(Point3f localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}
    
/// Convert from display coordinates to geocentric
Point3f SphericalMercatorCoordSystem::geocentricToLocal(Point3f geocPt)
{
    Point3f geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3f(localPt.x(),localPt.y(),geoCoordPlus.z());    
}


}
