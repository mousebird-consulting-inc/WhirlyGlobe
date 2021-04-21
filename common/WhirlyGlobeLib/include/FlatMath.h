/*  FlatMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2021 mousebird consulting
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
    
/** The Plate Careee just unrolls lat/lon in radians and
    represents the map as a flat non-projection of that.
    This is plate carree: http://en.wikipedia.org/wiki/Equirectangular_projection
  */
class PlateCarreeCoordSystem : public CoordSystem
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(Point3f p) const override { return GeoCoord(p.x(),p.y()); }
    virtual GeoCoord localToGeographic(Point3d p) const override { return GeoCoord(p.x(),p.y()); }
    virtual Point2d localToGeographicD(Point3d p) const override { return Point2d(p.x(),p.y()); }
    
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(GeoCoord c) const override { return Point3f(c.lon(),c.lat(),0.0); }
    virtual Point3d geographicToLocal3d(GeoCoord c) const override { return Point3d(c.lon(),c.lat(),0.0); }
    virtual Point3d geographicToLocal(Point2d c) const override { return Point3d(c.x(),c.y(),0.0); }

    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(Point3f) const override;
    virtual Point3d localToGeocentric(Point3d) const override;
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(Point3f) const override;
    virtual Point3d geocentricToLocal(Point3d) const override;
        
    /// Return true if the other coordinate system is also Plate Carree
    virtual bool isSameAs(CoordSystem *coordSys) const override;
};
    
/** Flat Earth refers to the MultiGen flat earth coordinate system.
    This is a scaled unrolling from a center point.
 */
class FlatEarthCoordSystem : public CoordSystem
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    FlatEarthCoordSystem(const GeoCoord &origin);

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(Point3f) const override;
    virtual GeoCoord localToGeographic(Point3d) const override;
    virtual Point2d localToGeographicD(Point3d) const override;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(GeoCoord) const override;
    virtual Point3d geographicToLocal3d(GeoCoord) const override;
    virtual Point3d geographicToLocal(Point2d) const override;
    
    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(Point3f) const override;
    virtual Point3d localToGeocentric(Point3d) const override;
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(Point3f) const override;
    virtual Point3d geocentricToLocal(Point3d) const override;
    
    /// Return true if the other coordinate system is Flat Earth with the same origin
    virtual bool isSameAs(CoordSystem *coordSys) const override;

    /// Return the origin
    GeoCoord getOrigin() const { return origin; }
    
protected:
    GeoCoord origin;
    float converge;
};
    
/// A representative earth radius value.  We use this for scaling, not accurate geolocation.
static constexpr float EarthRadius = 6371000;

/** This coord system just passes through output values as inputs.
    It's useful for an offline renderer.
  */
class PassThroughCoordSystem : public CoordSystem
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    PassThroughCoordSystem() = default;
    
    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(Point3f) const override;
    virtual GeoCoord localToGeographic(Point3d) const override;
    virtual Point2d localToGeographicD(Point3d) const override;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(GeoCoord) const override;
    virtual Point3d geographicToLocal3d(GeoCoord) const override;
    virtual Point3d geographicToLocal(Point2d) const override;
    
    /// Convert from local coordinates to WGS84 geocentric
    virtual Point3f localToGeocentric(Point3f p) const override { return p; }
    virtual Point3d localToGeocentric(Point3d p) const override { return p; }
    /// Convert from WGS84 geocentric to local coordinates
    virtual Point3f geocentricToLocal(Point3f p) const override { return p; }
    virtual Point3d geocentricToLocal(Point3d p) const override { return p; }
    
    /// Return true if the other coordinate system is Flat Earth with the same origin
    virtual bool isSameAs(CoordSystem *coordSys) const override;
};

typedef std::shared_ptr<PassThroughCoordSystem> PassThroughCoordSystemRef;

}
