/*  CoordSystem.h
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

#import <memory>
#import "WhirlyGeometry.h"
#import "WhirlyVector.h"

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

struct CoordSystem;
using CoordSystemRef = std::shared_ptr<CoordSystem>;

/// Base class for the various coordinate systems
///  we use in the toolkits.
/*abstract*/ struct CoordSystem : public DelayedDeletable
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    virtual ~CoordSystem() = default;

    /// If anything went wrong during construction, this will return false
    virtual bool isValid() const = 0;

    /// Create a new instance equivalent to this one
    virtual CoordSystemRef clone() const = 0;

    /// Convert from the local coordinate system to lat/lon
    virtual GeoCoord localToGeographic(const Point3f&) const = 0;
    virtual Point2d localToGeographicD(const Point3d&) const = 0;
    virtual GeoCoord localToGeographic(const Point3d&) const = 0;
    /// Convert from lat/lon t the local coordinate system
    virtual Point3f geographicToLocal(const GeoCoord&) const = 0;
    virtual Point3d geographicToLocal(const Point2d&) const = 0;
    virtual Point2d geographicToLocal2(const Point2d&) const = 0;
    virtual Point3d geographicToLocal3d(const GeoCoord&) const = 0;

    /// Convert from the local coordinate system to geocentric
    virtual Point3f localToGeocentric(const Point3f&) const = 0;
    virtual Point3d localToGeocentric(const Point3d&) const = 0;
    
    /// Convert from display coordinates to geocentric
    virtual Point3f geocentricToLocal(const Point3f&) const = 0;
    virtual Point3d geocentricToLocal(const Point3d&) const = 0;
    
    /// Return true if the given coordinate system is the same as the one passed in
    virtual bool isSameAs(const CoordSystem *coordSys) const = 0;

    // Deprecated as ambiguous, use getBoundsGeo
    GeoMbr getBounds() const { return GeoMbr(bounds); }
    // Deprecated as ambiguous, use getBoundsGeo
    const MbrD &getBoundsD() const { return bounds; }

    // Get the bounding box where this system is valid, in geographic coordinates
    const MbrD &getBoundsGeo() const { return bounds; }

    // Get the bounding box where this system is valid, in local coordinates
    MbrD getBoundsLocal() const;

    template <typename T> void setBounds(T mbr) { bounds = mbr; }
    template <typename T> void setBounds(T ll, T ur) { bounds = { ll, ur }; }

    virtual Point3d getWrapCoords() const { return { 0, 0, 0 }; }
    virtual bool canBeWrapped() const { return canWrap; }
    virtual void setCanBeWrapped(bool b) { canWrap = b; }

protected:
    CoordSystem() = default;
    CoordSystem(const CoordSystem&) = default;

    MbrD bounds = { { -M_PI, -M_PI_2 }, { M_PI, M_PI_2 } };
    bool canWrap = false;
};
    
/// Convert a point from one coordinate system to another
Point3f CoordSystemConvert(const CoordSystem *inSystem,const CoordSystem *outSystem,const Point3f &inCoord);
Point3d CoordSystemConvert3d(const CoordSystem *inSystem,const CoordSystem *outSystem,const Point3d &inCoord);

struct CoordSystemDisplayAdapter;
typedef std::shared_ptr<CoordSystemDisplayAdapter> CoordSystemDisplayAdapterRef;

/** The Coordinate System Display Adapter handles the task of
    converting coordinates in the native system to data values we
    can display.
 */
/*abstract*/ struct CoordSystemDisplayAdapter : public DelayedDeletable
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    // Does not take ownership of `coordSys`, caller must manage object lifetimes
    CoordSystemDisplayAdapter(const CoordSystem *coordSys, const Point3d &center);
    // Shared ownership of `coordSys`, caller must manage object lifetimes
    CoordSystemDisplayAdapter(const CoordSystemDisplayAdapter &);
    virtual ~CoordSystemDisplayAdapter() = default;

    // Make a copy of this object, whatever it is
    virtual CoordSystemDisplayAdapterRef clone() const = 0;

    // Make a copy, but one using a different coordinate system reference.
    virtual CoordSystemDisplayAdapterRef cloneWithCoordSys(CoordSystem *coordSys) const;

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
    virtual Point3f localToDisplay(const Point3f&) const = 0;
    virtual Point3d localToDisplay(const Point3d&) const = 0;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual Point3f displayToLocal(const Point3f&) const = 0;
    virtual Point3d displayToLocal(const Point3d&) const = 0;
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(const Point3f&) const = 0;
    virtual Point3d normalForLocal(const Point3d&) const = 0;

    /// Get a reference to the coordinate system
    virtual const CoordSystem *getCoordSystem() const { return coordSys; }
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
#if !MAPLY_MINIMAL
    virtual bool isFlat() const = 0;
#else
    // Allow the compiler to discard `if(!...->isFlat())` sections
    bool isFlat() const { return true; }
#endif //!MAPLY_MINIMAL

protected:
    Point3d center;
    Point3d scale = { 1, 1, 1 };
    const CoordSystem *coordSys;
};


/** The general coord system display adapter is used by flat maps to encapsulate a general coordinate system.
    This needs to be one which is flat, but is otherwise unconstrained.  The bounding box is where the coordinate system is valid and the center will be the center of display coordinates.
  */
struct GeneralCoordSystemDisplayAdapter : public CoordSystemDisplayAdapter
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    GeneralCoordSystemDisplayAdapter(CoordSystem *coordSys,
                                     const Point3d &ll,const Point3d &ur,
                                     const Point3d &center,const Point3d &scale);
    GeneralCoordSystemDisplayAdapter(const GeneralCoordSystemDisplayAdapter &);

    virtual CoordSystemDisplayAdapterRef clone() const override;

    /// Bounding box where the coordinate system is valid
    virtual bool getBounds(Point3f &ll,Point3f &ur) const override;

    /// Return the valid area of the source coordinate system in display coordinates
    virtual bool getDisplayBounds(Point3d &ll,Point3d &ur) const override;
    
    /// Return the valid area of the coordinate system in lon/lat radians
    virtual bool getGeoBounds(Point2d &ll,Point2d &ur) const override;

    /// Convert from the system's local coordinates to display coordinates
    virtual Point3f localToDisplay(const Point3f&) const override;
    virtual Point3d localToDisplay(const Point3d&) const override;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual Point3f displayToLocal(const Point3f&) const override;
    virtual Point3d displayToLocal(const Point3d&) const override;
    
    /// For flat systems the normal is Z up.
    virtual Point3f normalForLocal(const Point3f&) const override { return {0,0,1 }; }
    virtual Point3d normalForLocal(const Point3d&) const override { return {0,0,1 }; }

    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
#if !MAPLY_MINIMAL
    virtual bool isFlat() const override { return true; }
#endif //!MAPLY_MINIMAL

protected:
    Point3d ll,ur;
    Point3d dispLL,dispUR;
    Point2d geoLL,geoUR;
};

typedef std::shared_ptr<GeneralCoordSystemDisplayAdapter> GeneralCoordSystemDisplayAdapterRef;

}
