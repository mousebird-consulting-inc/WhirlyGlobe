/*
 *  debug.mm
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 1/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <WhirlyGlobe.h>

/* This is here to pull in the various print methods from objects
   we may need to debug in the WhirlyGlobe lib.
	Put this formatter in place for all the various types in Xcode:
	 {$VAR.debugString()}:s
 */
void PullInMethods()
{
	WhirlyGlobe::Point3f pt3(0,0,0);
	pt3.debugString();

	WhirlyGlobe::Point2f pt2(0,0);
	pt2.debugString();

	WhirlyGlobe::Matrix3f mat3;
	mat3.debugString();
	
	WhirlyGlobe::Matrix4f mat4;
	mat4.debugString();
	
	WhirlyGlobe::GeoCoord gc(0,0);
	gc.debugString();
}
