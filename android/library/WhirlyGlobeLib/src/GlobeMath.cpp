/*
 *  GlobeMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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


#import "GlobeMath.h"
#import "FlatMath.h"
#import "proj_api.h"

using namespace Eigen;
using namespace WhirlyKit;

// These are just pointers and they won't change
// We'll initialize them once
static projPJ pj_latlon=NULL,pj_geocentric=NULL;

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

GeoCoord GeoCoordSystem::localToGeographic(WhirlyKit::Point3d pt)
{
    return GeoCoord(pt.x(),pt.y());
}

Point2d GeoCoordSystem::localToGeographicD(WhirlyKit::Point3d pt)
{
    return Point2d(pt.x(),pt.y());
}

/// Convert from lat/lon t the local coordinate system
Point3f GeoCoordSystem::geographicToLocal(WhirlyKit::GeoCoord coord)
{
    return Point3f(coord.lon(),coord.lat(),0.0);
}

Point3d GeoCoordSystem::geographicToLocal3d(WhirlyKit::GeoCoord coord)
{
    return Point3d(coord.lon(),coord.lat(),0.0);
}

Point3d GeoCoordSystem::geographicToLocal(WhirlyKit::Point2d coord)
{
    return Point3d(coord.x(),coord.y(),0.0);
}

Point3f GeoCoordSystem::LocalToGeocentric(Point3f localPt)
{
    InitProj4();
    
    double x,y,z;
    x = localPt.x(), y = localPt.y(), z = localPt.z();
    pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    return Point3f(x,y,z);
}

Point3d GeoCoordSystem::LocalToGeocentric(Point3d localPt)
{
    InitProj4();
    
    double x,y,z;
    x = localPt.x(), y = localPt.y(), z = localPt.z();
    pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    return Point3d(x,y,z);
}

/// Convert from local coordinates to WGS84 geocentric
Point3f GeoCoordSystem::localToGeocentric(Point3f localPt)
{
    return LocalToGeocentric(localPt);
}

Point3d GeoCoordSystem::localToGeocentric(Point3d localPt)
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

Point3d GeoCoordSystem::GeocentricToLocal(Point3d geocPt)
{
    InitProj4();
    
    double x,y,z;
    x = geocPt.x(), y = geocPt.y(), z = geocPt.z();
    pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    return Point3d(x,y,z);
}
    
/// Convert from WGS84 geocentric to local coordinates
Point3f GeoCoordSystem::geocentricToLocal(Point3f geocPt)
{
    return GeocentricToLocal(geocPt);
}

Point3d GeoCoordSystem::geocentricToLocal(Point3d geocPt)
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
    // Scale outward with the z value
    if (geoPt.z() != 0.0)
    {
        pt *= 1.0 + geoPt.z() / EarthRadius;
    }
    return pt;
}

Point3d FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d geoPt)
{
    double z = sin(geoPt.y());
    double rad = sqrt(1.0-z*z);
    Point3d pt(rad*cos(geoPt.x()),rad*sin(geoPt.x()),z);
    // Scale outward with the z value
    if (geoPt.z() != 0.0)
    {
        pt *= 1.0 + geoPt.z() / EarthRadius;
    }
    return pt;
}
    
Point3f FakeGeocentricDisplayAdapter::localToDisplay(Point3f geoPt)
{
    return LocalToDisplay(geoPt);
}

Point3d FakeGeocentricDisplayAdapter::localToDisplay(Point3d geoPt)
{
    return LocalToDisplay(geoPt);
}
    
Point3f FakeGeocentricDisplayAdapter::DisplayToLocal(Point3f pt)
{
    pt.normalize();
    
    GeoCoord geoCoord;
    geoCoord.lat() = asinf(pt.z());
    float rad = sqrtf(1.0-pt.z()*pt.z());
    geoCoord.lon() = acosf(pt.x() / rad);
    if (pt.y() < 0)  geoCoord.lon() *= -1;
        
    return Point3f(geoCoord.lon(),geoCoord.lat(),0.0);
}

Point3d FakeGeocentricDisplayAdapter::DisplayToLocal(Point3d pt)
{
    Point2d geoCoord;
    geoCoord.y() = asin(pt.z());
    double rad = sqrt(1.0-pt.z()*pt.z());
    geoCoord.x() = acos(pt.x() / rad);
    if (pt.y() < 0)  geoCoord.x() *= -1;
    
    return Point3d(geoCoord.x(),geoCoord.y(),0.0);
}
    
Point3f FakeGeocentricDisplayAdapter::displayToLocal(Point3f pt)
{
    return DisplayToLocal(pt);
}
 
Point3d FakeGeocentricDisplayAdapter::displayToLocal(Point3d pt)
{
    return DisplayToLocal(pt);
}
    
Point3f FakeGeocentricDisplayAdapter::normalForLocal(Point3f pt)
{
    return LocalToDisplay(pt);
}

Point3d FakeGeocentricDisplayAdapter::normalForLocal(Point3d pt)
{
    return LocalToDisplay(pt);
}
    
	
Point3f GeocentricDisplayAdapter::LocalToDisplay(Point3f geoPt)
{
    Point3f geoCpt = GeoCoordSystem::LocalToGeocentric(geoPt);
    return Point3f(geoCpt.x()/EarthRadius,geoCpt.y()/EarthRadius,geoCpt.z()/EarthRadius);
}

Point3d GeocentricDisplayAdapter::LocalToDisplay(Point3d geoPt)
{
    Point3d geoCpt = GeoCoordSystem::LocalToGeocentric(geoPt);
    return Point3d(geoCpt.x()/EarthRadius,geoCpt.y()/EarthRadius,geoCpt.z()/EarthRadius);
}

Point3f GeocentricDisplayAdapter::localToDisplay(Point3f geoPt)
{
    return LocalToDisplay(geoPt);
}

Point3d GeocentricDisplayAdapter::localToDisplay(Point3d geoPt)
{
    return LocalToDisplay(geoPt);
}

Point3f GeocentricDisplayAdapter::DisplayToLocal(Point3f pt)
{
    Point3f geoCpt = pt * EarthRadius;
    return GeoCoordSystem::GeocentricToLocal(geoCpt);
}

Point3d GeocentricDisplayAdapter::DisplayToLocal(Point3d pt)
{
    Point3d geoCpt = pt * EarthRadius;
    return GeoCoordSystem::GeocentricToLocal(geoCpt);
}

Point3f GeocentricDisplayAdapter::displayToLocal(Point3f pt)
{
    return DisplayToLocal(pt);
}

Point3d GeocentricDisplayAdapter::displayToLocal(Point3d pt)
{
    return DisplayToLocal(pt);
}

Point3f GeocentricDisplayAdapter::normalForLocal(Point3f pt)
{
    return LocalToDisplay(pt);
}

Point3d GeocentricDisplayAdapter::normalForLocal(Point3d pt)
{
    return LocalToDisplay(pt);
}
    
float CheckPointAndNormFacing(const Point3f &dispLoc,const Point3f &norm,const Matrix4f &viewAndModelMat,const Matrix4f &viewModelNormalMat)
{
    Vector4f pt = viewAndModelMat * Vector4f(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4f testDir = viewModelNormalMat * Vector4f(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3f(-pt.x(),-pt.y(),-pt.z()).dot(Vector3f(testDir.x(),testDir.y(),testDir.z()));
}
    
double CheckPointAndNormFacing(const Point3d &dispLoc,const Point3d &norm,const Matrix4d &viewAndModelMat,const Matrix4d &viewModelNormalMat)
{
    Vector4d pt = viewAndModelMat * Vector4d(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4d testDir = viewModelNormalMat * Vector4d(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3d(-pt.x(),-pt.y(),-pt.z()).dot(Vector3d(testDir.x(),testDir.y(),testDir.z()));
}

}
