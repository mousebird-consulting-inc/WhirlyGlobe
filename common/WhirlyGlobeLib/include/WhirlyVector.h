/*
 *  WhirlyVector.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2019 mousebird consulting
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

// Note: This works around a problem in compilation for the iphone
#define EIGEN_DONT_VECTORIZE 1
//#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT 1

#import <Eigen/Eigen>
#import <vector>

namespace WhirlyKit
{

typedef Eigen::Vector3f Point3f;
typedef Eigen::Vector3d Point3d;
typedef Eigen::Vector2d Point2d;
typedef Eigen::Vector2f Point2f;
    
typedef std::vector<Point2f,Eigen::aligned_allocator<Point2f> > Point2fVector;
typedef std::vector<Point2d,Eigen::aligned_allocator<Point2d> > Point2dVector;
typedef std::vector<Point3f,Eigen::aligned_allocator<Point3f> > Point3fVector;
typedef std::vector<Point3d,Eigen::aligned_allocator<Point3d> > Point3dVector;
typedef std::vector<Eigen::Vector4f,Eigen::aligned_allocator<Eigen::Vector4f> > Vector4fVector;
typedef std::vector<Eigen::Vector4d,Eigen::aligned_allocator<Eigen::Vector4d> > Vector4dVector;
	
/// Convenience wrapper for texture coordinate
class TexCoord : public Eigen::Vector2f
{
public:
	TexCoord() { }
	TexCoord(float u,float v) : Eigen::Vector2f(u,v) { }
	float u() const { return x(); }
	float &u() { return x(); }
	float v() const { return y(); }
	float &v() { return y(); }
};

/// Convenience wrapper for geodetic coordinates
class GeoCoord : public Eigen::Vector2f
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
	GeoCoord() { }
	GeoCoord(float lon,float lat) : Eigen::Vector2f(lon,lat) { }
    /// Longitude
	float lon() const { return x(); }
	float &lon() { return x(); }
    /// Latitude
	float lat() const { return y(); }
	float &lat() { return y(); }
	GeoCoord operator + (const GeoCoord &that) { return GeoCoord(x()+that.x(),y()+that.y()); }
    bool operator == (const GeoCoord &that) { return x() == that.x() && y() == that.y(); }
    
    /// Create a geo coordinate using degrees intead of radians.
    /// Note the order of the arguments
    static GeoCoord CoordFromDegrees(float lon,float lat);
};
	
typedef std::vector<GeoCoord,Eigen::aligned_allocator<GeoCoord> > GeoCoordVector;

/// Color. RGBA, 8 bits per
class RGBAColor
{
public:
	RGBAColor() { }
	RGBAColor(unsigned char r,unsigned char g,unsigned char b,unsigned char a) : r(r), g(g), b(b), a(a) { }
	RGBAColor(unsigned char r,unsigned char g,unsigned char b) : r(r), g(g), b(b), a(255) { }
    
    // Create an RGBColor from unit floats
    static RGBAColor FromUnitFloats(float *ret) {
        return RGBAColor(ret[0] * 255.0,ret[1] * 255.0,ret[2] * 255.0,ret[3] * 255.0);
    }

    // Create an RGBAColor from an int
    static RGBAColor FromInt(int color) {
	    return RGBAColor((color >> 16) & 0xff,(color >> 8)&0xff,color&0xff, color >> 24);
	}
    
    // Create an RGBAColor from HSV
    static RGBAColor FromHSV(int hue,double s,double v) {
        double c = s * v;
        double x = c * (1 - std::abs(fmod(hue / 60.0, 2.0) - 1));
        double m = v - c;
        double rs,gs,bs;
        if (hue >= 0 && hue < 60) {
            rs= c;  gs = x;  bs = 0;
        } else if(hue >= 60 && hue < 120) {
            rs = x;  gs = c;  bs = 0;
        } else if(hue >= 120 && hue < 180) {
            rs = 0;  gs = c;  bs = x;
        } else if(hue >= 180 && hue < 240) {
            rs = 0;  gs = x;  bs = c;
        } else if(hue >= 240 && hue < 300) {
            rs = x;  gs = 0;  bs = c;
        } else {
            rs = c;  gs = 0;  bs = x;
        }
        
        return RGBAColor((rs + m) * 255.0, (gs + m) * 255.0, (bs + m) * 255.0);
    }

    // Create an RGBAColor from HSL
    static RGBAColor FromHSL(int hue,double s,double l) {
        double c = (1 - std::abs(2*l - 1)) * s;
        double x = c * (1 - std::abs(fmod(hue / 60.0, 2.0) - 1));
        double m = l - c/2.0;
        double rs,gs,bs;
        if (hue >= 0 && hue < 60) {
            rs= c;  gs = x;  bs = 0;
        } else if(hue >= 60 && hue < 120) {
            rs = x;  gs = c;  bs = 0;
        } else if(hue >= 120 && hue < 180) {
            rs = 0;  gs = c;  bs = x;
        } else if(hue >= 180 && hue < 240) {
            rs = 0;  gs = x;  bs = c;
        } else if(hue >= 240 && hue < 300) {
            rs = x;  gs = 0;  bs = c;
        } else {
            rs = c;  gs = 0;  bs = x;
        }
        
        return RGBAColor((rs + m) * 255.0, (gs + m) * 255.0, (bs + m) * 255.0);
    }

    // Standard colors to create & return

    static RGBAColor black() { return RGBAColor(0,0,0,255); }
    static RGBAColor white() { return RGBAColor(255,255,255,255); }
    static RGBAColor red() { return RGBAColor(255,0,0,255); }
    static RGBAColor green() { return RGBAColor(0,255,0,255); }
    static RGBAColor blue() { return RGBAColor(0,0,255,255); }
    static RGBAColor clear() { return RGBAColor(0,0,0,0); }

    /// Returns an an array of 4 floats
    void asUnitFloats(float *ret) const { ret[0] = (float)r / 255.0;  ret[1] = (float)g / 255.0; ret[2] = (float)b / 255.0; ret[3] = (float)a / 255.0; }
    
    /// Convert to a 32 bit integer (ala Android)
    int asInt() const { return a << 24 | r << 16 | g << 8 | b; }
    
    /// Returns as a 4 component array of unsigned chars
    void asUChar4(unsigned char *ret) const { ret[0] = r; ret[1] = g; ret[2] = b; ret[3] = a; }
    
    bool operator == (const RGBAColor &that) const { return (r == that.r && g == that.g && b == that.b && a == that.a); }
//    bool operator == (RGBAColor that) const { return (r == that.r && g == that.g && b == that.b && a == that.a); }
    RGBAColor operator * (float alpha) const { return RGBAColor(r*alpha,g*alpha,b*alpha,a*alpha); }
	
	unsigned char r,g,b,a;
};
typedef std::shared_ptr<RGBAColor> RGBAColorRef;
    
class MbrD;
	
/** Bounding rectangle.
  */
class Mbr
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Construct empty, which is marked as invalid
	Mbr() : pt_ll(0.f,0.f), pt_ur(-1.f,-1.f) { }
    /// Construct with a lower left and upper right point
	Mbr(Point2f ll,Point2f ur) : pt_ll(ll), pt_ur(ur) { }
    /// Construct from the double version
    Mbr(const MbrD &inMbr);
	/// Construct from the MBR of a vector of points
	Mbr(const Point2fVector &pts);
    
    bool operator == (const Mbr &that) const;
    
    /// Resets back to invalid
    void reset() { pt_ll = Point2f(0.f,0.f);  pt_ur = Point2f(-1.f,-1.f); }
	
    /// Lower left corner
	const Point2f &ll() const { return pt_ll; }
	Point2f &ll() { return pt_ll; }
    /// Lower right corner
    Point2f lr() const { return Point2f(pt_ur.x(),pt_ll.y()); }
    /// Upper right corner
	const Point2f &ur() const { return pt_ur; }
	Point2f &ur() { return pt_ur; }
    /// Upper left corner
    Point2f ul() { return Point2f(pt_ll.x(),pt_ur.y()); }
    /// Middle
    const Point2f mid() const { return (pt_ll+pt_ur)/2.0; }

    /// span
    Point2f span() const;

	/// Check validity
	bool valid() const { return pt_ur.x() >= pt_ll.x(); }
	
	/// Calculate area
	float area() const;

	/// Extend the MBR by the given point
	void addPoint(Point2f pt);

    /// Extend the MBR by the given point
	void addPoint(Point2d pt);

    /// Extend the MBR by the given points
    void addPoints(const Point2fVector &coords);

    /// Extend the MBR by the given points
    void addPoints(const Point2dVector &coords);

	/// See if this Mbr overlaps the other one
	bool overlaps(const Mbr &that) const;

	/// Check if the given 2d point is inside this MBR
	bool inside(Point2f pt) const { return ((pt_ll.x() < pt.x()) && (pt_ll.y() < pt.y()) && (pt.x() < pt_ur.x()) && (pt.y() < pt_ur.y())); }
    
    /// The given MBR is contained within (or on the edge of) this one
    bool contained(const Mbr &that) { return that.insideOrOnEdge(pt_ll) && that.insideOrOnEdge(pt_ur); }
    
    /// Inside or on the edge
    bool insideOrOnEdge(Point2f pt) const { return ((pt_ll.x() <= pt.x()) && (pt_ll.y() <= pt.y()) && (pt.x() <= pt_ur.x()) && (pt.y() <= pt_ur.y())); }
    
    /// Intersection of two MBRs
    Mbr intersect(const Mbr &that) const;
    
    /// Return a list of points, for those routines that need just a list of points
    void asPoints(Point2fVector &pts) const;
    void asPoints(Point2dVector &pts) const;
    
    /// Expand with the given MBR
    void expand(const Mbr &that);

    /// Expands by a given fraction of the receiver's size
    void expandByFraction(double bufferZone);

protected:
	Point2f pt_ll,pt_ur;
};
    
/** Bounding Rectangle with Doubles
  */
class MbrD
{
public:
    /// Construct empty, which is marked as invalid
    MbrD() : pt_ll(0.0,0.0), pt_ur(-1.0,-1.0) { }
    /// Construct with a lower left and upper right point
    MbrD(Point2d ll,Point2d ur) : pt_ll(ll), pt_ur(ur) { }
    /// Construct a double version from the float version
    MbrD(const Mbr &inMbr) : pt_ll(Point2d(inMbr.ll().x(),inMbr.ll().y())), pt_ur(Point2d(inMbr.ur().x(),inMbr.ur().y())) { }
    /// Construct from the MBR of a vector of points
    MbrD(const Point2dVector &pts);
    
    bool operator == (const MbrD &that) const;
    
    /// Resets back to invalid
    void reset() { pt_ll = Point2d(0.0,0.0);  pt_ur = Point2d(-1.0,-1.0); }
    
    /// Lower left corner
    const Point2d &ll() const { return pt_ll; }
    Point2d &ll() { return pt_ll; }
    /// Lower right corner
    Point2d lr() const { return Point2d(pt_ur.x(),pt_ll.y()); }
    /// Upper right corner
    const Point2d &ur() const { return pt_ur; }
    Point2d &ur() { return pt_ur; }
    /// Upper left corner
    Point2d ul() { return Point2d(pt_ll.x(),pt_ur.y()); }
    /// Middle
    const Point2d mid() const { return (pt_ll+pt_ur)/2.0; }
    
    /// span
    Point2d span() const;
    
    /// Check validity
    bool valid() const { return pt_ur.x() >= pt_ll.x(); }
    
    /// Calculate area
    float area() const;
    
    /// Extend the MBR by the given point
    void addPoint(Point2f pt);
    
    /// Extend the MBR by the given point
    void addPoint(Point2d pt);
    
    /// Extend the MBR by the given points
    void addPoints(const Point2fVector &coords);
    
    /// Extend the MBR by the given points
    void addPoints(const Point2dVector &coords);
    
    /// See if this Mbr overlaps the other one
    bool overlaps(const MbrD &that) const;

    /// Check if the given 2d point is inside this MBR
    bool inside(Point2d pt) const { return ((pt_ll.x() < pt.x()) && (pt_ll.y() < pt.y()) && (pt.x() < pt_ur.x()) && (pt.y() < pt_ur.y())); }
    
    /// The given MBR is contained within (or on the edge of) this one
    bool contained(const MbrD &that) { return that.insideOrOnEdge(pt_ll) && that.insideOrOnEdge(pt_ur); }
    
    /// Inside or on the edge
    bool insideOrOnEdge(Point2d pt) const { return ((pt_ll.x() <= pt.x()) && (pt_ll.y() <= pt.y()) && (pt.x() <= pt_ur.x()) && (pt.y() <= pt_ur.y())); }
    
    /// Intersection of two MBRs
    MbrD intersect(const MbrD &that) const;
    
    /// Return a list of points, for those routines that need just a list of points
    void asPoints(Point2dVector &pts) const;
    
    /// Expand with the given MBR
    void expand(const MbrD &that);
    
    /// Expands by a given fraction of the receiver's size
    void expandByFraction(double bufferZone);
    
protected:
    Point2d pt_ll,pt_ur;
};
    
/** Bounding box in 3D (doubles)
  */
class BBox
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    BBox() : pt_ll(0,0,0), pt_ur(-1,-1,-1) { }
    
    /// Add a point to the bounding box
    void addPoint(const Point3d &pt);
    
    /// Add a vector of points to the bounding box
    void addPoints(const Point3dVector &pts);
    
    /// Copy the corners into a vector of points
    void asPoints(Point3dVector &pts) const;
    void asPoints(Point3fVector &pts) const;
    
    // Check if the given bounding box is valid
    bool isValid() { return pt_ur.x() >= pt_ll.x(); }
    
    const Point3d &ll() const { return pt_ll; }
    const Point3d &ur() const { return pt_ur; }
        
protected:
    Point3d pt_ll,pt_ur;
};
	
/** Geographic bounding rectangle.
    Coordinates are restricted to [-180,-90]->[+180,+90], but in radians.
  */
class GeoMbr
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct invalid
	GeoMbr() : pt_ll(-1000,-1000), pt_ur(-1000,-1000) { }
    /// Construct with two coordinates to start
	GeoMbr(GeoCoord ll,GeoCoord ur) : pt_ll(ll), pt_ur(ur) { }
	/// Construct from a list of geo coordinates
	GeoMbr(const std::vector<GeoCoord> &coords);
	/// Construct with a list of 2d coordinates.  X is lon, Y is lat
	GeoMbr(const Point2fVector &pts);

    /// Resets back to invalid
    void reset() { pt_ll = GeoCoord(-1000,-1000);  pt_ur = GeoCoord(-1000,-1000); }

	/// Fetch the lower left
	const GeoCoord &ll() const { return pt_ll; }
	GeoCoord &ll() { return pt_ll; }
    /// Fetch the upper right
	const GeoCoord &ur() const { return pt_ur; }
	GeoCoord &ur() { return pt_ur; }
    /// Fetch the lower right
	GeoCoord lr() const { return GeoCoord(pt_ur.x(),pt_ll.y()); }
    /// Fetch the upper left
	GeoCoord ul() const { return GeoCoord(pt_ll.x(),pt_ur.y()); }
	
	/// Construct the mid point
	GeoCoord mid() const { return GeoCoord((pt_ll.x()+pt_ur.x())/2,(pt_ll.y()+pt_ur.y())/2); }
    
	/// Check the validity.  Will be invalid after construction
	bool valid() { return (pt_ll.x() != -1000); }

	/// Calculate area
	/// This is an approximation, treating the coordinates as Euclidean
	float area() const;
	
	/// Expand the MBR by this amount
	void addGeoCoord(const GeoCoord &coord);
        void addGeoCoord(const Point3d &coord);
	
	/// Expand by the vector of geo coords
	void addGeoCoords(const std::vector<GeoCoord> &coords);
        /// Expand by a vector of 2d coordinates.  x is lon, y is lat.
	void addGeoCoords(const Point2fVector &coords);
        void addGeoCoords(const Point3dVector &coords);
        void addGeoCoords(const GeoCoordVector &coords);
	
	/// Determine overlap.
	/// This takes into account MBRs that wrap over -180/+180
	bool overlaps(const GeoMbr &that) const;

	/// See if a single geo coordinate is inside the MBR
	bool inside(GeoCoord coord) const;
    
    /// Expand this MBR by the bounds of the other one
    void expand(const GeoMbr &mbr);
    
    operator Mbr() { return Mbr(pt_ll,pt_ur); }

    /// Break into one or two MBRs
	void splitIntoMbrs(std::vector<Mbr> &mbrs) const;

protected:
	
	GeoCoord pt_ll,pt_ur;
};
    
/// Generate a quaternion from two vectors
/// The version that comes with eigen does an epsilon check that is too large for our purposes
Eigen::Quaterniond QuatFromTwoVectors(const Point3d &a,const Point3d &b);

/// Convert a 4f matrix to a 4d matrix
Eigen::Matrix4d Matrix4fToMatrix4d(const Eigen::Matrix4f &inMat);

/// Convert a 4d matrix to a 4f matrix
Eigen::Matrix4f Matrix4dToMatrix4f(const Eigen::Matrix4d &inMat);
    
/// Floats to doubles
Eigen::Vector2d Vector2fToVector2d(const Eigen::Vector2f &inVec);
/// Doubles to floats
Eigen::Vector2f Vector2dToVector2f(const Eigen::Vector2d &inVec);
    
/// Floats to doubles
Eigen::Vector3d Vector3fToVector3d(const Eigen::Vector3f &inVec);
/// Doubles to floats
Eigen::Vector3f Vector3dToVector3f(const Eigen::Vector3d &inVec);

/// Floats to doubles
Eigen::Vector4d Vector4fToVector4d(const Eigen::Vector4f &inVec);

}
