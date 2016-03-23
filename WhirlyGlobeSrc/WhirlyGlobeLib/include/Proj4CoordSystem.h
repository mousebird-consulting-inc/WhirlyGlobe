/*
 *  Proj4CoordSystem.h
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/** The proj4 coord system object wraps a proj.4 implemented coordinate system.
  */
class Proj4CoordSystem : public CoordSystem
{
public:
    /// Construct with a proj4 string to be passsed to proj.4 (duh)
    Proj4CoordSystem(const std::string &proj4Str);
    
    /// Convert from the local coordinate system to lat/lon
    GeoCoord localToGeographic(Point3f);
    GeoCoord localToGeographic(Point3d);
    Point2d localToGeographicD(Point3d);
    /// Convert from lat/lon t the local coordinate system
    Point3f geographicToLocal(GeoCoord);
    Point3d geographicToLocal3d(GeoCoord);
    Point3d geographicToLocal(Point2d);
    
    /// Convert from the local coordinate system to geocentric
    Point3f localToGeocentric(Point3f);
    Point3d localToGeocentric(Point3d);
    /// Convert from display coordinates to geocentric
    Point3f geocentricToLocal(Point3f);
    Point3d geocentricToLocal(Point3d);
    
    /// True if the other system is Spherical Mercator with the same origin
    virtual bool isSameAs(CoordSystem *coordSys);
    
    /// Check that it actually created the pj structures
    bool isValid();
    
protected:
    void *pj;
    void *pj_latlon,*pj_geocentric;
    std::string proj4Str;
};
    
}
