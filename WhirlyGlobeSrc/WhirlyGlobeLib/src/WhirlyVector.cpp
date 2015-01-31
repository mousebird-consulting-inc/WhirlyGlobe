/*
 *  WhirlyVector.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/25/11.
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

using namespace Eigen;

namespace WhirlyKit
{
    
GeoCoord GeoCoord::CoordFromDegrees(float lon,float lat)
{
    return GeoCoord(lon/180.f*M_PI,lat/180.f*M_PI);
}
	
Mbr::Mbr(const Point2fVector &pts)
    : pt_ll(0,0), pt_ur(-1,-1)
{
	for (unsigned int ii=0;ii<pts.size();ii++)
		addPoint(pts[ii]);
}
	
void Mbr::addPoint(Point2f pt)
{
	if (!valid())
	{
		pt_ll = pt_ur = pt;  
		return;
	}
	
	pt_ll.x() = std::min(pt_ll.x(),pt.x());  
	pt_ll.y() = std::min(pt_ll.y(),pt.y());
	pt_ur.x() = std::max(pt_ur.x(),pt.x());
	pt_ur.y() = std::max(pt_ur.y(),pt.y());
}
    
void Mbr::addPoint(Point2d pt)
{
    if (!valid())
    {
        pt_ll = pt_ur = Point2f(pt.x(),pt.y());
        return;
    }
    
    pt_ll.x() = std::min(pt_ll.x(),(float)pt.x());
    pt_ll.y() = std::min(pt_ll.y(),(float)pt.y());
    pt_ur.x() = std::max(pt_ur.x(),(float)pt.x());
    pt_ur.y() = std::max(pt_ur.y(),(float)pt.y());
}
    
void Mbr::addPoints(const Point2fVector &coords)
{
    for (unsigned int ii=0;ii<coords.size();ii++)
        addPoint(coords[ii]);
}

void Mbr::addPoints(const Point2dVector &coords)
{
    for (unsigned int ii=0;ii<coords.size();ii++)
        addPoint(Point2f(coords[ii].x(),coords[ii].y()));
}

// Calculate MBR overlap.  All the various kinds.
bool Mbr::overlaps(const Mbr &that) const
{
	// Basic inclusion cases
	if ((that.insideOrOnEdge(pt_ll) || that.insideOrOnEdge(pt_ur) || that.insideOrOnEdge(Point2f(pt_ll.x(),pt_ur.y())) || that.insideOrOnEdge(Point2f(pt_ur.x(),pt_ll.y()))) ||
		(insideOrOnEdge(that.pt_ll) || insideOrOnEdge(that.pt_ur) || insideOrOnEdge(Point2f(that.pt_ll.x(),that.pt_ur.y())) || insideOrOnEdge(Point2f(that.pt_ur.x(),that.pt_ll.y()))))
		return true;
	
	// Now for the skinny overlap cases
	if ((that.pt_ll.x() <= pt_ll.x() && pt_ur.x() <= that.pt_ur.x() &&
		 pt_ll.y() <= that.pt_ll.y() && that.pt_ur.y() <= pt_ur.y()) ||
		(pt_ll.x() <= that.pt_ll.x() && that.pt_ur.x() <= pt_ur.x() &&
		 that.pt_ll.y() <= pt_ll.y() && pt_ur.y() <= that.pt_ur.y()))
		return true;
	if ((pt_ll.x() <= that.pt_ll.x() && that.pt_ur.x() <= pt_ur.x() &&
		 that.pt_ll.y() <= pt_ll.y() && pt_ur.y() <= that.pt_ur.y()) ||
		(that.pt_ll.x() <= pt_ll.x() && pt_ur.x() <= that.pt_ur.x() &&
		 pt_ll.y() <= that.pt_ll.y() && that.pt_ur.y() <= pt_ur.y()))
		return true;
	
	return false;
}
	
float Mbr::area() const
{
	return (pt_ur.x() - pt_ll.x())*(pt_ur.y() - pt_ll.y());
}
    
void Mbr::expand(const Mbr &that)
{
    addPoint(that.pt_ll);
    addPoint(that.pt_ur);
}

    
void Mbr::asPoints(Point2fVector &pts) const
{
    pts.push_back(pt_ll);
    pts.push_back(Point2f(pt_ur.x(),pt_ll.y()));
    pts.push_back(pt_ur);
    pts.push_back(Point2f(pt_ll.x(),pt_ur.y()));
}

void Mbr::asPoints(Point2dVector &pts) const
{
    pts.push_back(Point2d(pt_ll.x(),pt_ll.y()));
    pts.push_back(Point2d(pt_ur.x(),pt_ll.y()));
    pts.push_back(Point2d(pt_ur.x(),pt_ur.y()));
    pts.push_back(Point2d(pt_ll.x(),pt_ur.y()));
}
	
Mbr Mbr::intersect(const Mbr &that) const
{
    Mbr out;
    out.ll().x() = std::max(ll().x(),that.ll().x());
    out.ll().y() = std::max(ll().y(),that.ll().y());
    out.ur().x() = std::min(ur().x(),that.ur().x());
    out.ur().y() = std::min(ur().y(),that.ur().y());
        
    return out;
}
	
GeoMbr::GeoMbr(const std::vector<GeoCoord> &coords)
	: pt_ll(-1000,-1000), pt_ur(-1000,-1000)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
		addGeoCoord(coords[ii]);
}
	
GeoMbr::GeoMbr(const Point2fVector &pts)
	: pt_ll(-1000,-1000), pt_ur(-1000,-1000)
{
	for (unsigned int ii=0;ii<pts.size();ii++)
	{
		const Point2f &pt = pts[ii];
		addGeoCoord(GeoCoord(pt.x(),pt.y()));
	}
}

// Expand the MBR by this coordinate
void GeoMbr::addGeoCoord(GeoCoord coord)
{
	if (!valid())
	{
		pt_ll = pt_ur = coord;
		return;
	}
	
	pt_ll.x() = std::min(pt_ll.x(),coord.x());
	pt_ll.y() = std::min(pt_ll.y(),coord.y());
	pt_ur.x() = std::max(pt_ur.x(),coord.x());
	pt_ur.y() = std::max(pt_ur.y(),coord.y());
}
	
void GeoMbr::addGeoCoords(const std::vector<GeoCoord> &coords)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
		addGeoCoord(coords[ii]);
}

void GeoMbr::addGeoCoords(const Point2fVector &coords)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
	{
		const Point2f &pt = coords[ii];
		addGeoCoord(GeoCoord(pt.x(),pt.y()));
	}
}
	
bool GeoMbr::overlaps(const GeoMbr &that) const
{
	std::vector<Mbr> mbrsA,mbrsB;

	splitIntoMbrs(mbrsA);
	that.splitIntoMbrs(mbrsB);
	
	for (unsigned int aa=0;aa<mbrsA.size();aa++)
		for (unsigned int bb=0;bb<mbrsB.size();bb++)
			if (mbrsA[aa].overlaps(mbrsB[bb]))
				return true;
	
	return false;
}
	
bool GeoMbr::inside(GeoCoord coord) const
{
	std::vector<Mbr> mbrs;
	splitIntoMbrs(mbrs);
	
	for (unsigned int ii=0;ii<mbrs.size();ii++)
		if (mbrs[ii].inside(coord))
			return true;
	
	return false;
}

void GeoMbr::expand(const GeoMbr &mbr)
{
    addGeoCoord(mbr.ll());
    addGeoCoord(mbr.ur());
}
	
float GeoMbr::area() const
{
	float area = 0;
	std::vector<Mbr> mbrs;
	splitIntoMbrs(mbrs);
	
	for (unsigned int ii=0;ii<mbrs.size();ii++)
		area += mbrs[ii].area();
	
	return area;
}
    	
// Break a a geoMbr into one or two pieces
// If we overlap -180/+180 then we need two mbrs
void GeoMbr::splitIntoMbrs(std::vector<Mbr> &mbrs) const
{
	// Simple case
	if (pt_ll.x() <= pt_ur.x())
		mbrs.push_back(Mbr(pt_ll,pt_ur));
	else {
		mbrs.push_back(Mbr(pt_ll,Point2f((float)M_PI,pt_ur.y())));
		mbrs.push_back(Mbr(Point2f((float)(-M_PI),pt_ll.y()),pt_ur));
	}
}

void BBox::addPoint(const Point3d &pt)
{
    if (isValid())
    {
        pt_ll.x() = std::min(pt_ll.x(),pt.x());
        pt_ll.y() = std::min(pt_ll.y(),pt.y());
        pt_ll.z() = std::min(pt_ll.z(),pt.z());
        pt_ur.x() = std::max(pt_ur.x(),pt.x());
        pt_ur.y() = std::max(pt_ur.y(),pt.y());
        pt_ur.z() = std::max(pt_ur.z(),pt.z());
    } else {
        pt_ll = pt;
        pt_ur = pt;
    }
}
    
void BBox::addPoints(const Point3dVector &pts)
{
    for (unsigned int ii=0;ii<pts.size();ii++)
        addPoint(pts[ii]);
}
    
void BBox::asPoints(Point3fVector &pts) const
{
    pts.push_back(Point3f(pt_ll.x(),pt_ll.y(),pt_ll.z()));
    pts.push_back(Point3f(pt_ur.x(),pt_ll.y(),pt_ll.z()));
    pts.push_back(Point3f(pt_ur.x(),pt_ur.y(),pt_ll.z()));
    pts.push_back(Point3f(pt_ll.x(),pt_ur.y(),pt_ll.z()));
    pts.push_back(Point3f(pt_ll.x(),pt_ll.y(),pt_ur.z()));
    pts.push_back(Point3f(pt_ur.x(),pt_ll.y(),pt_ur.z()));
    pts.push_back(Point3f(pt_ur.x(),pt_ur.y(),pt_ur.z()));
    pts.push_back(Point3f(pt_ll.x(),pt_ur.y(),pt_ur.z()));
}

void BBox::asPoints(Point3dVector &pts) const
{
    pts.push_back(Point3d(pt_ll.x(),pt_ll.y(),pt_ll.z()));
    pts.push_back(Point3d(pt_ur.x(),pt_ll.y(),pt_ll.z()));
    pts.push_back(Point3d(pt_ur.x(),pt_ur.y(),pt_ll.z()));
    pts.push_back(Point3d(pt_ll.x(),pt_ur.y(),pt_ll.z()));
    pts.push_back(Point3d(pt_ll.x(),pt_ll.y(),pt_ur.z()));
    pts.push_back(Point3d(pt_ur.x(),pt_ll.y(),pt_ur.z()));
    pts.push_back(Point3d(pt_ur.x(),pt_ur.y(),pt_ur.z()));
    pts.push_back(Point3d(pt_ll.x(),pt_ur.y(),pt_ur.z()));
}

Eigen::Quaterniond QuatFromTwoVectors(const Point3d &a,const Point3d &b)
{
    Eigen::Quaterniond ret;
    
    Vector3d v0 = a.normalized();
    Vector3d v1 = b.normalized();
    double c = v0.dot(v1);
    
    // The trick here is that we've taken out the checks against
    //  1 (vectors are nearly identical) and -1
    
    Vector3d axis = v0.cross(v1);
    double s = sqrt((1.f+c)*2.f);
    double invs = 1.f/s;
    ret.vec() = axis * invs;
    ret.w() = s * 0.5f;
    
    return ret;
}

/// Convert a 4f matrix to a 4d matrix
Eigen::Matrix4d Matrix4fToMatrix4d(const Eigen::Matrix4f &inMat)
{
    Matrix4d outMat;
    for (unsigned int ii=0;ii<16;ii++)
        outMat.data()[ii] = inMat.data()[ii];
    
    return outMat;
}
    
Eigen::Matrix4f Matrix4dToMatrix4f(const Eigen::Matrix4d &inMat)
{
    Matrix4f outMat;
    for (unsigned int ii=0;ii<16;ii++)
        outMat.data()[ii] = inMat.data()[ii];
    
    return outMat;
}
    
/// Floats to doubles
Eigen::Vector2d Vector2fToVector2d(const Eigen::Vector2f &inVec)
{
    return Vector2d(inVec.x(),inVec.y());
}
    
/// Doubles to floats
Eigen::Vector2f Vector2dToVector2f(const Eigen::Vector2d &inVec)
{
    return Vector2f(inVec.x(),inVec.y());
}

/// Floats to doubles
Eigen::Vector3d Vector3fToVector3d(const Eigen::Vector3f &inVec)
{
    Vector3d outVec;
    outVec.x() = inVec.x();  outVec.y() = inVec.y();  outVec.z() = inVec.z();
    
    return outVec;
}

// Double to floats
Eigen::Vector3f Vector3dToVector3f(const Eigen::Vector3d &inVec)
{
    Vector3f outVec;
    outVec.x() = inVec.x();  outVec.y() = inVec.y();  outVec.z() = inVec.z();
    
    return outVec;
}
    
/// Floats to doubles
Eigen::Vector4d Vector4fToVector4d(const Eigen::Vector4f &inVec)
{
    Vector4d outVec;
    outVec.x() = inVec.x();  outVec.y() = inVec.y();  outVec.z() = inVec.z();  outVec.w() = inVec.w();
    
    return outVec;    
}


}
