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
	
bool IntersectLines(const Point2f &p1,const Point2f &p2,const Point2f &p3,const Point2f &p4,Point2f *iPt)
{
    float denom = (p1.x()-p2.x())*(p3.y()-p4.y()) - (p1.y() - p2.y())*(p3.x() - p4.x());
    if (denom == 0.0)
        return false;
    
    float termA = (p1.x()*p2.y() - p1.y()*p2.x());
    float termB = (p3.x() * p4.y() - p3.y() * p4.x());
    iPt->x() = ( termA * (p3.x() - p4.x()) - (p1.x() - p2.x()) * termB)/denom;
    iPt->y() = ( termA * (p3.y() - p4.y()) - (p1.y() - p2.y()) * termB)/denom;
    
    return true;
}
    
// Homogeneous clipping code credit to:
//   http://wwwx.cs.unc.edu/~sud/courses/236/a5/softgl_homoclip_smooth.cpp

// Clipping planes
typedef enum {Left,Right,Bottom,Top,Near,Far} ClipPlane;
    
Eigen::Vector4f intersectPlane(const Vector4f &p1,const Vector4f &p2,ClipPlane plane)
{
    float t=0.0;
    switch (plane)
    {
        case Left   : t=(-p1.w()-p1.x())/(p2.x()-p1.x()+p2.w()-p1.w()); break;
        case Right  : t=(p1.w()-p1.x())/(p2.x()-p1.x()-p2.w()+p1.w());  break;
        case Bottom : t=(-p1.w()-p1.y())/(p2.y()-p1.y()+p2.w()-p1.w()); break;
        case Top    : t=(p1.w()-p1.y())/(p2.y()-p1.y()-p2.w()+p1.w());  break;
        case Near   : t=(-p1.w()-p1.z())/(p2.z()-p1.z()+p2.w()-p1.w()); break;
        case Far    : t=(p1.w()-p1.z())/(p2.z()-p1.z()-p2.w()+p1.w());  break;
    }
    
    Vector4f pt = p1 + (p2-p1)*t;
    
    return pt;
}
    
bool insidePlane(const Vector4f &pt,ClipPlane plane)
{
    switch (plane)
    {
        case Left: return pt.x()>=  -pt.w();
        case Right: return pt.x()<=  pt.w();
        case Bottom: return pt.y()>=-pt.w();
        case Top: return pt.y()<=    pt.w();
        case Near: return pt.z()>=  -pt.w();
        case Far: return pt.z()<=    pt.w();
    }
    
    // Won't get here
    return false;
}

void ClipHomogeneousPolyToPlane(const std::vector<Eigen::Vector4f> &pts,ClipPlane plane,std::vector<Eigen::Vector4f> &outPts)
{
    outPts.clear();
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        const Vector4f &p0 = pts[ii];
        const Vector4f &p1 = pts[(ii+1)%pts.size()];
        bool p0_in = insidePlane(p0,plane);
        bool p1_in = insidePlane(p1,plane);
        // Edge crosses plane
        if (p0_in != p1_in)
        {
            Vector4f newP = intersectPlane(p0,p1,plane);
            outPts.push_back(newP);
        }
        // 2nd vertex inside, add it
        if (p1_in)
            outPts.push_back(p1);
    }
}
    
void ClipHomogeneousPolygon(const std::vector<Eigen::Vector4f> &inPts,std::vector<Eigen::Vector4f> &outPts)
{
    if (inPts.size() < 3)
        return;
    std::vector<Vector4f> pts = inPts;
 
    ClipHomogeneousPolyToPlane(pts, Left, outPts);  pts = outPts;
    ClipHomogeneousPolyToPlane(pts, Right, outPts);  pts = outPts;
    ClipHomogeneousPolyToPlane(pts, Bottom, outPts);  pts = outPts;
    ClipHomogeneousPolyToPlane(pts, Top, outPts);  pts = outPts;
    ClipHomogeneousPolyToPlane(pts, Near, outPts);  pts = outPts;
    ClipHomogeneousPolyToPlane(pts, Far, outPts);
}

// Note: Maybe finish implementing this
bool RectSolidRayIntersect(const Ray3f &ray,const Point3f *pts,float &dist2)
{
    return false;
}
    
// Inspired by: http://geomalgorithms.com/a01-_area.html
float PolygonArea(const std::vector<Point3f> &poly,const Point3f &norm)
{
    if (poly.size() < 3)
        return 0.0;
    float area = 0.0;

    // Decide which coordinate to ignore
    Point3f a(std::abs(norm.x()),std::abs(norm.y()),std::abs(norm.z()));
    int coord = (a.x() > a.y()) ? (a.x() > a.z() ? 1 : 3) : (a.y() > a.z() ? 2 : 3);
    
    // Area of the 3D version
    for (unsigned int ii=0;ii<poly.size();ii++)
    {
        const Point3f &p1 = poly[ii];
        const Point3f &p2 = poly[(ii+1)%poly.size()];
        switch (coord)
        {
            case 1:
                area += p1.y()*p2.z() - p1.z()*p2.y();
                break;
            case 2:
                area += p1.x()*p2.z() - p1.z()*p2.x();
                break;
            case 3:
                area += p1.x()*p2.y() - p1.y()*p2.x();
                break;
        }
    }
                 
    // Scale to get a 3D area back
    float an = sqrtf(a.x() * a.x() + a.y() * a.y() + a.z() * a.z());
    switch (coord)
    {
        case 1:
            area *= (an / (2*a.x()));
            break;
        case 2:
            area *= (an / (2*a.y()));
            break;
        case 3:
            area *= (an / (2*a.z()));
            break;
    }
                 
    return area;
}

	
}
