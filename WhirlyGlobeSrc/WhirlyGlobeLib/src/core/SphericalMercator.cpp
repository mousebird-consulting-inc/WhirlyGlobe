/*
 *  SphericalMercator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/12.
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

#import "SphericalMercator.h"
#import "GlobeMath.h"

namespace WhirlyKit
{

SphericalMercatorCoordSystem::SphericalMercatorCoordSystem(float originLon)
    : originLon(originLon)
{
}

// Keep things right below/above the poles
const float PoleLimit = DegToRad(85.05112878);
    
/// Convert from the local coordinate system to lat/lon
GeoCoord SphericalMercatorCoordSystem::localToGeographic(Point3f pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atanf(sinhf(pt.y()));
    
    return coord;    
}

GeoCoord SphericalMercatorCoordSystem::localToGeographic(Point3d pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atan(sinh(pt.y()));
    
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

Point3d SphericalMercatorCoordSystem::geographicToLocal3d(GeoCoord geo)
{
    Point3d coord;
    coord.x() = geo.lon() - originLon;
    float lat = geo.lat();
    if (lat < -PoleLimit) lat = -PoleLimit;
    if (lat > PoleLimit) lat = PoleLimit;
    coord.y() = log((1.0f+sin(lat))/cos(lat));
    coord.z() = 0.0;
    
    return coord;    
}
    
/// Convert from the local coordinate system to geocentric
Point3f SphericalMercatorCoordSystem::localToGeocentric(Point3f localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}

Point3d SphericalMercatorCoordSystem::localToGeocentric(Point3d localPt)
{
    GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::LocalToGeocentric(Point3d(geoCoord.x(),geoCoord.y(),localPt.z()));
}
    
/// Convert from display coordinates to geocentric
Point3f SphericalMercatorCoordSystem::geocentricToLocal(Point3f geocPt)
{
    Point3f geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3f(localPt.x(),localPt.y(),geoCoordPlus.z());    
}

Point3d SphericalMercatorCoordSystem::geocentricToLocal(Point3d geocPt)
{
    Point3d geoCoordPlus = GeoCoordSystem::GeocentricToLocal(geocPt);
    Point3d localPt = geographicToLocal3d(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return Point3d(localPt.x(),localPt.y(),geoCoordPlus.z());
}
    
bool SphericalMercatorCoordSystem::isSameAs(CoordSystem *coordSys)
{
    SphericalMercatorCoordSystem *other = dynamic_cast<SphericalMercatorCoordSystem *>(coordSys);
    
    if (!other)
        return false;
    
    return other->originLon == originLon;
}


SphericalMercatorDisplayAdapter::SphericalMercatorDisplayAdapter(float originLon,GeoCoord geoLL,GeoCoord geoUR)
    : CoordSystemDisplayAdapter(&smCoordSys,Point3d(0,0,0)), smCoordSys(originLon)
{
    Point3f ll3d = smCoordSys.geographicToLocal(geoLL);
    Point3f ur3d = smCoordSys.geographicToLocal(geoUR);
    ll.x() = ll3d.x();  ll.y() = ll3d.y();
    ur.x() = ur3d.x();  ur.y() = ur3d.y();
    
    org = (ll+ur)/2.0;
}
    
SphericalMercatorDisplayAdapter::SphericalMercatorDisplayAdapter(float originLon,GeoCoord geoLL,GeoCoord geoUR,Point3d displayOrigin)
: CoordSystemDisplayAdapter(&smCoordSys,displayOrigin), smCoordSys(originLon)
{
    Point3f ll3d = smCoordSys.geographicToLocal(geoLL);
    Point3f ur3d = smCoordSys.geographicToLocal(geoUR);
    ll.x() = ll3d.x();  ll.y() = ll3d.y();
    ur.x() = ur3d.x();  ur.y() = ur3d.y();
    
    org = (ll+ur)/2.0;
}
    
/// Return the valid boundary in spherical mercator.  Z coordinate is ignored at present.
bool SphericalMercatorDisplayAdapter::getBounds(Point3f &outLL,Point3f &outUR)
{
    outLL.x() = ll.x();  outLL.y() = ll.y();  outLL.z() = 0.0;
    outUR.x() = ur.x();  outUR.y() = ur.y();  outUR.z() = 0.0;
    
    return true;
}

/// Convert from the system's local coordinates to display coordinates
WhirlyKit::Point3f SphericalMercatorDisplayAdapter::localToDisplay(WhirlyKit::Point3f localPt)
{
    Point3f dispPt = localPt-Point3f(org.x(),org.y(),0.0);
    return dispPt;
}

WhirlyKit::Point3d SphericalMercatorDisplayAdapter::localToDisplay(WhirlyKit::Point3d localPt)
{
    Point3d dispPt = localPt-Point3d(org.x(),org.y(),0.0);
    return dispPt;
}
    
/// Convert from display coordinates to the local system's coordinates
WhirlyKit::Point3f SphericalMercatorDisplayAdapter::displayToLocal(WhirlyKit::Point3f dispPt)
{
    Point3f localPt = dispPt+Point3f(org.x(),org.y(),0.0);
    return localPt;
}

WhirlyKit::Point3d SphericalMercatorDisplayAdapter::displayToLocal(WhirlyKit::Point3d dispPt)
{
    Point3d localPt = dispPt+Point3d(org.x(),org.y(),0.0);
    return localPt;
}
    
/// For flat systems the normal is Z up.  For the globe, it's based on the location.
Point3f SphericalMercatorDisplayAdapter::normalForLocal(Point3f)
{
    return Point3f(0,0,1);
}

Point3d SphericalMercatorDisplayAdapter::normalForLocal(Point3d)
{
    return Point3d(0,0,1);
}
    
/// Get a reference to the coordinate system
CoordSystem *SphericalMercatorDisplayAdapter::getCoordSystem()
{
    return &smCoordSys;
}

}
