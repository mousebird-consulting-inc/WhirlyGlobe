/*
 *  FlatMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/9/12.
 *  Copyright 2011 mousebird consulting
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
#import "CoordSystem.h"

namespace WhirlyMap
{
    
/** The Flat Coordinate System just unrolls lat/lon in radians and
    represents the map as a flat non-projection of that.
    Technically, this is plate caree: http://en.wikipedia.org/wiki/Equirectangular_projection
  */
class FlatCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// From a geo coordinate, generate the 3D location on a globe of radius 1.0
    WhirlyGlobe::Point3f pointFromGeo(WhirlyGlobe::GeoCoord geo);
    
    /// From a 3D point on a plane at z = 0.0, convert to geographic
    WhirlyGlobe::GeoCoord geoFromPoint(WhirlyGlobe::Point3f pt);     
    
    /// Working in a flat space
    bool isFlat() { return true; }
};
    
/** The Mercator Projection, bane of cartographers everywhere.
    It stretches out the world in a familiar way, making the US
    look almost as big as our collective ego.  And Greenland.  For some reason.
  */
class MercatorCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// Construct with an optional origin for the projection in radians
    /// The equator is default
    MercatorCoordSystem(float originLon=0.0);
    
    /// From a geo coordinate, generate the 3D location on a globe of radius 1.0
    WhirlyGlobe::Point3f pointFromGeo(WhirlyGlobe::GeoCoord geo);
    
    /// From a 3D point on a plane at z = 0.0, convert to geographic
    WhirlyGlobe::GeoCoord geoFromPoint(WhirlyGlobe::Point3f pt);     

    bool isFlat() { return true; }
    
protected:
    float originLon;
};
    
}
