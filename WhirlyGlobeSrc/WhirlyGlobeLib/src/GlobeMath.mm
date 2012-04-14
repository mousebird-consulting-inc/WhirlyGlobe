/*
 *  GlobeMath.mm
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


#import "GlobeMath.h"

using namespace WhirlyKit;

namespace WhirlyGlobe
{

Point3f GlobeCoordSystem::pointFromGeo(GeoCoord geo) 
{ 
	float z = sinf(geo.lat());
	float rad = sqrtf(1.0-z*z);
	Point3f pt(rad*cosf(geo.lon()),rad*sinf(geo.lon()),z);
	return pt;
}

GeoCoord GlobeCoordSystem::geoFromPoint(Point3f pt)
{
	GeoCoord geoCoord;
	geoCoord.lat() = asinf(pt.z());
	float rad = sqrtf(1.0-pt.z()*pt.z());
	geoCoord.lon() = acosf(pt.x() / rad);
	if (pt.y() < 0)  geoCoord.lon() *= -1;
	
	return geoCoord;
}
	
}
