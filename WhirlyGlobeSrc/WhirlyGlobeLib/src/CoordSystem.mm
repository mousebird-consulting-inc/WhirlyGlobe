/*
 *  CoordSystem.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/11.
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

#import "CoordSystem.h"

using namespace Eigen;

namespace WhirlyKit
{

Point3f CoordSystemConvert(CoordSystem *inSystem,CoordSystem *outSystem,Point3f inCoord)
{
    // Easy if the coordinate systems are the same
    if (inSystem->isSameAs(outSystem))
        return inCoord;
    
    // We'll go through geocentric which isn't horrible, but obviously we're assuming the same datum
    Point3f geoCPt = inSystem->localToGeocentric(inCoord);
    Point3f outPt = outSystem->geocentricToLocal(geoCPt);
    return outPt;
}

}
