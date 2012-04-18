/*
 *  GlobeMath.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/2/11.
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

namespace WhirlyGlobe
{

class GlobeCoordSystem : public WhirlyKit::CoordSystem
{
public:
    /// From a geo coordinate, generate the 3D location on a globe of radius 1.0
    WhirlyKit::Point3f pointFromGeo(WhirlyKit::GeoCoord geo);
	
    /// From a 3D point on a sphere of radius 1.0, generate the corresponding geo coordinate
    WhirlyKit::GeoCoord geoFromPoint(WhirlyKit::Point3f pt);
    
    /// Not flat
    bool isFlat() { return false; }
};

}
