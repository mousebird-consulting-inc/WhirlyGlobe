/*
 *  GlobeMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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


#import "GlobeMath.h"

using namespace WhirlyKit;

namespace WhirlyKit
{
    
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

Point3f GeoCoordSystem::localToGeocentricish(Point3f geoPt)
{
    return LocalToGeocentricish(geoPt);
}

    
/// Convert from the local coordinate system to display coordinates (geocentric-ish)
Point3f GeoCoordSystem::LocalToGeocentricish(Point3f geoPt)
{
	float z = sinf(geoPt.y());
	float rad = sqrtf(1.0-z*z);
	Point3f pt(rad*cosf(geoPt.x()),rad*sinf(geoPt.x()),z);
	return pt;
}
    
Point3f GeoCoordSystem::LocalToGeocentricish(GeoCoord geoCoord)
{
    return LocalToGeocentricish(Point3f(geoCoord.x(),geoCoord.y(),0.0));
}    

Point3f GeoCoordSystem::geocentricishToLocal(Point3f pt)
{
    return GeocentricishToLocal(pt);
}
    
Point3f GeoCoordSystem::GeocentricishToLocal(Point3f pt)
{
    GeoCoord geoCoord;
    geoCoord.lat() = asinf(pt.z());
    float rad = sqrtf(1.0-pt.z()*pt.z());
    geoCoord.lon() = acosf(pt.x() / rad);
    if (pt.y() < 0)  geoCoord.lon() *= -1;
    
    return Point3f(geoCoord.lon(),geoCoord.lat(),0.0);        
}
    
GeoCoord GeoCoordSystem::GeocentricishToGeoCoord(Point3f pt)
{
    Point3f geo3d = GeocentricishToLocal(pt);
    return GeoCoord(geo3d.x(),geo3d.y());
}
	
}
