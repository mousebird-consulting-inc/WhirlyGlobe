/*  FlatMath.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import "FlatMath.h"
#import "GlobeMath.h"

using namespace WhirlyKit;

PlateCarreeCoordSystem::PlateCarreeCoordSystem(const PlateCarreeCoordSystem& other) :
    GeoCoordSystem(other)
{
}

bool PlateCarreeCoordSystem::isValid() const
{
    return GeoCoordSystem::isValid();
}

CoordSystemRef PlateCarreeCoordSystem::clone() const
{
    return std::make_shared<PlateCarreeCoordSystem>(*this);
}

Point3f PlateCarreeCoordSystem::localToGeocentric(const Point3f &localPt) const
{
    return GeoCoordSystem::localToGeocentric(localPt.cast<float>());
}

Point3d PlateCarreeCoordSystem::localToGeocentric(const Point3d &localPt) const
{
    return GeoCoordSystem::localToGeocentric(localPt);
}

/// Convert from WGS84 geocentric to local coordinates
Point3f PlateCarreeCoordSystem::geocentricToLocal(const Point3f &geocPt) const
{
    return GeoCoordSystem::geocentricToLocal(geocPt);
}

Point3d PlateCarreeCoordSystem::geocentricToLocal(const Point3d &geocPt) const
{
    return GeoCoordSystem::geocentricToLocal(geocPt);
}

bool PlateCarreeCoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    return GeoCoordSystem::isSameAs(coordSys);
}


FlatEarthCoordSystem::FlatEarthCoordSystem(const GeoCoordD &origin) :
    origin(origin),
    converge(std::cos(origin.lat()))
{
}

// Works for flat earth, but not ideal
static constexpr double MetersPerRadian = 111120.0 * 180.0 / M_PI;

FlatEarthCoordSystem::FlatEarthCoordSystem(const FlatEarthCoordSystem& other) :
    GeoCoordSystem(other),
    origin(other.origin),
    converge(other.converge)
{
}

bool FlatEarthCoordSystem::isValid() const
{
    return true;        // TODO: validate origin
}

CoordSystemRef FlatEarthCoordSystem::clone() const
{
    return std::make_shared<FlatEarthCoordSystem>(*this);
}

GeoCoord FlatEarthCoordSystem::localToGeographic(const Point3f &pt) const
{
    return {
        (float)(pt.x() / (MetersPerRadian * converge) + origin.lon()),
        (float)(pt.y() / MetersPerRadian + origin.lat())
    };
}

GeoCoord FlatEarthCoordSystem::localToGeographic(const Point3d &pt) const
{
    return {
        (float)(pt.x() / (MetersPerRadian * converge) + origin.lon()),
        (float)(pt.y() / MetersPerRadian + origin.lat())
    };
}

Point2d FlatEarthCoordSystem::localToGeographicD(const Point3d &pt) const
{
    return {
        pt.x() / (MetersPerRadian * converge) + origin.lon(),
        pt.y() / MetersPerRadian + origin.lat()
    };
}

Point3f FlatEarthCoordSystem::geographicToLocal(const GeoCoord &geo) const
{
    return {
        (float)((geo.lon() - origin.lon()) * converge * MetersPerRadian),
        (float)((geo.lat() - origin.lat()) * MetersPerRadian),
        0.0f,
    };
}

Point3d FlatEarthCoordSystem::geographicToLocal3d(const GeoCoord &geo) const
{
    return {
        (geo.lon() - origin.lon()) * converge * MetersPerRadian,
        (geo.lat() - origin.lat()) * MetersPerRadian,
        0.0
    };
}

Point3d FlatEarthCoordSystem::geographicToLocal(const Point2d &geo) const
{
    return {
        (geo.x() - origin.lon()) * converge * MetersPerRadian,
        (geo.y() - origin.lat()) * MetersPerRadian,
        0.0
    };
}

Point2d FlatEarthCoordSystem::geographicToLocal2(const Point2d &geo) const
{
    return {(geo.x() - origin.lon()) * converge * MetersPerRadian,
        (geo.y() - origin.lat()) * MetersPerRadian };
}

/// Convert from local coordinates to WGS84 geocentric
Point3f FlatEarthCoordSystem::localToGeocentric(const Point3f &localPt) const
{
    const GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::localToGeocentric(Point3f(geoCoord.x(),geoCoord.y(),localPt.z()));
}

Point3d FlatEarthCoordSystem::localToGeocentric(const Point3d &localPt) const
{
    const GeoCoord geoCoord = localToGeographic(localPt);
    return GeoCoordSystem::localToGeocentric(Point3d(geoCoord.x(),geoCoord.y(),localPt.z()));
}

/// Convert from WGS84 geocentric to local coordinates
Point3f FlatEarthCoordSystem::geocentricToLocal(const Point3f &geocPt) const
{
    const Point3f geoCoordPlus = GeoCoordSystem::geocentricToLocal(geocPt);
    const Point3f localPt = geographicToLocal(GeoCoord(geoCoordPlus.x(),geoCoordPlus.y()));
    return { localPt.x(),localPt.y(),geoCoordPlus.z() };
}

Point3d FlatEarthCoordSystem::geocentricToLocal(const Point3d &geocPt) const
{
    const Point3d geoCoordPlus = GeoCoordSystem::geocentricToLocal(geocPt);
    const Point3d localPt = geographicToLocal3d(GeoCoord((float)geoCoordPlus.x(), (float)geoCoordPlus.y()));
    return { localPt.x(),localPt.y(),geoCoordPlus.z() };
}

bool FlatEarthCoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    const auto other = dynamic_cast<const FlatEarthCoordSystem *>(coordSys);
    return other && other->origin == origin && GeoCoordSystem::isSameAs(coordSys);
}


PassThroughCoordSystem::PassThroughCoordSystem(const PassThroughCoordSystem& other) :
    CoordSystem(other)
{
}

CoordSystemRef PassThroughCoordSystem::clone() const
{
    return std::make_shared<PassThroughCoordSystem>(*this);
}

GeoCoord PassThroughCoordSystem::localToGeographic(const Point3f &pt) const
{
    return { pt.x(), pt.y() };
}

GeoCoord PassThroughCoordSystem::localToGeographic(const Point3d &pt) const
{
    return { (float)pt.x(), (float)pt.y() };
}

Point2d PassThroughCoordSystem::localToGeographicD(const Point3d &pt) const
{
    return { pt.x(), pt.y() };
}

Point3f PassThroughCoordSystem::geographicToLocal(const GeoCoord &geo) const
{
    return { geo.lon(), geo.lat(), 0.0f };
}

Point3d PassThroughCoordSystem::geographicToLocal3d(const GeoCoord &geo) const
{
    return { geo.lon(), geo.lat(), 0.0 };
}

Point3d PassThroughCoordSystem::geographicToLocal(const Point2d &geo) const
{
    return { geo.x(), geo.y(), 0.0 };
}

Point2d PassThroughCoordSystem::geographicToLocal2(const Point2d &geo) const
{
    return { geo.x(), geo.y() };
}

bool PassThroughCoordSystem::isSameAs(const CoordSystem *coordSys) const
{
    const auto other = dynamic_cast<const PassThroughCoordSystem *>(coordSys);
    return (other != nullptr);
}

