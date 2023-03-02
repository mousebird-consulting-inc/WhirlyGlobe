/*  SphericalMercator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/19/12.
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
#import "GlobeMath.h"

namespace WhirlyKit 
{

struct SphericalMercatorCoordSystem;
using SphericalMercatorCoordSystemRef = std::shared_ptr<SphericalMercatorCoordSystem>;

/** The Mercator Projection, bane of cartographers everywhere.
 It stretches out the world in a familiar way, making the US
 look almost as big as our collective ego.  And Greenland.  For some reason.
 */
struct SphericalMercatorCoordSystem : public GeoCoordSystem
{
    /// Construct with an optional origin for the projection in radians
    /// The equator is default
    SphericalMercatorCoordSystem(double originLon = 0.0);
    SphericalMercatorCoordSystem(const SphericalMercatorCoordSystem &);

    static SphericalMercatorCoordSystemRef makeWebStandard();

    virtual bool isValid() const override;

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

    virtual Point3d getWrapCoords() const override { return { M_PI, 0, 0 }; }

protected:
    double originLon;
};

/** The Spherical Mercator Display adapter uses an origin and geo MBR
    to convert coordinates in and out of display space.
  */
struct SphericalMercatorDisplayAdapter : public CoordSystemDisplayAdapter
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct with the origin longitude for the projection and the
    ///  bounding box for the constraints.  The latter are in geographic (WGS84)
    SphericalMercatorDisplayAdapter(float originLon, const GeoCoord &ll, const GeoCoord &ur);
    
    /// Construct with the origin longitude for the projection and the
    ///  bounding box for the constraints.  The latter are in geographic (WGS84).
    /// Also pass in a display origin.  We'll offset everything from there.
    SphericalMercatorDisplayAdapter(float originLon, const GeoCoord &ll, const GeoCoord &ur,
                                    const Point3d &displayOrigin);

    SphericalMercatorDisplayAdapter(const SphericalMercatorDisplayAdapter &);
    
    virtual CoordSystemDisplayAdapterRef clone() const override;

    /// Return the valid boundary in spherical mercator.  Z coordinate is ignored at present.
    virtual bool getBounds(Point3f &ll,Point3f &ur) const override;

    /// Return the bounds of the display in geo coordinates
    virtual bool getGeoBounds(Point2d &outll, Point2d &outur) const override {
        outll = geoLL;
        outur = geoUR;
        return true;
    }

    /// Convert from the system's local coordinates to display coordinates
    virtual Point3f localToDisplay(const Point3f&) const override;
    virtual Point3d localToDisplay(const Point3d&) const override;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual Point3f displayToLocal(const Point3f&) const override;
    virtual Point3d displayToLocal(const Point3d&) const override;
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(const Point3f&) const override { return {0,0,1 }; }
    virtual Point3d normalForLocal(const Point3d&) const override { return {0,0,1 }; }

    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
#if !MAPLY_MINIMAL
    virtual bool isFlat() const override { return true; }
#endif //!MAPLY_MINIMAL

protected:
    Point2d org,ll,ur,geoLL,geoUR;
    Point3d displayOrigin;
    SphericalMercatorCoordSystem smCoordSys;
};
    
}
