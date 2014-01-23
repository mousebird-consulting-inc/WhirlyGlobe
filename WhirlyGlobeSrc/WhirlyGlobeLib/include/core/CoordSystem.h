/*
 *  CoordSystem.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import <boost/shared_ptr.hpp>
#import "WhirlyGeometry.h"

namespace WhirlyKit
{
    
/// Degree to radians conversion
template<typename T>
T DegToRad(T deg) { return deg / 180.0 * (T)M_PI; }

/// Radians to degress
template<typename T>
T RadToDeg(T rad) { return rad / (T)M_PI * 180.0; }
    
/// This class give us a virtual destructor to make use of
///  when we're deleting random objects at the end of the layer thread.
class DelayedDeletable
{
public:
    virtual ~DelayedDeletable() { }
};
    
typedef boost::shared_ptr<DelayedDeletable> DelayedDeletableRef;

/// Base class for the various coordinate systems
///  we use in the toolkits.
class CoordSystem : public DelayedDeletable
{
public:
    CoordSystem() { }
    virtual ~CoordSystem() { }
    
    /// Convert from the local coordinate system to lat/lon
    virtual WhirlyKit::GeoCoord localToGeographic(WhirlyKit::Point3f) = 0;
    virtual WhirlyKit::GeoCoord localToGeographic(WhirlyKit::Point3d) = 0;
    /// Convert from lat/lon t the local coordinate system
    virtual WhirlyKit::Point3f geographicToLocal(WhirlyKit::GeoCoord) = 0;
    virtual WhirlyKit::Point3d geographicToLocal3d(WhirlyKit::GeoCoord) = 0;

    /// Convert from the local coordinate system to geocentric
    virtual WhirlyKit::Point3f localToGeocentric(WhirlyKit::Point3f) = 0;
    virtual WhirlyKit::Point3d localToGeocentric(WhirlyKit::Point3d) = 0;
    
    /// Convert from display coordinates to geocentric
    virtual WhirlyKit::Point3f geocentricToLocal(WhirlyKit::Point3f) = 0;
    virtual WhirlyKit::Point3d geocentricToLocal(WhirlyKit::Point3d) = 0;
    
    /// Return true if the given coordinate system is the same as the one passed in
    virtual bool isSameAs(CoordSystem *coordSys) { return false; }
};
    
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
    CoordSystemDisplayAdapter(CoordSystem *coordSys,Point3d center) : coordSys(coordSys), center(0.0,0.0,0.0) { }
    virtual ~CoordSystemDisplayAdapter() { }
    
    /// If the subclass can support a bounding box, this returns true
    ///  and the bounds.  Z values are ignored for now.
    /// If the subclass can't support bounds (e.g. a globe), you get false back.
    virtual bool getBounds(Point3f &ll,Point3f &ur) = 0;

    /// Convert from the system's local coordinates to display coordinates
    virtual WhirlyKit::Point3f localToDisplay(WhirlyKit::Point3f) = 0;
    virtual WhirlyKit::Point3d localToDisplay(WhirlyKit::Point3d) = 0;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual WhirlyKit::Point3f displayToLocal(WhirlyKit::Point3f) = 0;
    virtual WhirlyKit::Point3d displayToLocal(WhirlyKit::Point3d) = 0;
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(Point3f) = 0;
    virtual Point3d normalForLocal(Point3d) = 0;

    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() = 0;
    
    /// Return the current center
    Point3d getCenter() { return center; }

    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() = 0;
    
protected:
    Point3d center;
    CoordSystem *coordSys;
};
    
/** The general coord system display adapter is used by flat maps to encapsulate a general coordinate system.
    This needs to be one which is flat, but is otherwise unconstrained.  The bounding box is where the coordinate system is valid and the center will be the center of display coordinates.
  */
class GeneralCoordSystemDisplayAdapter : public CoordSystemDisplayAdapter
{
public:
    GeneralCoordSystemDisplayAdapter(CoordSystem *coordSys,Point3d ll,Point3d ur,Point3d center);
    ~GeneralCoordSystemDisplayAdapter();

    /// Bounding box where the coordinate system is valid
    bool getBounds(Point3f &ll,Point3f &ur);
    
    /// Convert from the system's local coordinates to display coordinates
    WhirlyKit::Point3f localToDisplay(WhirlyKit::Point3f);
    WhirlyKit::Point3d localToDisplay(WhirlyKit::Point3d);
    
    /// Convert from display coordinates to the local system's coordinates
    WhirlyKit::Point3f displayToLocal(WhirlyKit::Point3f);
    WhirlyKit::Point3d displayToLocal(WhirlyKit::Point3d);
    
    /// For flat systems the normal is Z up.
    Point3f normalForLocal(Point3f) { return Point3f(0,0,1); }
    Point3d normalForLocal(Point3d) { return Point3d(0,0,1); }
    
    /// Get a reference to the coordinate system
    CoordSystem *getCoordSystem() { return coordSys; }
    
    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    bool isFlat() { return true; }
    
protected:
    Point3d ll,ur,center;
    CoordSystem *coordSys;
};

}
