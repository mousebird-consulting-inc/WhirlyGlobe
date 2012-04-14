/*
 *  Cullable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "GlobeScene.h"
#import "GlobeMath.h"

namespace WhirlyKit
{
	
void Cullable::setGeoMbr(const GeoMbr &inMbr,WhirlyKit::CoordSystem *coordSystem)
{
	geoMbr = inMbr;
	
	// Turn the corner points in real world values
	cornerPoints[0] = coordSystem->pointFromGeo(geoMbr.ll());
	cornerPoints[1] = coordSystem->pointFromGeo(GeoCoord(geoMbr.ur().x(),geoMbr.ll().y()));
	cornerPoints[2] = coordSystem->pointFromGeo(geoMbr.ur());
	cornerPoints[3] = coordSystem->pointFromGeo(GeoCoord(geoMbr.ll().x(),geoMbr.ur().y()));
	
	// Normals happen to be the same
	for (unsigned int ii=0;ii<4;ii++)
		cornerNorms[ii] = cornerPoints[ii];
}

}
