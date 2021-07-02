/*
 *  Proj4CoordSystem.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/10/15.
 *  Copyright 2011-2019 mousebird consulting
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct with a proj4 string to be passsed to proj.4 (duh)
    Proj4CoordSystem(const std::string &proj4Str);
    virtual ~Proj4CoordSystem();
    
    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(Point3f) const override;
    virtual GeoCoord localToGeographic(Point3d) const override;
    virtual Point2d localToGeographicD(Point3d) const override;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(GeoCoord) const override;
    virtual Point3d geographicToLocal3d(GeoCoord) const override;
    virtual Point3d geographicToLocal(Point2d) const override;
    
    /// Convert from the local coordinate system to geocentric
    virtual Point3f localToGeocentric(Point3f) const override;
    virtual Point3d localToGeocentric(Point3d) const override;
    /// Convert from display coordinates to geocentric
    virtual Point3f geocentricToLocal(Point3f) const override;
    virtual Point3d geocentricToLocal(Point3d) const override;
    
    /// True if the other system is Spherical Mercator with the same origin
    virtual bool isSameAs(CoordSystem *coordSys) const override;
    
    /// Check that it actually created the pj structures
    bool isValid() const { return pj != nullptr; }
    
protected:
    void *pj;
    void *pj_latlon,*pj_geocentric;
    std::string proj4Str;
};
    
}
