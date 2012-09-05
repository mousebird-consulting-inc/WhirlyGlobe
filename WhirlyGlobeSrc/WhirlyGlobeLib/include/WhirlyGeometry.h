/*
 *  WhirlyGeometry.h
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

#import "WhirlyVector.h"

namespace WhirlyKit
{

/** Intersect a unit sphere with the given origin/vector
    Return true if we found one
    Returns the intersection in hit or the closest pass
 */
bool IntersectUnitSphere(Point3f org,Eigen::Vector3f dir,Point3f &hit);

/// Returns true if the given point is inside the close polygon
///  defined by ring.  Standard winding-ish test.
bool PointInPolygon(Point2f pt,const std::vector<Point2f> &ring);
	
/// Return the next higher power of 2 unless the input is a power of 2.  Doesn't work for 0.
unsigned int NextPowOf2(unsigned int val);
    
/// Find the point on a line segment closest to the give point
Point2f ClosestPointOnLineSegment(const Point2f &p0,const Point2f &p1,const Point2f &pt);
	
}
