/*  GlobeMath.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
 *  Copyright 2011-2023 mousebird consulting
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
#import <WhirlyKitLog.h>

//#define PROFILE_PROJ4_LOCKS
#if defined(PROFILE_PROJ4_LOCKS)
#import <ProfilingLockGuard.h>
using LockGuard = WhirlyKit::ProfilingLockGuard;
#else
using LockGuard = std::lock_guard<std::mutex>;
#endif

using namespace Eigen;
using namespace WhirlyKit;

GeoCoordSystem::GeoCoordSystem()
{
    init();
}

GeoCoordSystem::GeoCoordSystem(const GeoCoordSystem &other) :
    CoordSystem(other)
{
    init();
}

GeoCoordSystem::GeoCoordSystem(GeoCoordSystem &&other) :
    CoordSystem(other)
{
    LockGuard lock(other.mutex);
    
    pj_latlon = other.pj_latlon;
    pj_latlon_ctx = other.pj_latlon_ctx;
    pj_geocentric = other.pj_geocentric;
    pj_geocentric_ctx = other.pj_geocentric_ctx;

    other.pj_latlon = nullptr;
    other.pj_latlon_ctx = nullptr;
    other.pj_geocentric = nullptr;
    other.pj_geocentric_ctx = nullptr;
}

void GeoCoordSystem::init()
{
    if ((pj_latlon_ctx = pj_ctx_alloc()))
    {
        pj_latlon = pj_init_plus_ctx(pj_latlon_ctx, "+proj=latlong +datum=WGS84");
    }
    if ((pj_geocentric_ctx = pj_ctx_alloc()))
    {
        pj_geocentric = pj_init_plus_ctx(pj_geocentric_ctx, "+proj=geocent +datum=WGS84");
    }
}

GeoCoordSystem::~GeoCoordSystem()
{
    if (pj_latlon)
    {
        pj_free(pj_latlon);
        pj_latlon = nullptr;
    }
    if (pj_latlon_ctx)
    {
        pj_ctx_free(pj_latlon_ctx);
        pj_latlon_ctx = nullptr;
    }
    if (pj_geocentric)
    {
        pj_free(pj_geocentric);
        pj_geocentric = nullptr;
    }
    if (pj_geocentric_ctx)
    {
        pj_ctx_free(pj_geocentric_ctx);
        pj_geocentric_ctx = nullptr;
    }
}

bool GeoCoordSystem::isValid() const
{
    return pj_latlon && pj_geocentric;
}

CoordSystemRef GeoCoordSystem::clone() const
{
    return std::make_shared<GeoCoordSystem>(*this);
}

Point3f GeoCoordSystem::localToGeocentric(const Point3f &localPt) const
{
    double x = localPt.x(), y = localPt.y(), z = localPt.z();
    if (pj_latlon && pj_geocentric)
    {
        LockGuard lock(mutex);
        pj_transform(pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    }
    return { (float)x, (float)y, (float)z };
}

Point3d GeoCoordSystem::localToGeocentric(const Point3d &localPt) const
{
    double x = localPt.x(), y = localPt.y(), z = localPt.z();
    if (pj_latlon && pj_geocentric)
    {
        LockGuard lock(mutex);
        pj_transform( pj_latlon, pj_geocentric, 1, 1, &x, &y, &z );
    }
    return { x, y, z };
}

Point3f GeoCoordSystem::geocentricToLocal(const Point3f &geocPt) const
{
    double x = geocPt.x(), y = geocPt.y(), z = geocPt.z();
    if (pj_latlon && pj_geocentric)
    {
        LockGuard lock(mutex);
        pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    }
    return { (float)x, (float)y, (float)z };
}

Point3d GeoCoordSystem::geocentricToLocal(const Point3d &geocPt) const
{
    double x = geocPt.x(), y = geocPt.y(), z = geocPt.z();
    if (pj_latlon && pj_geocentric)
    {
        LockGuard lock(mutex);
        pj_transform(pj_geocentric, pj_latlon, 1, 1, &x, &y, &z);
    }
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
    // All instances are equivalent
    const auto other = dynamic_cast<const GeoCoordSystem *>(coordSys);
    return (other != nullptr);
}



FakeGeocentricDisplayAdapter::FakeGeocentricDisplayAdapter() :
    CoordSystemDisplayAdapter(&geoCoordSys, {0,0,0})
{
}

FakeGeocentricDisplayAdapter::FakeGeocentricDisplayAdapter(const FakeGeocentricDisplayAdapter &other) :
    geoCoordSys(other.geoCoordSys),
    CoordSystemDisplayAdapter(&geoCoordSys, {0,0,0})
{
}

CoordSystemDisplayAdapterRef FakeGeocentricDisplayAdapter::clone() const
{
    return std::make_shared<FakeGeocentricDisplayAdapter>(*this);
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

Point3f GeocentricDisplayAdapter::localToDisplay(const Point3f &geoPt) const
{
    return geoCoordSys.localToGeocentric(geoPt) / EarthRadius;
}

Point3d GeocentricDisplayAdapter::localToDisplay(const Point3d &geoPt) const
{
    return geoCoordSys.localToGeocentric(geoPt) / EarthRadius;
}

Point3f GeocentricDisplayAdapter::displayToLocal(const Point3f &pt) const
{
    return geoCoordSys.geocentricToLocal(Point3f(pt * EarthRadius));
}

Point3d GeocentricDisplayAdapter::displayToLocal(const Point3d &pt) const
{
    return geoCoordSys.geocentricToLocal(Point3d(pt * EarthRadius));
}

float WhirlyKit::CheckPointAndNormFacing(const Point3f &dispLoc, const Point3f &norm,
                                         const Matrix4f &viewAndModelMat,const Matrix4f &viewModelNormalMat)
{
    Vector4f pt = viewAndModelMat * Vector4f(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4f testDir = viewModelNormalMat * Vector4f(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3f(-pt.x(),-pt.y(),-pt.z()).dot(Vector3f(testDir.x(),testDir.y(),testDir.z()));
}

double WhirlyKit::CheckPointAndNormFacing(const Point3d &dispLoc,const Point3d &norm,
                                          const Matrix4d &viewAndModelMat,const Matrix4d &viewModelNormalMat)
{
    Vector4d pt = viewAndModelMat * Vector4d(dispLoc.x(),dispLoc.y(),dispLoc.z(),1.0);
    pt /= pt.w();
    Vector4d testDir = viewModelNormalMat * Vector4d(norm.x(),norm.y(),norm.z(),0.0);
    return Vector3d(-pt.x(),-pt.y(),-pt.z()).dot(Vector3d(testDir.x(),testDir.y(),testDir.z()));
}

#endif //!MAPLY_MINIMAL

