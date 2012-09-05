/*
 *  WhirlyGeometry.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
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

#import "WhirlyGeometry.h"

using namespace Eigen;

namespace WhirlyKit
{

bool IntersectUnitSphere(Point3f org,Vector3f dir,Point3f &hit)
{
	float a = dir.dot(dir);
	float b = 2.0f * org.dot(dir);
	float c = org.dot(org) - 1.0;
	
	float sq = b*b - 4.0f * a * c;
	if (sq < 0.0)
		return false;
	
	float rt = sqrtf(sq);
	float ta = (-b + rt) / (2.0f * a);
	float tb = (-b - rt) / (2.0f * a);
	
	float t = std::min(ta,tb);
	
	hit = org + dir * t;
	return true;
}
    	
// Point in poly routine
// Courtesy: http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

bool PointInPolygon(Point2f pt,const std::vector<Point2f> &ring)
{
	int ii, jj;
	bool c = false;
	for (ii = 0, jj = ring.size()-1; ii < ring.size(); jj = ii++) {
		if ( ((ring[ii].y()>pt.y()) != (ring[jj].y()>pt.y())) &&
			(pt.x() < (ring[jj].x()-ring[ii].x()) * (pt.y()-ring[ii].y()) / (ring[jj].y()-ring[ii].y()) + ring[ii].x()) )
			c = !c;
	}
	return c;
}

// Courtesy: http://acius2.blogspot.com/2007/11/calculating-next-power-of-2.html
unsigned int NextPowOf2(unsigned int val)
{
	val--;
	val = (val >> 1) | val;
	val = (val >> 2) | val;
	val = (val >> 4) | val;
	val = (val >> 8) | val;
	val = (val >> 16) | val;
	
	return (val + 1);
}
    
// General purpose 2D point closest to line segment
Point2f ClosestPointOnLineSegment(const Point2f &p0,const Point2f &p1,const Point2f &pt)
{
    float dx = p1.x()-p0.x(), dy = p1.y()-p0.y();
    float denom = dx*dx+dy*dy;
    
    if (denom == 0.0)
        return p0;
    
    float u = ((pt.x()-p0.x())*(p1.x()-p0.x())+(pt.y()-p0.y())*(p1.y()-p0.y()))/denom;
    
    if (u <= 0.0)
        return p0;
    
    if (u >= 1.0)
        return p1;

    return Point2f(p0.x()+dx*u,p0.y()+dy*u);
}
	
}
