/*  WhirlyVector.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/25/11.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "WhirlyVector.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{

Mbr::Mbr(const MbrD &inMbr) :
    pt_ll(Point2f(inMbr.ll().x(),inMbr.ll().y())),
    pt_ur(Point2f(inMbr.ur().x(),inMbr.ur().y()))
{
}
	
Mbr::Mbr(const Point2fVector &pts)
    : pt_ll(0,0), pt_ur(-1,-1)
{
	for (unsigned int ii=0;ii<pts.size();ii++)
		addPoint(pts[ii]);
}

bool Mbr::operator == (const Mbr &that) const
{
    return pt_ll == that.pt_ll && pt_ur == that.pt_ur;
}

void Mbr::addPoint(const Point2f &pt)
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

void Mbr::addPoint(const Point2d &pt)
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

template <typename T> bool inside(const T &ll, const T &ur, const typename T::Scalar x, const typename T::Scalar y) {
    return ll.x() < x && ll.y() < y && x < ur.x() && y < ur.y();
}
template <typename T> bool inside(const T &ll, const T &ur, const T &pt) { return inside(ll, ur, pt.x(), pt.y()); }

template <typename T> bool insideOrOnEdge(const T &ll, const T &ur, const typename T::Scalar x, const typename T::Scalar y) {
    return ll.x() <= x && ll.y() <= y && x <= ur.x() && y <= ur.y();
}
template <typename T> bool insideOrOnEdge(const T &ll, const T &ur, const T &pt) { return insideOrOnEdge(ll, ur, pt.x(), pt.y()); }

// not reflexive
template <typename T> bool insideOrOnEdgeOneWay(const T &mbrA, const T &mbrB)
{
    return insideOrOnEdge(mbrA.ll(), mbrA.ur(), mbrB.ll()) ||
           insideOrOnEdge(mbrA.ll(), mbrA.ur(), mbrB.ur()) ||
           insideOrOnEdge(mbrA.ll(), mbrA.ur(), mbrB.ll().x(), mbrB.ur().y()) ||
           insideOrOnEdge(mbrA.ll(), mbrA.ur(), mbrB.ur().x(), mbrB.ll().y());
}

template <typename T> bool overlapOneWay(const T &a, const T &b)
{
    return (b.ll().x() <= a.ll().x() && a.ur().x() <= b.ur().x() &&
            a.ll().y() <= b.ll().y() && b.ur().y() <= a.ur().y());
}

bool Mbr::inside(const Point2f &pt) const
{
    return WhirlyKit::inside(pt_ll, pt_ur, pt);
}

bool Mbr::insideOrOnEdge(const Point2f &pt) const
{
    return WhirlyKit::insideOrOnEdge(pt_ll, pt_ur, pt);
}

// Calculate MBR overlap.  All the various kinds.
bool Mbr::overlaps(const Mbr &that) const
{
	return insideOrOnEdgeOneWay(*this, that) ||
           insideOrOnEdgeOneWay(that, *this) ||
           overlapOneWay(*this, that) ||
           overlapOneWay(that, *this);
}
	
float Mbr::area() const
{
	return (pt_ur.x() - pt_ll.x())*(pt_ur.y() - pt_ll.y());
}

Point2f Mbr::span() const
{
    return Point2f(pt_ur.x()-pt_ll.x(),pt_ur.y()-pt_ll.y());
}

void Mbr::expand(const Mbr &that)
{
    addPoint(that.pt_ll);
    addPoint(that.pt_ur);
}

void Mbr::expandByFraction(double bufferZone)
{
    const Point2f spanViewMbr = span();
    pt_ll.x() = pt_ll.x()-spanViewMbr.x()*bufferZone;
    pt_ll.y() = pt_ll.y()-spanViewMbr.y()*bufferZone;
    pt_ur.x() = pt_ur.x()+spanViewMbr.x()*bufferZone;
    pt_ur.y() = pt_ur.y()+spanViewMbr.y()*bufferZone;
}

void Mbr::asPoints(Point2fVector &pts) const
{
    pts.reserve(pts.size() + 4);
    pts.push_back(pt_ll);
    pts.emplace_back(pt_ur.x(),pt_ll.y());
    pts.push_back(pt_ur);
    pts.emplace_back(pt_ll.x(),pt_ur.y());
}

void Mbr::asPoints(Point2dVector &pts) const
{
    pts.reserve(pts.size() + 4);
    pts.emplace_back(pt_ll.x(),pt_ll.y());
    pts.emplace_back(pt_ur.x(),pt_ll.y());
    pts.emplace_back(pt_ur.x(),pt_ur.y());
    pts.emplace_back(pt_ll.x(),pt_ur.y());
}

Mbr Mbr::intersect(const Mbr &that) const
{
    return {
        { std::max(ll().x(),that.ll().x()), std::max(ll().y(),that.ll().y()) },
        { std::min(ur().x(),that.ur().x()), std::min(ur().y(),that.ur().y()) }
    };
}

MbrD::MbrD(const Point2dVector &pts)
: pt_ll(0,0), pt_ur(-1,-1)
{
    for (unsigned int ii=0;ii<pts.size();ii++)
        addPoint(pts[ii]);
}

bool MbrD::operator == (const MbrD &that) const
{
    return pt_ll == that.pt_ll && pt_ur == that.pt_ur;
}
    
void MbrD::addPoint(const Point2f &pt)
{
    if (!valid())
    {
        pt_ll = pt_ur = Point2d(pt.x(),pt.y());
        return;
    }
    
    pt_ll.x() = std::min(pt_ll.x(),(double)pt.x());
    pt_ll.y() = std::min(pt_ll.y(),(double)pt.y());
    pt_ur.x() = std::max(pt_ur.x(),(double)pt.x());
    pt_ur.y() = std::max(pt_ur.y(),(double)pt.y());
}

void MbrD::addPoint(const Point2d &pt)
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

void MbrD::addPoints(const Point2fVector &coords)
{
    for (unsigned int ii=0;ii<coords.size();ii++)
        addPoint(coords[ii]);
}

void MbrD::addPoints(const Point2dVector &coords)
{
    for (unsigned int ii=0;ii<coords.size();ii++)
        addPoint(coords[ii]);
}

bool MbrD::inside(const Point2d &pt) const
{
    return WhirlyKit::inside(pt_ll, pt_ur, pt);
}

bool MbrD::insideOrOnEdge(const Point2d &pt) const
{
    return WhirlyKit::insideOrOnEdge(pt_ll, pt_ur, pt);
}

// Calculate MBR overlap.  All the various kinds.
bool MbrD::overlaps(const MbrD &that) const
{
    return insideOrOnEdgeOneWay(*this, that) ||
           insideOrOnEdgeOneWay(that, *this) ||
           overlapOneWay(*this,that) ||
           overlapOneWay(that,*this);
}

float MbrD::area() const
{
    return (pt_ur.x() - pt_ll.x())*(pt_ur.y() - pt_ll.y());
}

Point2d MbrD::span() const
{
    return Point2d(pt_ur.x()-pt_ll.x(),pt_ur.y()-pt_ll.y());
}


void MbrD::expand(const MbrD &that)
{
    addPoint(that.pt_ll);
    addPoint(that.pt_ur);
}


void MbrD::expandByFraction(double bufferZone)
{
    const Point2d spanViewMbr = span();
    pt_ll.x() = pt_ll.x()-spanViewMbr.x()*bufferZone;
    pt_ll.y() = pt_ll.y()-spanViewMbr.y()*bufferZone;
    pt_ur.x() = pt_ur.x()+spanViewMbr.x()*bufferZone;
    pt_ur.y() = pt_ur.y()+spanViewMbr.y()*bufferZone;
}


void MbrD::asPoints(Point2fVector &pts) const
{
    pts.reserve(pts.size() + 4);
    pts.emplace_back(pt_ll.x(),pt_ll.y());
    pts.emplace_back(pt_ur.x(),pt_ll.y());
    pts.emplace_back(pt_ur.x(),pt_ur.y());
    pts.emplace_back(pt_ll.x(),pt_ur.y());
}

void MbrD::asPoints(Point2dVector &pts) const
{
    pts.reserve(pts.size() + 4);
    pts.push_back(pt_ll);
    pts.emplace_back(pt_ur.x(),pt_ll.y());
    pts.push_back(pt_ur);
    pts.emplace_back(pt_ll.x(),pt_ur.y());
}

MbrD MbrD::intersect(const MbrD &that) const
{
    return {
        { std::max(ll().x(),that.ll().x()), std::max(ll().y(),that.ll().y()) },
        { std::min(ur().x(),that.ur().x()), std::min(ur().y(),that.ur().y()) }
    };
}

// Expand a longitude bound to include a new point, returning the amount expanded
template<typename T> T expandLonDelta(T &w, T& e, T lon)
{
    auto deltaW = 0.0;
    auto deltaE = 0.0;
    if (w <= e)
    {
        // the span doesn't include the anti-meridian
        // just expand east or west as needed
        if (lon > e)
        {
            deltaW = w - lon + 2*(T)M_PI;
            deltaE = lon - e;
        }
        else if (lon < w)
        {
            deltaW = w - lon;
            deltaE = lon - e + 2*(T)M_PI;
        }
        else
        {
            // nothing to do
            return 0;
        }
    }
    else
    {
        // the 	 does include the anti-meridian
        if (lon <= e || w <= lon)
        {
            // it's already included
            return 0;
        }

        // it does span the anti-meridian, which way is closer?
        deltaW = w - lon;
        deltaE = lon - e;
    }

    // Expand whichever side is closer, or whichever
    // one leaves east and west on the same side.
    if (deltaW < deltaE || (deltaW == deltaE && e * lon > 0))
    {
        w = lon;
        return deltaW;
    }
    else
    {
        e = lon;
        return deltaE;
    }
}

template <typename T> void expandLon(T &w, T &e, T lon)
{
    if (lon == (T)M_PI || lon == -(T)M_PI)
    {
        // Account for adding +180 to a bound using -180, or vice versa
        // what a pain
        // Expand whichever way is shorter, falling back on what doesn't cross the anti-meridian
        T e1 = e, w1 = w;
        T const d1 = expandLonDelta(w1, e1, lon);
        T e2 = e, w2 = w;
        T const d2 = expandLonDelta(w2, e2, -lon);

        if (d1 < d2 || (d1 == d2 && w1 < e1))
        {
            e = e1;
            w = w1;
        }
        else
        {
            e = e2;
            w = w2;
        }
    }
    else
    {
        // Just expand it whichever way makes sense
        expandLonDelta(w, e, lon);
    }
}


GeoMbr::GeoMbr(const std::vector<GeoCoord> &coords)
	: pt_ll(BadVal,BadVal), pt_ur(BadVal,BadVal)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
		addGeoCoord(coords[ii]);
}
	
GeoMbr::GeoMbr(const Point2fVector &pts)
	: pt_ll(BadVal,BadVal), pt_ur(BadVal,BadVal)
{
	for (unsigned int ii=0;ii<pts.size();ii++)
	{
		addGeoCoord(pts[ii]);
	}
}

// Expand the MBR by this coordinate
void GeoMbr::addGeoCoord(const Point2f &coord)
{
	if (!valid())
	{
		pt_ll = pt_ur = coord;
		return;
	}

	pt_ll.y() = std::min(pt_ll.y(),coord.y());
	pt_ur.y() = std::max(pt_ur.y(),coord.y());

    expandLon(pt_ll.x(), pt_ur.x(), coord.x());
}

void GeoMbr::addGeoCoord(const Point3d &coord)
{
    addGeoCoord(GeoCoord(coord.x(),coord.y()));
}
	
void GeoMbr::addGeoCoords(const GeoCoordVector &coords)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
		addGeoCoord(coords[ii]);
}
    
void GeoMbr::addGeoCoords(const Point3dVector &coords)
{
    for (const Point3d &coord: coords)
        addGeoCoord(coord);
}

void GeoMbr::addGeoCoords(const Point2fVector &coords)
{
	for (unsigned int ii=0;ii<coords.size();ii++)
	{
		addGeoCoord(coords[ii]);
	}
}
	
bool GeoMbr::overlaps(const GeoMbr &that) const
{
    if (that.ll().y() > pt_ur.y() ||
        that.ur().y() < pt_ll.y())
    {
        return false;
    }
    
    if (pt_ll.x() <= pt_ur.x())
    {
        // this is a normal rect
        if (that.ll().x() <= that.ur().x())
        {
            // that is also a normal rect
            return (that.ur().x() >= pt_ll.x() && that.ll().x() <= pt_ur.x());
        }
        // that crosses the anti-meridian
        return (that.ur().x() >= pt_ll.x() || that.ll().x() <= pt_ur.x());
    }

    // this crosses the anti-meridian
    if (that.ll().x() <= that.ur().x())
    {
        // that is a normal rect
        return (that.ur().x() >= pt_ll.x() || that.ll().x() <= pt_ur.x());
    }

    // both rects cross the anti-meridian, and so must overlap in longitude
    return true;
}
	
bool GeoMbr::inside(const Point2f &coord) const
{
    const auto x = coord.x();
    const auto y = coord.y();
    const auto n = pt_ur.y();
    const auto s = pt_ll.y();
    const auto e = pt_ur.x();
    const auto w = pt_ll.x();
    return s <= y && y <= n &&                  // between latitudes
        ((w < e && w <= x && x <= e) ||         // normal rect, between is inside
        ((e <= w) && (w <= x || x <= e)));      // crossing anti-meridian, between is outside
}

void GeoMbr::expand(const GeoMbr &mbr)
{
    addGeoCoord(mbr.ll());
    addGeoCoord(mbr.ur());
}

GeoCoord GeoMbr::mid() const
{
    const auto n = pt_ur.y();
    const auto s = pt_ll.y();
    const auto e = pt_ur.x();
    const auto w = pt_ll.x();
    if (w < e)
    {
        return { (e + w) / 2, (n + s) / 2 };
    }
    // Add half the span to the west edge, fix overflow if necessary
    const auto x = w + (2 * M_PI - w + e) / 2;
    return { (float)((x <= M_PI) ? x : x - 2 * M_PI), (n + s) / 2 };
}

Point2f GeoMbr::span() const
{
    const auto n = pt_ur.y();
    const auto s = pt_ll.y();
    const auto e = pt_ur.x();
    const auto w = pt_ll.x();
    return {(w <= e) ? (e - w) : (2 * M_PI - w + e), n - s};
}

float GeoMbr::area() const
{
    const Point2f s = span();
    return s.x() * s.y();
}

// Break a a geoMbr into one or two pieces
// If we overlap -180/+180 then we need two mbrs
void GeoMbr::splitIntoMbrs(std::vector<Mbr> &mbrs) const
{
	// Simple case
	if (pt_ll.x() <= pt_ur.x())
		mbrs.push_back(Mbr(pt_ll,pt_ur));
	else {
        mbrs.reserve(2);
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
    pts.reserve(pts.size() + 8);
    pts.emplace_back(pt_ll.x(),pt_ll.y(),pt_ll.z());
    pts.emplace_back(pt_ur.x(),pt_ll.y(),pt_ll.z());
    pts.emplace_back(pt_ur.x(),pt_ur.y(),pt_ll.z());
    pts.emplace_back(pt_ll.x(),pt_ur.y(),pt_ll.z());
    pts.emplace_back(pt_ll.x(),pt_ll.y(),pt_ur.z());
    pts.emplace_back(pt_ur.x(),pt_ll.y(),pt_ur.z());
    pts.emplace_back(pt_ur.x(),pt_ur.y(),pt_ur.z());
    pts.emplace_back(pt_ll.x(),pt_ur.y(),pt_ur.z());
}

void BBox::asPoints(Point3dVector &pts) const
{
    pts.reserve(pts.size() + 8);
    pts.emplace_back(pt_ll.x(),pt_ll.y(),pt_ll.z());
    pts.emplace_back(pt_ur.x(),pt_ll.y(),pt_ll.z());
    pts.emplace_back(pt_ur.x(),pt_ur.y(),pt_ll.z());
    pts.emplace_back(pt_ll.x(),pt_ur.y(),pt_ll.z());
    pts.emplace_back(pt_ll.x(),pt_ll.y(),pt_ur.z());
    pts.emplace_back(pt_ur.x(),pt_ll.y(),pt_ur.z());
    pts.emplace_back(pt_ur.x(),pt_ur.y(),pt_ur.z());
    pts.emplace_back(pt_ll.x(),pt_ur.y(),pt_ur.z());
}

Eigen::Quaterniond QuatFromTwoVectors(const Point3d &a,const Point3d &b)
{
    const Vector3d v0 = a.normalized();
    const Vector3d v1 = b.normalized();
    const double c = v0.dot(v1);
    
    // The trick here is that we've taken out the checks against
    //  1 (vectors are nearly identical) and -1

    if (c < -0.9999999)
    {
        wkLogLevel(Verbose, "QuatFromTwoVectors on opposite vectors");
    }

    const Vector3d axis = v0.cross(v1);
    const double s = sqrt((1+c)*2);

    Eigen::Quaterniond ret;
    ret.vec() = (s != 0) ? (axis / s) : axis;
    ret.w() = s * 0.5;

    return ret;
}

/// Convert a 4f matrix to a 4d matrix
Eigen::Matrix4d Matrix4fToMatrix4d(const Eigen::Matrix4f &inMat)
{
    return inMat.cast<double>();
}

Eigen::Matrix4f Matrix4dToMatrix4f(const Eigen::Matrix4d &inMat)
{
    return inMat.cast<float>();
}

/// Floats to doubles
Eigen::Vector2d Vector2fToVector2d(const Eigen::Vector2f &inVec)
{
    return inVec.cast<double>();
}
    
/// Doubles to floats
Eigen::Vector2f Vector2dToVector2f(const Eigen::Vector2d &inVec)
{
    return inVec.cast<float>();
}

/// Floats to doubles
Eigen::Vector3d Vector3fToVector3d(const Eigen::Vector3f &inVec)
{
    return inVec.cast<double>();
}

// Double to floats
Eigen::Vector3f Vector3dToVector3f(const Eigen::Vector3d &inVec)
{
    return inVec.cast<float>();
}

/// Floats to doubles
Eigen::Vector4d Vector4fToVector4d(const Eigen::Vector4f &inVec)
{
    return inVec.cast<double>();
}

//#define LOW_LEVEL_UNIT_TESTS
#if defined(LOW_LEVEL_UNIT_TESTS)
static struct UnitTests {
    UnitTests() {
        genericMbr<Mbr>();
        genericMbr<MbrD>();
        genericMbr<GeoMbr>();
        genericGeoMbr<GeoMbr>();
        
        mbrInclude<Mbr,float>();
        mbrInclude<Mbr,double>();
        mbrInclude<MbrD,float>();
        mbrInclude<MbrD,double>();
        mbrInclude<GeoMbr,float>();
        mbrInclude<GeoMbr,double>();
        geoMbrInclude<GeoMbr,float>();
        geoMbrInclude<GeoMbr,double>();
        //geoMbrInclude<GeoMbrD,float>();
        //geoMbrInclude<GeoMbrD,double>();

        mbrOverlap<Mbr,float>();
        mbrOverlap<Mbr,double>();
        mbrOverlap<MbrD,float>();
        mbrOverlap<MbrD,double>();
        mbrOverlap<GeoMbr,float>();
        mbrOverlap<GeoMbr,double>();
        geoMbrOverlap<GeoMbr,float>();
        geoMbrOverlap<GeoMbr,double>();
        //geoMbrOverlap<GeoMbrD,float>();
        //geoMbrOverlap<GeoMbrD,double>();

        wkLog("Vector unit tests passed");
    }
    template <typename T> void genericMbr() {
        using TP = typename T::value_type;
        assert(!T().valid());
        assert(T({1,1},{2,2}).valid());
        assert(T(T({1,1},{2,2})).valid());
        {T t({1,1},{2,2}); t.reset(); assert(!t.valid());}
        assert(T({1,2},{3,5}).area() == 6);
        assert(T({1,2},{3,5}).span() == TP(2,3));
        assert(T({1,2},{3,5}).mid() == TP(2,3.5));
    }
    template <typename TM, typename T = typename TM::value_type::Scalar> void genericGeoMbr() {
        using TP = typename TM::value_type;
        const auto a = (T)M_PI; // anti-meridian
        const auto b = (T)(5 * M_PI / 180); // arbitrary but distinguishable amounts
        const auto c = (T)(10 * M_PI / 180);
        const auto ep = (T)1e-6;
        assert(approx(TM({a-c,2},{-a+b,5}).span().x(), b+c, ep));
        assert(approx(TM({a-b,2},{-a+c,5}).span().x(), b+c, ep));
        assert(approx(TM({a-c,2},{-a+b,5}).area(), (b+c)*3, ep));
        assert(approx(TM({a-b,2},{-a+c,5}).area(), (b+c)*3, ep));
        assert(approx(TM({a-c,2},{-a+b,5}).mid().x(), a-c+(b+c)/2, ep));
        assert(approx(TM({a-b,2},{-a+c,5}).mid().x(), -a-b+(b+c)/2, ep));
    }
    template <typename TM,typename T = typename TM::value_type::Scalar> void inc(
            T n, T e, T s, T w,         // starting area
            T lat, T lon,               // new point
            T rn, T re, T rs, T rw,     // expected result
            T epsilon = 1.0e-6f) {
        using TP = typename TM::value_type;
        TM m(TP(w,s), TP(e,n));
        m.addPoint(TP(lon,lat));
        assert(std::fabs(m.ur().y() - rn) <= epsilon);
        assert(std::fabs(m.ur().x() - re) <= epsilon);
        assert(std::fabs(m.ll().y() - rs) <= epsilon);
        assert(std::fabs(m.ll().x() - rw) <= epsilon);
    }
    template <typename TM, typename T> void mbrInclude() {
        const auto b = (T)(5 * M_PI / 180);
        //           ------- start ------- --- input -- ------ result -------
        //           n  e        s  w      lat  lon     n  e        s  w
        inc<TM,T>(0, +0 + 0,  0, +0 + 0, +0, +0 + 0, 0, +0 + 0, +0, +0 + 0);    // no-op
        inc<TM,T>(0, +0 + 0,  0, +0 + 0, +b, +0 + 0, b, +0 + 0, +0, +0 + 0);    // expand north
        inc<TM,T>(0, +0 + 0,  0, +0 + 0, -b, +0 + 0, 0, +0 + 0, -b, +0 + 0);    // expand south
        inc<TM,T>(0, +0 + 0,  0, +0 + 0, +0, +0 + b, 0, +0 + b, +0, +0 + 0);    // expand east
        inc<TM,T>(0, +0 + 0,  0, +0 + 0, +0, +0 - b, 0, +0 + 0, +0, +0 - b);    // expand west
    }
    template <typename TM, typename T = typename TM::value_type::Scalar> void geoMbrInclude() {
        const auto a = (T)M_PI; // anti-meridian
        const auto b = (T)(5 * M_PI / 180); // arbitrary but distinguishable amounts
        const auto c = (T)(10 * M_PI / 180);

        //           ------- start ------- --- input -- ------ result -------
        //           n  e        s  w      lat  lon     n  e        s  w
        inc<TM,T>(b, +a - b, -b, +a - c, +b, +a + 0, b,  a + 0, -b, +a - c);    // expand eastward to anti-meridian
        inc<TM,T>(b, -a + c, -b, -a + b, +b, +a + 0, b, -a + c, -b, -a + 0);    // expand westward to anti-meridian
        inc<TM,T>(b, -a + b, -b, +a - b, +c, +a - b, c, -a + b, -b, +a - b);    // already crossing
        inc<TM,T>(b, -a + b, -b, +a - b, -c, +a - b, b, -a + b, -c, +a - b);
        inc<TM,T>(b, -a + b, -b, +a - b, +0, +a - c, b, -a + b, -b, +a - c);
        inc<TM,T>(b, -a + b, -b, +a - b, +0, -a + c, b, -a + c, -b, +a - b);
        inc<TM,T>(b, -a + c, -b, -a + b, +0, +a - b, b, -a + c, -b, +a - b);
        inc<TM,T>(b, +a - b, -b, +a - c, +0, -a + b, b, -a + b, -b, +a - c);
    }
    template <typename TM, typename T = typename TM::value_type::Scalar> void ovl(
            T n1, T s1, T e1, T w1,
            T n2, T s2, T e2, T w2, bool exp) {
        using TP = typename TM::value_type;
        const TM m1(TP(w1, s1), TP(e1, n1));
        const TM m2(TP(w2, s2), TP(e2, n2));
        assert(m1.overlaps(m2) == exp);
        assert(m2.overlaps(m1) == exp);
    }
    template <typename TM, typename T> void mbrOverlap() {
        ovl<TM,T>(0,0,0,0, 0,0,0,0, true);   // empty rects overlap
        ovl<TM,T>(0,0,0,0, 1,0,1,0, true);
        ovl<TM,T>(0,0,0,0, 1,0,0,0, true);
        ovl<TM,T>(0,0,0,0, 0,0,1,0, true);

        ovl<TM,T>(2,1,2,1, 3,2,3,2, true);   // rects with equal edges/corners overlap
        ovl<TM,T>(2,1,2,1, 3,2,1,0, true);   // (overlap has zero width and/or height)
        ovl<TM,T>(2,1,2,1, 1,0,3,2, true);
        ovl<TM,T>(2,1,2,1, 1,0,1,0, true);
        
        ovl<TM,T>(4,1,4,1, 3,2,3,2, true);   // A contains B and vice versa
        
        ovl<TM,T>(4,1,4,1, 3,2,6,0, true);   // Contained in one dimension
        ovl<TM,T>(4,1,4,1, 3,2,6,5, false);
        ovl<TM,T>(4,1,4,1, 3,2,0,0, false);

        ovl<TM,T>(4,1,4,1, 6,0,3,2, true);   // Contained in the other dimension
        ovl<TM,T>(4,1,4,1, 6,5,3,2, false);
        ovl<TM,T>(4,1,4,1, 0,0,3,2, false);
    }
    template <typename TM, typename T> void geoMbrOverlap() {
        const auto a = (T)M_PI;
        const auto b = (T)(5 * M_PI / 180);
        const auto c = (T)(10 * M_PI / 180);

        ovl<TM,T>(1,0, a  ,a-b, 1,0,-a+b,-a,   false);   // touching along the anti-meridian doesn't count (maybe it should)
        ovl<TM,T>(1,0,-a+c,a-c, 1,0,-a+b,-a-b, true);    // both crossing
        ovl<TM,T>(1,0,-a+c,a-c, 1,0, a-b, a-c, true);    // crossing and not
        ovl<TM,T>(1,0,-a+c,a-c, 1,0,-a+b,-a+c, true);    // other side

        ovl<TM,T>(1,0,-a+b,a-b, 1,0,a-c,a-b-c, false);
        ovl<TM,T>(1,0,-a+b,a-b, 1,0,-a+b+c,-a+c, false);
    }
    template <typename T> bool approx(T a, T b, T e) { return std::fabs(a-b) <= e; }
} tests;
#endif

}
