/*  Proj4CoordSystem.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/10/15.
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

#import "WhirlyKitLog.h"
#import "Proj4CoordSystem.h"
#import "GlobeMath.h"
#import "proj_api.h"

#define PJ_ERR_BOUNDS -14   // boring out-of-bounds error

//#define PROFILE_PROJ4_LOCKS
#if defined(PROFILE_PROJ4_LOCKS)
#import <ProfilingLockGuard.h>
using LockGuard = WhirlyKit::ProfilingLockGuard;
#else
using LockGuard = std::lock_guard<std::mutex>;
#endif

using namespace WhirlyKit;

Proj4CoordSystem::Proj4CoordSystem(std::string inProj4Str) :
    proj4Str(std::move(inProj4Str))
{
    init();
}

Proj4CoordSystem::Proj4CoordSystem(const Proj4CoordSystem &other) :
    CoordSystem(other),
    proj4Str(other.proj4Str)
{
    init();
}

Proj4CoordSystem::Proj4CoordSystem(Proj4CoordSystem &&other) :
    CoordSystem(other)
{
    LockGuard lock(other.mutex);

    proj4Str = std::move(other.proj4Str);
    pj = other.pj;
    pj_ctx = other.pj_ctx;
    pj_latlon = other.pj_latlon;
    pj_latlon_ctx = other.pj_latlon_ctx;
    pj_geocentric = other.pj_geocentric;
    pj_geocentric_ctx = other.pj_geocentric_ctx;
    other.pj = nullptr;
    other.pj_ctx = nullptr;
    other.pj_latlon = nullptr;
    other.pj_latlon_ctx = nullptr;
    other.pj_geocentric = nullptr;
    other.pj_geocentric_ctx = nullptr;

}

void Proj4CoordSystem::init()
{
    if ((pj_ctx = pj_ctx_alloc()))
    {
        pj = pj_init_plus_ctx(pj_ctx, proj4Str.c_str());
    }
    if ((pj_latlon_ctx = pj_ctx_alloc()))
    {
        pj_latlon = pj_init_plus_ctx(pj_latlon_ctx, "+proj=latlong +datum=WGS84");
    }
    if ((pj_geocentric_ctx = pj_ctx_alloc()))
    {
        pj_geocentric = pj_init_plus_ctx(pj_geocentric_ctx, "+proj=geocent +datum=WGS84");
    }
}

Proj4CoordSystem::~Proj4CoordSystem()
{
    try
    {
        if (pj)
        {
            pj_free(pj);
            pj = nullptr;
        }
        if (pj_ctx)
        {
            pj_ctx_free(pj_ctx);
            pj_ctx = nullptr;
        }

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
    WK_STD_DTOR_CATCH()
}

bool Proj4CoordSystem::isValid() const
{
    return pj && pj_latlon && pj_geocentric;
}

/// Create a new instance equivalent to this one
CoordSystemRef Proj4CoordSystem::clone() const
{
    return std::make_shared<Proj4CoordSystem>(*this);
}

/// Convert from the local coordinate system to lat/lon
GeoCoord Proj4CoordSystem::localToGeographic(const Point3f &pt) const
{
    double x = pt.x(),y = pt.y(),z = pt.z();
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::localToGeographic error (%d) converting to geographic %f,%f,%f",
                           result,pt.x(),pt.y(),pt.z());
            }
            return {0,0};
        }
    }
    return { (float)x, (float)y };
}

GeoCoord Proj4CoordSystem::localToGeographic(const Point3d &pt) const
{
    double x = pt.x(),y = pt.y(),z = pt.z();
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::localToGeographic error (%d) converting to geographic %f,%f,%f",
                           result,pt.x(),pt.y(),pt.z());
            }
            return {0,0};
        }
    }
    return { (float)x, (float)y };
}

Point2d Proj4CoordSystem::localToGeographicD(const Point3d &pt) const
{
    double x = pt.x(),y = pt.y(),z = pt.z();
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::localToGeographicD error (%d) converting to geographic %f,%f,%f",
                           result,pt.x(),pt.y(),pt.z());
            }
            return {0,0};
        }
    }
    return {x,y};
}

/// Convert from lat/lon t the local coordinate system
Point3f Proj4CoordSystem::geographicToLocal(const GeoCoord &geo) const
{
    double x = geo.x(),y = geo.y(),z = 0.0;
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geographicToLocal error (%d) converting from geographic %f,%f",
                           result,geo.x(),geo.y());
            }
            return {0,0,0};
        }
    }
    return { (float)x, (float)y, (float)z };
}

Point3d Proj4CoordSystem::geographicToLocal3d(const GeoCoord &geo) const
{
    double x = geo.x(),y = geo.y(),z = 0.0;
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geographicToLocal3d error (%d) converting from geographic %f,%f",
                           result,geo.x(),geo.y());
            }
            return {0,0,0};
        }
    }
    return {x,y,z};
}

Point3d Proj4CoordSystem::geographicToLocal(const Point2d &geo) const
{
    double x = geo.x(),y = geo.y(),z = 0.0;
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geographicToLocal error (%d) converting from geographic %f,%f",
                           result,geo.x(),geo.y());
            }
            return {0,0,0};
        }
    }
    return {x,y,z};
}

Point2d Proj4CoordSystem::geographicToLocal2(const Point2d &geo) const
{
    double x = geo.x(),y = geo.y(),z = 0.0;
    if (pj && pj_latlon)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geographicToLocal error (%d) converting from geographic %f,%f",
                           result,geo.x(),geo.y());
            }
            return {0,0};
        }
    }
    return {x,y};
}

/// Convert from the local coordinate system to geocentric
Point3f Proj4CoordSystem::localToGeocentric(const Point3f &localPt) const
{
    double x = localPt.x(),y = localPt.y(),z = localPt.z();
    if (pj && pj_geocentric)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj, pj_geocentric, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::localToGeocentric error (%d) converting to geocentric %f,%f,%f",
                           result,localPt.x(),localPt.y(),localPt.z());
            }
            return {0,0,0};
        }
    }
    return { (float)x, (float)y, (float)z };
}

Point3d Proj4CoordSystem::localToGeocentric(const Point3d &localPt) const
{
    double x = localPt.x(),y = localPt.y(),z = localPt.z();
    if (pj && pj_geocentric)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj, pj_geocentric, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::localToGeocentric error (%d) converting to geocentric %f,%f,%f",
                           result,localPt.x(),localPt.y(),localPt.z());
            }
            return {0,0,0};
        }
    }
    return {x,y,z};
}

/// Convert from display coordinates to geocentric
Point3f Proj4CoordSystem::geocentricToLocal(const Point3f &geocPt) const
{
    double x = geocPt.x(),y = geocPt.y(),z = geocPt.z();
    if (pj && pj_geocentric)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_geocentric, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geocentricToLocal error (%d) converting from geocentric %f,%f,%f",
                           result,geocPt.x(),geocPt.y(),geocPt.z());
            }
            return {0,0,0};
        }
    }
    return { (float)x, (float)y, (float)z };
}

Point3d Proj4CoordSystem::geocentricToLocal(const Point3d &geocPt) const
{
    double x = geocPt.x(),y = geocPt.y(),z = geocPt.z();
    if (pj && pj_geocentric)
    {
        LockGuard lock(mutex);
        const auto result = pj_transform(pj_geocentric, pj, 1, 1, &x, &y, &z);
        if (result != 0) {
            if (result != PJ_ERR_BOUNDS) {
                wkLogLevel(Debug, "Proj4CoordSystem::geocentricToLocal error (%d) converting from geocentric %f,%f,%f",
                           result,geocPt.x(),geocPt.y(),geocPt.z());
            }
            return {0,0,0};
        }
    }
    return {x,y,z};
}

bool Proj4CoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    const auto other = dynamic_cast<const Proj4CoordSystem *>(coordSys);
    return other && proj4Str == other->proj4Str;
}
