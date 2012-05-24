/*
 *  GlobeMath.h
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/** Geographic coordinate system represents a coordinate system that uses
    lat/lon/elevation.
  */
class GeoCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    
    /// Convert from the local coordinate system to display coordinates (geocentric-ish)
    Point3f localToGeocentricish(Point3f);
    
    /// Static so other coordinate systems can use it
    static Point3f LocalToGeocentricish(Point3f);
    static Point3f LocalToGeocentricish(GeoCoord);

    /// Convert from display coordinates to the local coordinate system
    Point3f geocentricishToLocal(Point3f);
    
    /// Static so other coordinate systems can use it
    static Point3f GeocentricishToLocal(Point3f);
    static GeoCoord GeocentricishToGeoCoord(Point3f);
    /// Convenience routine to convert a whole MBR to local coordinates
    static Mbr GeographicMbrToLocal(GeoMbr);

    /// Not flat
    bool isFlat() { return false; }
};

}
