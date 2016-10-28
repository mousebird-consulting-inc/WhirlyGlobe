/*
 *  WhirlyGeometry.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2016 mousebird consulting
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
bool IntersectUnitSphere(Point3d org,Eigen::Vector3d dir,Point3d &hit,double *t=NULL);

/// Intersect with a sphere of a given radius
bool IntersectSphereRadius(Point3d org,Eigen::Vector3d dir,double radius,Point3d &hit,double *retT);

/// Returns true if the given point is inside the close polygon
///  defined by ring.  Standard winding-ish test.
bool PointInPolygon(Point2f pt,const Point2fVector &ring);

/// Run a convex polygon intersection check.  Returns true if they overlap
bool ConvexPolyIntersect(const Point2fVector &pts0,const Point2fVector &pts1);
	
/// Run a convex polygon intersection check.  Returns true if they overlap
bool ConvexPolyIntersect(const Point2dVector &pts0,const Point2dVector &pts1);
	
/// Return the next higher power of 2 unless the input is a power of 2.  Doesn't work for 0.
unsigned int NextPowOf2(unsigned int val);
    
/// Find the point on a line segment closest to the give point
Point2f ClosestPointOnLineSegment(const Point2f &p0,const Point2f &p1,const Point2f &pt,float &t);
	
/// Find the point on a line segment closest to the given point.  Also returns the parametric value.
Point2d ClosestPointOnLineSegment(const Point2d &p0,const Point2d &p1,const Point2d &pt,double &t);
	
/// Calculates the intersection point of two lines (not line segments) if there is one
bool IntersectLines(const Point2f &a0,const Point2f &a1,const Point2f &b0,const Point2f &b1,Point2f *iPt);

/// Clip and return a polygon represented in homogeneous coordinates
void ClipHomogeneousPolygon(const Vector4dVector &pts,Vector4dVector &outPts);
	
/// Project and clip a given polygon to screen space.  Clips in homogeneous coordinates.
void ClipAndProjectPolygon(Eigen::Matrix4d &modelMat,Eigen::Matrix4d &projMat,Point2f frameSize,Point3dVector &poly,Point2fVector &screenPoly);
    
/// Return the area of the 3D polygon
float PolygonArea(const Point3fVector &poly,const Point3f &norm);

/// Return the area of the 3D polygon
double PolygonArea(const Point3dVector &poly,const Point3d &norm);
    
/// Return the Barycentric coordinates for the given point within the given triangle
void BarycentricCoords(const Point2d &p,const Point2d &a,const Point2d &b,const Point2d &c,double &u,double &v,double &w);

/// Look for a ray/bounding box intersection
bool BoundingBoxRayIntersect(const Point3d &org,const Point3d &dir,const BBox &bbox, double *minT, double *maxT, Point3d *minPt, Point3d *maxPt);

/// Look for a ray/polygon intersection
//bool PolygonRayIntersect(const Point3d &org,const Point3d &dir,const std::vector<Point3d> &poly,Point3d *iPt);

/// Look for a ray/triangle intersection
bool TriangleRayIntersection(const Point3d &org,const Point3d &dir,const Point3d pts[3], double *outT, Point3d *iPt);
    
}
