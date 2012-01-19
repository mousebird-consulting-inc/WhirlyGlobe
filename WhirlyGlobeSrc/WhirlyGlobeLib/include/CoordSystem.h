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

namespace WhirlyGlobe
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
    
    /// Implement this to convert from lat/lon (radians) to the location in
    ///  3D.
    virtual WhirlyGlobe::Point3f pointFromGeo(WhirlyGlobe::GeoCoord geo) = 0;
  
    /// Implement this to convert from a 3D point to a lat/lon value in radians
    virtual WhirlyGlobe::GeoCoord geoFromPoint(WhirlyGlobe::Point3f pt) = 0;
    
    /// Return true if this is a relatively flat coordinate system.
    /// False for a globe.
    virtual bool isFlat() = 0;
};

}
