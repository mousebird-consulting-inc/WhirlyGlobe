/*  GlobeMath.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "GlobeMath.h"
#import "FlatMath.h"
#import "proj_api.h"

using namespace Eigen;
using namespace WhirlyKit;

// These are just pointers and they won't change
// We'll initialize them once
static projPJ pj_latlon=nullptr,pj_geocentric=nullptr;

namespace WhirlyKit
{

std::once_flag globeMathFlag;
    
// Initialize the Proj-4 objects
void InitProj4()
{
    std::call_once(globeMathFlag, []()
    { 
            pj_latlon = pj_init_plus("+proj=latlong +datum=WGS84");
            pj_geocentric = pj_init_plus("+proj=geocent +datum=WGS84");
    });
}

Point3f GeoCoordSystem::LocalToGeocentric(const Point3f &localPt)
{
    InitProj4();

    double x,y,z;
    x = localPt.x(); y = localPt.y(); z = localPt.z();
    pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    return { (float)x, (float)y, (float)z };
}

Point3d GeoCoordSystem::LocalToGeocentric(const Point3d &localPt)
{
    InitProj4();

    double x,y,z;
    x = localPt.x(); y = localPt.y(); z = localPt.z();
    pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    return { x, y, z };
}

Point3f GeoCoordSystem::GeocentricToLocal(const Point3f &geocPt)
{
    InitProj4();
    
    double x,y,z;
    x = geocPt.x(); y = geocPt.y(); z = geocPt.z();
    pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    return { (float)x, (float)y, (float)z };
}

Point3d GeoCoordSystem::GeocentricToLocal(const Point3d &geocPt)
{
    InitProj4();
    
    double x,y,z;
    x = geocPt.x(); y = geocPt.y(); z = geocPt.z();
    pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    return { x, y, z };
}

Mbr GeoCoordSystem::GeographicMbrToLocal(const GeoMbr &geoMbr)
{
    Mbr localMbr;
    localMbr.addPoint(Point2f(geoMbr.ll().x(),geoMbr.ll().y()));
    localMbr.addPoint(Point2f(geoMbr.ur().x(),geoMbr.ur().y()));
    return localMbr;
}

#if !MAPLY_MINIMAL

bool GeoCoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    const auto other = dynamic_cast<const GeoCoordSystem *>(coordSys);
    return (other != nullptr);
}

Point3f FakeGeocentricDisplayAdapter::LocalToDisplay(const Point3f &geoPt)
{
    float z = sinf(geoPt.y());
    float rad = sqrtf(1.0f-z*z);
    Point3f pt(rad*cosf(geoPt.x()),rad*sinf(geoPt.x()),z);
    // Scale outward with the z value
    if (geoPt.z() != 0.0)
    {
        pt *= 1.0f + geoPt.z() / EarthRadius;
    }
    return pt;
}

Point3d FakeGeocentricDisplayAdapter::LocalToDisplay(const Point3d &geoPt)
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

Point3f FakeGeocentricDisplayAdapter::DisplayToLocal(const Point3f &pt)
{
    const Point3f ptn = pt.normalized();

    GeoCoord geoCoord;
    geoCoord.lat() = asinf(ptn.z());
    float rad = sqrtf(1.0f - ptn.z() * ptn.z());
    geoCoord.lon() = acosf(ptn.x() / rad);
    if (ptn.y() < 0)  geoCoord.lon() *= -1;
        
    return { geoCoord.lon(), geoCoord.lat(), 0.0f };
}

Point3d FakeGeocentricDisplayAdapter::DisplayToLocal(const Point3d &pt)
{
    Point2d geoCoord;
    geoCoord.y() = asin(pt.z());
    double rad = sqrt(1.0-pt.z()*pt.z());
    if (rad >= 0.0)
        geoCoord.x() = acos(pt.x() / rad);
    else
        geoCoord.x() = 0.0;
    if (std::isnan(geoCoord.x()))
        geoCoord.x() = 0.0;
    if (pt.y() < 0)  geoCoord.x() *= -1;
    
    return { geoCoord.x(), geoCoord.y(), 0.0 };
}

Point3f GeocentricDisplayAdapter::LocalToDisplay(const Point3f &geoPt)
{
    return GeoCoordSystem::LocalToGeocentric(geoPt) / EarthRadius;
}

Point3d GeocentricDisplayAdapter::LocalToDisplay(const Point3d &geoPt)
{
    return GeoCoordSystem::LocalToGeocentric(geoPt) / EarthRadius;
}

Point3f GeocentricDisplayAdapter::DisplayToLocal(const Point3f &pt)
{
    return GeoCoordSystem::GeocentricToLocal(Point3f(pt * EarthRadius));
}

Point3d GeocentricDisplayAdapter::DisplayToLocal(const Point3d &pt)
{
    return GeoCoordSystem::GeocentricToLocal(Point3d(pt * EarthRadius));
}

float CheckPointAndNormFacing(const Point3f &dispLoc, const Point3f &norm,
                              const Matrix4f &viewAndModelMat,const Matrix4f &viewModelNormalMat)
{
    Vector4f pt = viewAndModelMat * Vector4f(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4f testDir = viewModelNormalMat * Vector4f(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3f(-pt.x(),-pt.y(),-pt.z()).dot(Vector3f(testDir.x(),testDir.y(),testDir.z()));
}

double CheckPointAndNormFacing(const Point3d &dispLoc,const Point3d &norm,
                               const Matrix4d &viewAndModelMat,const Matrix4d &viewModelNormalMat)
{
    Vector4d pt = viewAndModelMat * Vector4d(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4d testDir = viewModelNormalMat * Vector4d(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3d(-pt.x(),-pt.y(),-pt.z()).dot(Vector3d(testDir.x(),testDir.y(),testDir.z()));
}

#endif //!MAPLY_MINIMAL

}
