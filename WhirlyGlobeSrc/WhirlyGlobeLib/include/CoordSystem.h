/*
 *  CoordSystem.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2012 mousebird consulting
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

    /// Convert from the local coordinate system to display coordinates (geocentric-ish)
    virtual WhirlyKit::Point3f localToGeocentricish(WhirlyKit::Point3f) = 0;    
    /// Convert from display coordinates to the local coordinate system
    virtual WhirlyKit::Point3f geocentricishToLocal(WhirlyKit::Point3f) = 0;
        
    /// Return true if this is a relatively flat coordinate system.
    /// False for geographic.
    virtual bool isFlat() = 0;
};

}
