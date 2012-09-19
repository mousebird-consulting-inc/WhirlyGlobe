/*
 *  GlobeMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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


#import "GlobeMath.h"
#import "proj_api.h"

using namespace WhirlyKit;

// These are just pointers and they won't change
// We'll initialize them once
static projPJ pj_latlon,pj_geocentric;

namespace WhirlyKit
{
    
// Initialize the Proj-4 objects
void InitProj4()
{
    // Note: Bad monkey.  No cookie.
//    @synchronized(pj_latlon)
//    {
        if (!pj_latlon || !pj_geocentric)
        {
            pj_latlon = pj_init_plus("+proj=latlong +datum=WGS84");
            pj_geocentric = pj_init_plus("+proj=geocent +datum=WGS84");
        }
//    }
}
    
/// Convert from the local coordinate system to lat/lon
GeoCoord GeoCoordSystem::localToGeographic(WhirlyKit::Point3f pt)
{
    return GeoCoord(pt.x(),pt.y());
}

/// Convert from lat/lon t the local coordinate system
Point3f GeoCoordSystem::geographicToLocal(WhirlyKit::GeoCoord coord)
{
    return Point3f(coord.lon(),coord.lat(),0.0);
}
    
Point3f GeoCoordSystem::LocalToGeocentric(Point3f localPt)
{
    InitProj4();
    
    double x,y,z;
    x = localPt.x(), y = localPt.y(), z = localPt.z();
    pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    return Point3f(x,y,z);
}
    
/// Convert from local coordinates to WGS84 geocentric
Point3f GeoCoordSystem::localToGeocentric(Point3f localPt)
{
    return LocalToGeocentric(localPt);
}
    
Point3f GeoCoordSystem::GeocentricToLocal(Point3f geocPt)
{
    InitProj4();
    
    double x,y,z;
    x = geocPt.x(), y = geocPt.y(), z = geocPt.z();
    pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    return Point3f(x,y,z);
}

/// Convert from WGS84 geocentric to local coordinates
Point3f GeoCoordSystem::geocentricToLocal(Point3f geocPt)
{
    return GeocentricToLocal(geocPt);
}


Mbr GeoCoordSystem::GeographicMbrToLocal(GeoMbr geoMbr)
{
    Mbr localMbr;
    localMbr.addPoint(Point2f(geoMbr.ll().x(),geoMbr.ll().y()));
    localMbr.addPoint(Point2f(geoMbr.ur().x(),geoMbr.ur().y()));
    
    return localMbr;
}

bool GeoCoordSystem::isSameAs(CoordSystem *coordSys)
{
    GeoCoordSystem *other = dynamic_cast<GeoCoordSystem *>(coordSys);
    return (other != NULL);
}

    
Point3f FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f geoPt)
{
    float z = sinf(geoPt.y());
    float rad = sqrtf(1.0-z*z);
    Point3f pt(rad*cosf(geoPt.x()),rad*sinf(geoPt.x()),z);
    return pt;
}
    
Point3f FakeGeocentricDisplayAdapter::localToDisplay(Point3f geoPt)
{
    return LocalToDisplay(geoPt);
}
    
Point3f FakeGeocentricDisplayAdapter::DisplayToLocal(Point3f pt)
{
    GeoCoord geoCoord;
    geoCoord.lat() = asinf(pt.z());
    float rad = sqrtf(1.0-pt.z()*pt.z());
    geoCoord.lon() = acosf(pt.x() / rad);
    if (pt.y() < 0)  geoCoord.lon() *= -1;
        
        return Point3f(geoCoord.lon(),geoCoord.lat(),0.0);    
}
    
Point3f FakeGeocentricDisplayAdapter::displayToLocal(Point3f pt)
{
    return DisplayToLocal(pt);
}
    
Point3f FakeGeocentricDisplayAdapter::normalForLocal(Point3f pt)
{
    return LocalToDisplay(pt);
}

	
}
