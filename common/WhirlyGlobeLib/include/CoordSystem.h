/*  CoordSystem.h
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

#import <memory>
#import "WhirlyGeometry.h"

namespace WhirlyKit
{

/// Degree to radians conversion
template<typename T>
constexpr T DegToRad(T deg) { return deg / 180.0 * (T)M_PI; }

/// Radians to degress
template<typename T>
constexpr T RadToDeg(T rad) { return rad / (T)M_PI * 180.0; }


/// This class give us a virtual destructor to make use of
///  when we're deleting random objects at the end of the layer thread.
class DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    virtual ~DelayedDeletable() = default;
};

typedef std::shared_ptr<DelayedDeletable> DelayedDeletableRef;

/// Base class for the various coordinate systems
///  we use in the toolkits.
class CoordSystem : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    CoordSystem() = default;
    virtual ~CoordSystem() = default;
    
    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(Point3f) const = 0;
    virtual Point2d localToGeographicD(Point3d) const = 0;
    virtual GeoCoord localToGeographic(Point3d) const = 0;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(GeoCoord) const = 0;
    virtual Point3d geographicToLocal(Point2d) const = 0;
    virtual Point3d geographicToLocal3d(GeoCoord) const = 0;

    /// Convert from the local coordinate system to geocentric
    virtual Point3f localToGeocentric(Point3f) const = 0;
    virtual Point3d localToGeocentric(Point3d) const = 0;
    
    /// Convert from display coordinates to geocentric
    virtual Point3f geocentricToLocal(Point3f) const = 0;
    virtual Point3d geocentricToLocal(Point3d) const = 0;
    
    /// Return true if the given coordinate system is the same as the one passed in
    virtual bool isSameAs(CoordSystem *coordSys) const { return false; }
};
    
typedef std::shared_ptr<CoordSystem> CoordSystemRef;
    
/// Convert a point from one coordinate system to another
Point3f CoordSystemConvert(CoordSystem *inSystem,CoordSystem *outSystem,Point3f inCoord);
Point3d CoordSystemConvert3d(CoordSystem *inSystem,CoordSystem *outSystem,Point3d inCoord);
    
/** The Coordinate System Display Adapter handles the task of
    converting coordinates in the native system to data values we
    can display.
 */
class CoordSystemDisplayAdapter : public DelayedDeletable
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    CoordSystemDisplayAdapter(CoordSystem *coordSys,Point3d center) : coordSys(coordSys), center(0.0,0.0,0.0), scale(1.0,1.0,1.0) { }
    virtual ~CoordSystemDisplayAdapter() = default;
    
    /// If the subclass can support a bounding box, this returns true
    ///  and the bounds.  Z values are ignored for now.
    /// If the subclass can't support bounds (e.g. a globe), you get false back.
    virtual bool getBounds(Point3f &ll,Point3f &ur) const = 0;

    /// Return the bounds in display
    virtual bool getDisplayBounds(Point3d &ll,Point3d &ur) const {
        Point3f llf,urf;
        if (getBounds(llf, urf)) {
            ll = localToDisplay(Point3d(llf.x(), llf.y(), llf.z()));
            ur = localToDisplay(Point3d(urf.x(), urf.y(), urf.z()));
            return true;
        }
        return false;
    }
    
    /// Return the bounds of the display in geo coordinates
    virtual bool getGeoBounds(Point2d &ll,Point2d &ur) const {
        Point3f llf,urf;
        if (coordSys && getBounds(llf, urf)) {
            ll = coordSys->localToGeographicD(Point3d(llf.x(), llf.y(), llf.z()));
            ur = coordSys->localToGeographicD(Point3d(urf.x(), urf.y(), urf.z()));
            return true;
        }
        return false;
    }

    /// Return the current center
    Point3d getCenter() const { return center; }
    
    /// Set the scale for coordinates going to/from display space
    void setScale(const Point3d &inScale) { scale = inScale; }
    
    /// Return the display space scale
    Point3d getScale() const { return scale; }

    /// Convert from the system's local coordinates to display coordinates
    virtual Point3f localToDisplay(Point3f) const = 0;
    virtual Point3d localToDisplay(Point3d) const = 0;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual Point3f displayToLocal(Point3f) const = 0;
    virtual Point3d displayToLocal(Point3d) const = 0;
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(Point3f) const = 0;
    virtual Point3d normalForLocal(Point3d) const = 0;

    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() const = 0;
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() const = 0;
    
protected:
    Point3d center;
    Point3d scale;
    CoordSystem *coordSys;
};

typedef std::shared_ptr<CoordSystemDisplayAdapter> CoordSystemDisplayAdapterRef;

/** The general coord system display adapter is used by flat maps to encapsulate a general coordinate system.
    This needs to be one which is flat, but is otherwise unconstrained.  The bounding box is where the coordinate system is valid and the center will be the center of display coordinates.
  */
class GeneralCoordSystemDisplayAdapter : public CoordSystemDisplayAdapter
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    GeneralCoordSystemDisplayAdapter(CoordSystem *coordSys,const Point3d &ll,const Point3d &ur,const Point3d &center,const Point3d &scale);

    /// Bounding box where the coordinate system is valid
    virtual bool getBounds(Point3f &ll,Point3f &ur) const override;

    /// Return the valid area of the source coordinate system in display coordinates
    virtual bool getDisplayBounds(Point3d &ll,Point3d &ur) const override;
    
    /// Return the valid area of the coordinate system in lon/lat radians
    virtual bool getGeoBounds(Point2d &ll,Point2d &ur) const override;

    /// Convert from the system's local coordinates to display coordinates
    virtual Point3f localToDisplay(Point3f) const override;
    virtual Point3d localToDisplay(Point3d) const override;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual Point3f displayToLocal(Point3f) const override;
    virtual Point3d displayToLocal(Point3d) const override;
    
    /// For flat systems the normal is Z up.
    virtual Point3f normalForLocal(Point3f) const override { return Point3f(0,0,1); }
    virtual Point3d normalForLocal(Point3d) const override { return Point3d(0,0,1); }
    
    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() const override { return coordSys; }
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() const override { return true; }
    
protected:
    Point3d ll,ur;
    Point3d dispLL,dispUR;
    Point2d geoLL,geoUR;
    CoordSystem *coordSys;
};

typedef std::shared_ptr<GeneralCoordSystemDisplayAdapter> GeneralCoordSystemDisplayAdapterRef;

}
