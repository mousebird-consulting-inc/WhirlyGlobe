/*
 *  FlatMath.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/10/12.
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

#import "FlatMath.h"

using namespace WhirlyGlobe;

namespace WhirlyMap
{
    
Point3f FlatCoordSystem::pointFromGeo(GeoCoord geo)
{
    return Point3f(geo.lon(),geo.lat(),0.0);
}

GeoCoord FlatCoordSystem::geoFromPoint(Point3f pt)
{
    return GeoCoord(pt.x(),pt.y());
}

MercatorCoordSystem::MercatorCoordSystem(float originLon)
    : originLon(originLon)
{
}

// Keep things right below the poles
const float PoleLimit = DegToRad(85.05113);
    
Point3f MercatorCoordSystem::pointFromGeo(GeoCoord geo)
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

GeoCoord MercatorCoordSystem::geoFromPoint(Point3f pt)
{
    GeoCoord coord;
    coord.lon() = pt.x() + originLon;
    coord.lat() = atanf(sinhf(pt.y()));
    
    return coord;
}

}