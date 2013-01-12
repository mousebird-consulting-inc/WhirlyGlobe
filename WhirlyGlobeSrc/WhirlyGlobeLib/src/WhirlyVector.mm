/*
 *  WhirlyVector.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/25/11.
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

using namespace Eigen;

namespace WhirlyKit
{
    
GeoCoord GeoCoord::CoordFromDegrees(float lon,float lat)
{
    return GeoCoord(lon/180.f*M_PI,lat/180.f*M_PI);
}
	
Mbr::Mbr(const std::vector<Point2f> &pts)
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
    
void Mbr::addPoints(const std::vector<Point2f> &coords)
{
    for (unsigned int ii=0;ii<coords.size();ii++)
        addPoint(coords[ii]);
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

    
void Mbr::asPoints(std::vector<Point2f> &pts) const
{
    pts.push_back(pt_ll);
    pts.push_back(Point2f(pt_ur.x(),pt_ll.y()));
    pts.push_back(pt_ur);
    pts.push_back(Point2f(pt_ll.x(),pt_ur.y()));
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
	
GeoMbr::GeoMbr(const std::vector<Point2f> &pts)
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

void GeoMbr::addGeoCoords(const std::vector<Point2f> &coords)
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

Eigen::Quaternionf QuatFromTwoVectors(const Point3f &a,const Point3f &b)
{
    Eigen::Quaternionf ret;
    
    Vector3f v0 = a.normalized();
    Vector3f v1 = b.normalized();
    float c = v0.dot(v1);
    
    // The trick here is that we've taken out the checks against
    //  1 (vectors are nearly identical) and -1
    
    Vector3f axis = v0.cross(v1);
    float s = internal::sqrt((1.f+c)*2.f);
    float invs = 1.f/s;
    ret.vec() = axis * invs;
    ret.w() = s * 0.5f;
    
    return ret;
}


}
