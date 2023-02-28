/*  FlatMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
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
    
/** The Plate Careee just unrolls lat/lon in radians and
    represents the map as a flat non-projection of that.
    This is plate carree: http://en.wikipedia.org/wiki/Equirectangular_projection
  */
struct PlateCarreeCoordSystem : public GeoCoordSystem
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    PlateCarreeCoordSystem() = default;
    PlateCarreeCoordSystem(const PlateCarreeCoordSystem&);

    virtual bool isValid() const override;

    virtual CoordSystemRef clone() const override;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(const Point3f &p) const override { return { p.x(), p.y() }; }
    virtual GeoCoord localToGeographic(const Point3d &p) const override { return { (float)p.x(), (float)p.y() }; }
    virtual Point2d localToGeographicD(const Point3d &p) const override { return { p.x(), p.y() }; }
    
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(const GeoCoord &c) const override { return {c.lon(),c.lat(),0.0}; }
    virtual Point3d geographicToLocal3d(const GeoCoord &c) const override { return {c.lon(),c.lat(),0.0}; }
    virtual Point3d geographicToLocal(const Point2d &c) const override { return {c.x(),c.y(),0.0}; }
    virtual Point2d geographicToLocal2(const Point2d &c) const override { return {c.x(),c.y()}; }

    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(const Point3f&) const override;
    virtual Point3d localToGeocentric(const Point3d&) const override;
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(const Point3f&) const override;
    virtual Point3d geocentricToLocal(const Point3d&) const override;
        
    /// Return true if the other coordinate system is also Plate Carree
    virtual bool isSameAs(const CoordSystem *coordSys) const override;
};
    
/** Flat Earth refers to the MultiGen flat earth coordinate system.
    This is a scaled unrolling from a center point.
 */
struct FlatEarthCoordSystem : public GeoCoordSystem
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    FlatEarthCoordSystem(const GeoCoordD &origin);
    FlatEarthCoordSystem(const FlatEarthCoordSystem&);

    virtual bool isValid() const override;

    virtual CoordSystemRef clone() const override;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(const Point3f&) const override;
    virtual GeoCoord localToGeographic(const Point3d&) const override;
    virtual Point2d localToGeographicD(const Point3d&) const override;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(const GeoCoord&) const override;
    virtual Point3d geographicToLocal3d(const GeoCoord&) const override;
    virtual Point2d geographicToLocal2(const Point2d&) const override;
    virtual Point3d geographicToLocal(const Point2d&) const override;

    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(const Point3f&) const override;
    virtual Point3d localToGeocentric(const Point3d&) const override;
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(const Point3f&) const override;
    virtual Point3d geocentricToLocal(const Point3d&) const override;
    
    /// Return true if the other coordinate system is Flat Earth with the same origin
    virtual bool isSameAs(const CoordSystem *coordSys) const override;

    /// Return the origin
    GeoCoordD getOrigin() const { return origin; }

protected:
    GeoCoordD origin;
    double converge;
};
    
/// A representative earth radius value.  We use this for scaling, not accurate geolocation.
static constexpr float EarthRadius = 6371000;

/** This coord system just passes through output values as inputs.
    It's useful for an offline renderer.
  */
struct PassThroughCoordSystem : public CoordSystem
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    PassThroughCoordSystem() = default;
    PassThroughCoordSystem(const PassThroughCoordSystem &);

    virtual bool isValid() const override { return true; }

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

    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(const Point3f &p) const override { return p; }
    virtual Point3d localToGeocentric(const Point3d &p) const override { return p; }
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(const Point3f &p) const override { return p; }
    virtual Point3d geocentricToLocal(const Point3d &p) const override { return p; }
    
    /// Return true if the other coordinate system is Flat Earth with the same origin
    virtual bool isSameAs(const CoordSystem *coordSys) const override;
};

typedef std::shared_ptr<PassThroughCoordSystem> PassThroughCoordSystemRef;

}
