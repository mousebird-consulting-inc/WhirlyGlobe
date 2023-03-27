/*  Proj4CoordSystem.h
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

#import "WhirlyVector.h"
#import "CoordSystem.h"

namespace WhirlyKit
{

/** The proj4 coord system object wraps a proj.4 implemented coordinate system.
  */
struct Proj4CoordSystem : public CoordSystem
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct with a proj4 string to be passed to proj.4 (duh)
    Proj4CoordSystem(std::string proj4Str);
    Proj4CoordSystem(const Proj4CoordSystem &);
    Proj4CoordSystem(Proj4CoordSystem &&);
    virtual ~Proj4CoordSystem();
    
    /// Create a new instance equivalent to this one
    virtual CoordSystemRef clone() const override;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(const Point3f&) const override;
    virtual GeoCoord localToGeographic(const Point3d&) const override;
    virtual Point2d localToGeographicD(const Point3d&) const override;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(const GeoCoord&) const override;
    virtual Point3d geographicToLocal3d(const GeoCoord&) const override;
    virtual Point3d geographicToLocal(const Point2d&) const override;
    virtual Point2d geographicToLocal2(const Point2d&) const override;

    /// Convert from the local coordinate system to geocentric
    virtual Point3f localToGeocentric(const Point3f&) const override;
    virtual Point3d localToGeocentric(const Point3d&) const override;
    /// Convert from display coordinates to geocentric
    virtual Point3f geocentricToLocal(const Point3f&) const override;
    virtual Point3d geocentricToLocal(const Point3d&) const override;
    
    /// True if the other system is Spherical Mercator with the same origin
    virtual bool isSameAs(const CoordSystem *coordSys) const override;
    
    /// Check that it actually created the pj structures
    bool isValid() const override;
    
private:
    void init();

protected:
    void *pj = nullptr;
    void *pj_ctx = nullptr;
    void *pj_latlon = nullptr;
    void *pj_latlon_ctx = nullptr;
    void *pj_geocentric = nullptr;
    void *pj_geocentric_ctx = nullptr;
    std::string proj4Str;
    mutable std::mutex mutex;
};
    
}
