/*
 *  Proj4CoordSystem.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/10/15.
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

#import "WhirlyKitLog.h"
#import "Proj4CoordSystem.h"
#import "GlobeMath.h"
#import "proj_api.h"

namespace WhirlyKit
{

Proj4CoordSystem::Proj4CoordSystem(const std::string &proj4Str)
: proj4Str(proj4Str)
{
    pj = pj_init_plus(proj4Str.c_str());
    pj_latlon = pj_init_plus("+proj=latlong +datum=WGS84");
    pj_geocentric = pj_init_plus("+proj=geocent +datum=WGS84");
}
    
Proj4CoordSystem::~Proj4CoordSystem()
{
    pj_free(pj);
    pj_free(pj_latlon);
    pj_free(pj_geocentric);
}
    
bool Proj4CoordSystem::isValid()
{
    return pj != NULL;
}

/// Convert from the local coordinate system to lat/lon
GeoCoord Proj4CoordSystem::localToGeographic(Point3f pt)
{
    GeoCoord coord;
    double x = pt.x(),y = pt.y(),z = pt.z();
    pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
    coord.x() = x;
    coord.y() = y;
    
    return coord;
}

GeoCoord Proj4CoordSystem::localToGeographic(Point3d pt)
{
    GeoCoord coord;
    double x = pt.x(),y = pt.y(),z = pt.z();
    pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
    coord.x() = x;
    coord.y() = y;
    
    return coord;
}

Point2d Proj4CoordSystem::localToGeographicD(Point3d pt)
{
    Point2d coord;
    double x = pt.x(),y = pt.y(),z = pt.z();
    pj_transform(pj, pj_latlon, 1, 1, &x, &y, &z);
    coord.x() = x;
    coord.y() = y;
    
    return coord;
}

/// Convert from lat/lon t the local coordinate system
Point3f Proj4CoordSystem::geographicToLocal(GeoCoord geo)
{
    Point3f coord;
    double x = geo.x(),y = geo.y(),z = 0.0;
    pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

Point3d Proj4CoordSystem::geographicToLocal3d(GeoCoord geo)
{
    Point3d coord;
    double x = geo.x(),y = geo.y(),z = 0.0;
    if (pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z))
        WHIRLYKIT_LOGV("Proj4CoordSystem::geographicToLocal3d error converting to local");

    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

Point3d Proj4CoordSystem::geographicToLocal(Point2d geo)
{
    Point3d coord;
    double x = geo.x(),y = geo.y(),z = 0.0;
    pj_transform(pj_latlon, pj, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

/// Convert from the local coordinate system to geocentric
Point3f Proj4CoordSystem::localToGeocentric(Point3f localPt)
{
    Point3f coord;
    double x = localPt.x(),y = localPt.y(),z = localPt.z();
    pj_transform(pj, pj_geocentric, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

Point3d Proj4CoordSystem::localToGeocentric(Point3d localPt)
{
    Point3d coord;
    double x = localPt.x(),y = localPt.y(),z = localPt.z();
    pj_transform(pj, pj_geocentric, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

/// Convert from display coordinates to geocentric
Point3f Proj4CoordSystem::geocentricToLocal(Point3f geocPt)
{
    Point3f coord;
    double x = geocPt.x(),y = geocPt.y(),z = geocPt.z();
    pj_transform(pj_geocentric, pj, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

Point3d Proj4CoordSystem::geocentricToLocal(Point3d geocPt)
{
    Point3d coord;
    double x = geocPt.x(),y = geocPt.y(),z = geocPt.z();
    pj_transform(pj_geocentric, pj, 1, 1, &x, &y, &z);
    coord.x() = x;  coord.y() = y;  coord.z() = z;
    
    return coord;
}

bool Proj4CoordSystem::isSameAs(CoordSystem *coordSys)
{
    Proj4CoordSystem *other = dynamic_cast<Proj4CoordSystem *>(coordSys);
    if (!other)
        return false;
    
    return proj4Str == other->proj4Str;
}
    
}
