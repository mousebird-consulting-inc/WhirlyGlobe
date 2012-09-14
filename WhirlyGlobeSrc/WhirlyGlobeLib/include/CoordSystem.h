/*
 *  CoordSystem.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011-2012 mousebird consulting
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
#import "WhirlyGeometry.h"

namespace WhirlyKit
{
    
/// Degree to radians conversion
template<typename T>
T DegToRad(T deg) { return deg / 180.0 * (T)M_PI; }

/// Radians to degress
template<typename T>
T RadToDeg(T rad) { return rad / (T)M_PI * 180.0; }    
    
}

namespace WhirlyKit
{

/// Base class for the various coordinate systems
///  we use in the toolkits.
class CoordSystem
{
public:
    CoordSystem() { }
    virtual ~CoordSystem() { }
    
    /// Convert from the local coordinate system to lat/lon
    virtual WhirlyKit::GeoCoord localToGeographic(WhirlyKit::Point3f) = 0;
    /// Convert from lat/lon t the local coordinate system
    virtual WhirlyKit::Point3f geographicToLocal(WhirlyKit::GeoCoord) = 0;

    /// Convert from the local coordinate system to geocentric
    virtual WhirlyKit::Point3f localToGeocentric(WhirlyKit::Point3f) = 0;
    /// Convert from display coordinates to geocentric
    virtual WhirlyKit::Point3f geocentricToLocal(WhirlyKit::Point3f) = 0;
};
    
/// Convert a point from one coordinate system to another
Point3f CoordSystemConvert(CoordSystem *inSystem,CoordSystem *outSystem,Point3f inCoord);
    
/** The Coordinate System Display Adapter handles the task of
    converting coordinates in the native system to data values we
    can display.
 */
class CoordSystemDisplayAdapter
{
public:
    CoordSystemDisplayAdapter(CoordSystem *coordSys) : coordSys(coordSys) { }
    virtual ~CoordSystemDisplayAdapter() { }
    
    /// Convert from the system's local coordinates to display coordinates
    virtual WhirlyKit::Point3f localToDisplay(WhirlyKit::Point3f) = 0;
    
    /// Convert from display coordinates to the local system's coordinates
    virtual WhirlyKit::Point3f displayToLocal(WhirlyKit::Point3f) = 0;
    
    /// For flat systems the normal is Z up.  For the globe, it's based on the location.
    virtual Point3f normalForLocal(Point3f) = 0;

    /// Get a reference to the coordinate system
    virtual CoordSystem *getCoordSystem() = 0;

    /// Return true if this is a projected coordinate system.
    /// False for others, like geographic.
    virtual bool isFlat() = 0;
    
protected:
    CoordSystem *coordSys;
};

}
