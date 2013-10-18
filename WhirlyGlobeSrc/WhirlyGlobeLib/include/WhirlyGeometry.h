/*
 *  WhirlyGeometry.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2013 mousebird consulting
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
bool IntersectUnitSphere(Point3d org,Eigen::Vector3d dir,Point3d &hit);

/// Returns true if the given point is inside the close polygon
///  defined by ring.  Standard winding-ish test.
bool PointInPolygon(Point2f pt,const std::vector<Point2f> &ring);

/// Run a convex polygon intersection check.  Returns true if they overlap
bool ConvexPolyIntersect(const std::vector<Point2f> &pts0,const std::vector<Point2f> &pts1);
	
/// Return the next higher power of 2 unless the input is a power of 2.  Doesn't work for 0.
unsigned int NextPowOf2(unsigned int val);
    
/// Find the point on a line segment closest to the give point
Point2f ClosestPointOnLineSegment(const Point2f &p0,const Point2f &p1,const Point2f &pt);
	
/// Calculates the intersection point of two lines (not line segments) if there is one
bool IntersectLines(const Point2f &a0,const Point2f &a1,const Point2f &b0,const Point2f &b1,Point2f *iPt);

/// Clip and return a polygon represented in homogeneous coordinates
void ClipHomogeneousPolygon(const std::vector<Eigen::Vector4d> &pts,std::vector<Eigen::Vector4d> &outPts);
	
/// Project and clip a given polygon to screen space.  Clips in homogeneous coordinates.
void ClipAndProjectPolygon(Eigen::Matrix4d &modelMat,Eigen::Matrix4d &projMat,Point2f frameSize,std::vector<Point3d> &poly,std::vector<Point2f> &screenPoly);
    
/// Look for a ray/rectangular solid intersection.
/// Return true if we found one and the distance^2 from the ray origin to the intersection
bool RectSolidRayIntersect(const Ray3f &ray,const Point3f *pts,float &dist2);
    
/// Return the area of the 3D polygon
float PolygonArea(const std::vector<Point3f> &poly,const Point3f &norm);

/// Return the area of the 3D polygon
double PolygonArea(const std::vector<Point3d> &poly,const Point3d &norm);
    
}
