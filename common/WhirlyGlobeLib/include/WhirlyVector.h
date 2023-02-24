/*  WhirlyVector.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/18/11.
 *  Copyright 2011-2023 mousebird consulting
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

#import "WhirlyEigen.h"

#import <vector>
#import "Platform.h"

namespace WhirlyKit
{

/// Convenience wrapper for texture coordinate
struct TexCoord : public Eigen::Vector2f
{
    TexCoord() { }
    TexCoord(const TexCoord& that) : Eigen::Vector2f(that) { }
    TexCoord(const Eigen::Vector2f& that) : Eigen::Vector2f(that) { }
	TexCoord(float u,float v) : Eigen::Vector2f(u,v) { }
	float u() const { return x(); }
	float &u() { return x(); }
	float v() const { return y(); }
	float &v() { return y(); }
	using Point2f::operator=;
};

/// Convenience wrapper for geodetic coordinates
template <typename TBase>
class GeoCoordBasic : public TBase
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    using TThis = GeoCoordBasic<TBase>;
    using TScalar = typename TBase::Scalar;
    using TBase::x; // not sure why we need these...
    using TBase::y;

    GeoCoordBasic(TScalar lon = 0, TScalar lat = 0) : TBase(lon,lat) { }
    GeoCoordBasic(const TBase &other) : TBase(other) { }

    /// Longitude
    TScalar lon() const { return x(); }
    TScalar &lon() { return x(); }
    /// Latitude
    TScalar lat() const { return y(); }
    TScalar &lat() { return y(); }

    TThis operator +(const TBase &that) const { return {x()+that.x(), y()+that.y()}; }
    TThis operator -(const TBase &that) const { return {x()-that.x(), y()-that.y()}; }
    bool operator ==(const TBase &that) const { return x() == that.x() && y() == that.y(); }
    bool operator !=(const TBase &that) const { return x() != that.x() || y() != that.y(); }

    /// Create a geo coordinate using degrees instead of radians.
    /// Note the order of the arguments
    static TThis CoordFromDegrees(TScalar lon,TScalar lat) {
        return {(TScalar)(lon*M_PI/180), (TScalar)(lat*M_PI/180)};
    }
};
typedef GeoCoordBasic<Point2f> GeoCoord;
typedef GeoCoordBasic<Point2f> GeoCoordF;
typedef GeoCoordBasic<Point2d> GeoCoordD;
typedef std::vector<GeoCoord,Eigen::aligned_allocator<GeoCoord> > GeoCoordVector;

/// Color. RGBA, 8 bits per channel.
class RGBAColor
{
public:
	RGBAColor() : RGBAColor(0,0,0,0) { }

    RGBAColor(int r,int g,int b,int a) :
            r((uint8_t)r), g((uint8_t)g), b((uint8_t)b), a((uint8_t)a) { }
    RGBAColor(int r,int g,int b) :
            r((uint8_t)r), g((uint8_t)g), b((uint8_t)b), a(255) { }

	template <typename T>
    RGBAColor(const Eigen::Matrix<T,4,1> &v) :
        RGBAColor((uint8_t)(v.x()*255),(uint8_t)(v.y()*255),
                  (uint8_t)(v.z()*255),(uint8_t)(v.w()*255)) { }

    RGBAColor withAlpha(int newA) const { return RGBAColor(r,g,b,(uint8_t)newA); }
    RGBAColor withAlpha(float newA) const { return RGBAColor(r,g,b,(uint8_t)(newA * 255)); }
    RGBAColor withAlpha(double newA) const { return RGBAColor(r,g,b,(uint8_t)(newA * 255)); }

    RGBAColor withAlphaMultiply(float newA) const {
        return RGBAColor((uint8_t)(r*newA),(uint8_t)(g*newA),
                         (uint8_t)(b*newA),(uint8_t)(newA * 255));
	}
    RGBAColor withAlphaMultiply(double newA) const {
        return RGBAColor((uint8_t)(r*newA),(uint8_t)(g*newA),
                         (uint8_t)(b*newA),(uint8_t)(newA * 255));
    }

    // Create an RGBColor from unit floats
    static RGBAColor FromUnitFloats(const float *f) {
        return FromUnitFloats4(f);
    }
    static RGBAColor FromUnitFloats3(const float *f) {
        return FromUnitFloats(f[0],f[1],f[2]);
    }
    static RGBAColor FromUnitFloats4(const float *f) {
        return FromUnitFloats(f[0],f[1],f[2],f[3]);
    }
    static RGBAColor FromUnitFloats(float r, float g, float b) {
        return FromUnitFloats(r,g,b,1.0f);
    }
    static RGBAColor FromUnitFloats(float r, float g, float b, float a) {
        return RGBAColor((uint8_t)(r * 255.0f),(uint8_t)(g * 255.0f),
                         (uint8_t)(b * 255.0f),(uint8_t)(a * 255.0f));
    }
    static RGBAColor FromUnitFloats(double r, double g, double b) {
        return FromUnitFloats(r,g,b,1.0);
    }
    static RGBAColor FromUnitFloats(double r, double g, double b, double a) {
        return RGBAColor((uint8_t)(r * 255.0),(uint8_t)(g * 255.0),
                         (uint8_t)(b * 255.0),(uint8_t)(a * 255.0));
    }

    template <typename T>
    static RGBAColor FromVec(const Eigen::Matrix<T,4,1> &v) { return RGBAColor(v); }

    // Create an RGBAColor from an int
    static RGBAColor FromInt(uint32_t color) { return FromARGBInt(color); }
    static RGBAColor FromARGBInt(uint32_t color) {
	    return RGBAColor((uint8_t)((color >> 16) & 0xff),
                         (uint8_t)((color >> 8) & 0xff),
                         (uint8_t)(color & 0xff),
                         (uint8_t)(color >> 24));
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
    int asInt() const { return asARGBInt(); }
    int asARGBInt() const { return a << 24 | r << 16 | g << 8 | b; }

    /// Returns as a 4 component array of unsigned chars
    void asUChar4(unsigned char *ret) const { ret[0] = r; ret[1] = g; ret[2] = b; ret[3] = a; }

    Eigen::Vector4f asRGBAVecF() const { return {r/255.f,g/255.f,b/255.f,a/255.f}; }

    bool operator == (const RGBAColor &that) const { return (r == that.r && g == that.g && b == that.b && a == that.a); }
    bool operator != (const RGBAColor &that) const { return (r != that.r || g != that.g || b != that.b || a != that.a); }

    RGBAColor operator * (float alpha) const { return RGBAColor(r*alpha,g*alpha,b*alpha,a*alpha); }
	
	unsigned char r,g,b,a;
};
typedef std::shared_ptr<RGBAColor> RGBAColorRef;
    
class MbrD;
	
/** Bounding rectangle for plane geometry.  No special handling of geographic coordinates.
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
    Point2f ul() const { return Point2f(pt_ll.x(),pt_ur.y()); }
    /// Middle
    const Point2f mid() const { return (pt_ll+pt_ur)/2.0; }

    /// span
    Point2f span() const;

    // test if empty in either dimension
    bool empty() const { return pt_ll.x() == pt_ur.x() || pt_ll.y() == pt_ur.y(); }

    /// Check validity
	bool valid() const { return pt_ur.x() >= pt_ll.x() && pt_ur.y() >= pt_ll.y(); }
	
	/// Calculate area
	float area() const;

	/// Extend the MBR by the given point
	void addPoint(const Point2f &pt);

    /// Extend the MBR by the given point
	void addPoint(const Point2d &pt);

    /// Extend the MBR by the given points
    void addPoints(const Point2fVector &coords);
    void addPoints(const Point2f *coords, size_t count);
    template <size_t N>
    void addPoints(const Point2f (&coords)[N]) { addPoints(coords, N); }

    /// Extend the MBR by the given points
    void addPoints(const Point2dVector &coords);

	/// See if this Mbr overlaps the other one
	bool overlaps(const Mbr &that) const;

	/// Check if the given 2d point is inside this MBR
    bool inside(const Point2f &pt) const;

    /// The given MBR is contained within (or on the edge of) this one
    bool contained(const Mbr &that) { return that.insideOrOnEdge(pt_ll) && that.insideOrOnEdge(pt_ur); }
    
    /// Inside or on the edge
    bool insideOrOnEdge(const Point2f &pt) const;
    
    /// Intersection of two MBRs
    Mbr intersect(const Mbr &that) const;
    
    /// Return a list of points, for those routines that need just a list of points
    void asPoints(Point2fVector &pts) const;
    void asPoints(Point2dVector &pts) const;

    /// Expand with the given MBR
    void expand(const Mbr &that);

    /// Expands by a given fraction of the receiver's size
    void expandByFraction(double bufferZone);

    typedef Point2f value_type;

protected:
	Point2f pt_ll,pt_ur;
};

// todo: these could be combined into a template class

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

    template<std::size_t N>
    MbrD(const Point2f (&pts)[N]) : pt_ll(0,0), pt_ur(-1,-1) { addPoints(pts); }
    template<std::size_t N>
    MbrD(const Point2d (&pts)[N]) : pt_ll(0,0), pt_ur(-1,-1) { addPoints(pts); }

    bool operator == (const MbrD &that) const;
    bool operator != (const MbrD &that) const { return !operator==(that); }

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
    Point2d ul() const { return Point2d(pt_ll.x(),pt_ur.y()); }
    
    // Single dimensions
    Point2d x() const { return { pt_ll.x(), pt_ur.x() }; }
    Point2d y() const { return { pt_ll.y(), pt_ur.y() }; }

    /// Middle
    const Point2d mid() const { return (pt_ll+pt_ur)/2.0; }
    
    /// span
    Point2d span() const;

    // test if empty in either dimension
    bool empty() const { return pt_ll.x() == pt_ur.x() || pt_ll.y() == pt_ur.y(); }

    /// Check validity
    bool valid() const { return pt_ur.x() >= pt_ll.x() && pt_ur.y() >= pt_ll.y(); }

    /// Calculate area
    double area() const;
    
    /// Extend the MBR by the given point
    void addPoint(const Point2f &pt);
    
    /// Extend the MBR by the given point
    void addPoint(const Point2d &pt);
    
    /// Extend the MBR by the given points
    void addPoints(const Point2fVector &coords);
    
    /// Extend the MBR by the given points
    void addPoints(const Point2dVector &coords);

    template<std::size_t N>
    void addPoints(const Point2f (&pts)[N]) { for (std::size_t i=0;i<N;++i) addPoint(pts[i]); }
    template<std::size_t N>
    void addPoints(const Point2d (&pts)[N]) { for (std::size_t i=0;i<N;++i) addPoint(pts[i]); }

    /// See if this Mbr overlaps the other one
    bool overlaps(const MbrD &that) const;

    /// Check if the given 2d point is inside this MBR
    bool inside(const Point2d &pt) const;
    
    /// The given MBR is contained within (or on the edge of) this one
    bool contained(const MbrD &that) { return that.insideOrOnEdge(pt_ll) && that.insideOrOnEdge(pt_ur); }
    
    /// Inside or on the edge
    bool insideOrOnEdge(const Point2d &pt) const;
    
    /// Intersection of two MBRs
    MbrD intersect(const MbrD &that) const;
    
    /// Return a list of points, for those routines that need just a list of points
    void asPoints(Point2fVector &pts) const;
    void asPoints(Point2dVector &pts) const;
    
    /// Expand with the given MBR
    void expand(const MbrD &that);
    
    /// Expands by a given fraction of the receiver's size
    void expandByFraction(double bufferZone);
    
    typedef Point2d value_type;

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
    bool isValid() const { return pt_ur.y() >= pt_ll.y(); }
    
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
	GeoMbr() : pt_ll(BadVal,BadVal), pt_ur(BadVal,BadVal) { }
    GeoMbr(GeoMbr &&that) : GeoMbr(that.pt_ll, that.pt_ur) { }
    GeoMbr(const GeoMbr &that) : GeoMbr(that.pt_ll, that.pt_ur) { }
    GeoMbr(const MbrD &that) : GeoMbr(that.ll(), that.ur()) { }
    GeoMbr(Mbr &&that) : GeoMbr(that.ll(), that.ur()) { }
    GeoMbr(const Mbr &that) : GeoMbr(that.ll(), that.ur()) { }
    /// Construct with two coordinates to start
    GeoMbr(Point2f &&ll, Point2f &&ur) : pt_ll(ll), pt_ur(ur) { }
    GeoMbr(const Point2f &ll,const Point2f &ur) : pt_ll(ll), pt_ur(ur) { }
    GeoMbr(const Point2d &ll,const Point2d &ur) : pt_ll(ll.cast<float>()), pt_ur(ur.cast<float>()) { }
	/// Construct from a list of geo coordinates
	GeoMbr(const std::vector<GeoCoord> &coords);
	/// Construct with a list of 2d coordinates.  X is lon, Y is lat
	GeoMbr(const Point2fVector &pts);

    /// Resets back to invalid
    void reset() { pt_ll = GeoCoord(BadVal,BadVal);  pt_ur = GeoCoord(BadVal,BadVal); }

    /// Resets to specific values
    void reset(Point2f &&ll, Point2f &&ur) { pt_ll = ll;  pt_ur = ur; }
    void reset(const Point2f &ll, const Point2f &ur) { pt_ll = ll;  pt_ur = ur; }

	/// Fetch the lower left
	const GeoCoord &ll() const { return pt_ll; }
	GeoCoord &ll() { return pt_ll; }
    /// Fetch the upper right
	const GeoCoord &ur() const { return pt_ur; }
	GeoCoord &ur() { return pt_ur; }
    /// Fetch the lower right
    GeoCoord lr() const { return {pt_ur.x(),pt_ll.y()}; }
    /// Fetch the upper left
    GeoCoord ul() const { return {pt_ll.x(),pt_ur.y()}; }
	
	/// Construct the mid point
    GeoCoord mid() const;

    // test if empty in either dimension
    bool empty() const { return pt_ll.x() == pt_ur.x() || pt_ll.y() == pt_ur.y(); }

	/// Check the validity.  Will be invalid after construction
	bool valid() const { return pt_ll.x() != BadVal && pt_ur.x() != BadVal && pt_ll.y() <= pt_ur.y(); }

    Point2f span() const;

	/// Calculate area
	/// This is an approximation, treating the coordinates as Euclidean
	float area() const;
	
	/// Expand the MBR by this amount
    void addGeoCoord(float x, float y);
    void addGeoCoord(const Point2f &coord) { addGeoCoord(coord.x(),coord.y()); }
    void addGeoCoord(const Point2d &coord) { addGeoCoord(coord.x(),coord.y()); }
    void addGeoCoord(const Point3f &coord) { addGeoCoord(coord.x(),coord.y()); }
    void addGeoCoord(const Point3d &coord) { addGeoCoord(coord.x(),coord.y()); }

    void addPoint(const Point2f &coord) { addGeoCoord(coord); }
    void addPoint(const Point2d &coord) { addGeoCoord(coord); }

        /// Expand by a vector of 2d coordinates.  x is lon, y is lat.
    void addGeoCoords(const Point2fVector &coords);
    void addGeoCoords(const Point2dVector &coords);
    void addGeoCoords(const Point3fVector &coords);
    void addGeoCoords(const Point3dVector &coords);
    /// Expand by the vector of geo coords
    void addGeoCoords(const GeoCoordVector &coords);
	
	/// Determine overlap.
	/// This takes into account MBRs that wrap over -180/+180
	bool overlaps(const GeoMbr &that) const;

	/// See if a single geo coordinate is inside the MBR
	bool inside(const Point2f &coord) const;
    
    /// Expand this MBR by the bounds of the other one
    void expand(const GeoMbr &mbr);

    operator Mbr() const { return Mbr(pt_ll,pt_ur); }

    GeoMbr &operator=(GeoMbr &&that) { pt_ll = that.pt_ll; pt_ur = that.pt_ur; return *this; }
    GeoMbr &operator=(const GeoMbr &that) { pt_ll = that.pt_ll; pt_ur = that.pt_ur; return *this; }

    GeoMbr &operator=(Mbr &&that) { pt_ll = that.ll(); pt_ur = that.ur(); return *this; }
    GeoMbr &operator=(const Mbr &that) { pt_ll = that.ll(); pt_ur = that.ur(); return *this; }

    bool operator==(GeoMbr &&that) const { return pt_ll == that.pt_ll && pt_ur == that.pt_ur; }
    bool operator==(const GeoMbr &that) const { return pt_ll == that.pt_ll && pt_ur == that.pt_ur; }

    bool operator==(Mbr &&that) const { return pt_ll == that.ll() && pt_ur == that.ur(); }
    bool operator==(const Mbr &that) const { return pt_ll == that.ll() && pt_ur == that.ur(); }

    /// Break into one or two MBRs
	void splitIntoMbrs(std::vector<Mbr> &mbrs) const;

    static constexpr float BadVal = -1000;
    typedef Point2f value_type;

protected:
	GeoCoord pt_ll,pt_ur;
};

/// Generate a quaternion from two vectors
/// The version that comes with eigen does an epsilon check that is too large for our purposes
Eigen::Quaterniond QuatFromTwoVectors(const Point3d &a,const Point3d &b);

/// Convert a 4f matrix to a 4d matrix
inline Eigen::Matrix4d Matrix4fToMatrix4d(const Eigen::Matrix4f &inMat) { return inMat.cast<double>(); }

/// Convert a 4d matrix to a 4f matrix
inline Eigen::Matrix4f Matrix4dToMatrix4f(const Eigen::Matrix4d &inMat) { return inMat.cast<float>(); }

/// Floats to doubles
inline Eigen::Vector2d Vector2fToVector2d(const Eigen::Vector2f &inVec) { return inVec.cast<double>(); }

/// Doubles to floats
inline Eigen::Vector2f Vector2dToVector2f(const Eigen::Vector2d &inVec) { return inVec.cast<float>(); }

/// Floats to doubles
inline Eigen::Vector3d Vector3fToVector3d(const Eigen::Vector3f &inVec) { return inVec.cast<double>(); }

/// Double to floats
inline Eigen::Vector3f Vector3dToVector3f(const Eigen::Vector3d &inVec) { return inVec.cast<float>(); }

/// Floats to doubles
inline Eigen::Vector4d Vector4fToVector4d(const Eigen::Vector4f &inVec) { return inVec.cast<double>(); }

namespace detail {
    template<typename T,int N> struct PointT {
        using Type = Eigen::Matrix<T,N,1>;
        using Vector = std::vector<Type,Eigen::aligned_allocator<Type>>;
    };
}

template <typename TA,typename TB>
Eigen::Matrix<TA,2,1> Scale(const Eigen::Matrix<TA,2,1> &a, const Eigen::Matrix<TB,2,1> &b) {
    return {a.x() * b.x(), a.y() * b.y() };
}
template <typename TA,typename TB>
Eigen::Matrix<TA,3,1> Scale(const Eigen::Matrix<TA,3,1> &a, const Eigen::Matrix<TB,3,1> &b) {
    return {a.x() * b.x(), a.y() * b.y(), a.z() * b.z() };
}

/// Slice off the last component (or two)
// v[Eigen::seq(0,2)] might be better, but doesn't seem to be available in this version...
template <typename T>
Eigen::Matrix<T,2,1> Slice(const Eigen::Matrix<T,3,1> &v) { return { v.x(), v.y() }; }
template <typename T>
Eigen::Matrix<T,3,1> Slice(const Eigen::Matrix<T,4,1> &v) { return { v.x(), v.y(), v.z() }; }
template <typename T>
Eigen::Matrix<T,2,1> Slice2(const Eigen::Matrix<T,4,1> &v) { return { v.x(), v.y() }; }

/// Pad out with one (or two) additional components
template <typename T>
Eigen::Matrix<T,3,1> Pad(const Eigen::Matrix<T,2,1> &v, T z = 0) { return { v.x(), v.y(), z }; }
template <typename T>
Eigen::Matrix<T,4,1> Pad(const Eigen::Matrix<T,2,1> &v, T z, T w) { return { v.x(), v.y(), z, w }; }
template <typename T>
Eigen::Matrix<T,4,1> Pad(const Eigen::Matrix<T,3,1> &v, T w = 0) { return { v.x(), v.y(), v.z(), w }; }

/// Convert from clip-space by dividing the first three components by the fourth
template <typename T>
Eigen::Matrix<T,3,1> Clip(const Eigen::Matrix<T,4,1> &v) { return Slice(Eigen::Matrix<T,4,1>(v / v.w())); }

// Slice with arbitrary (inlined) function
template <typename TI,typename TO>
typename detail::PointT<TO,2>::Vector SliceF(const typename detail::PointT<TI,3>::Vector &iv,
        std::function<Eigen::Matrix<TO,2,1>(const Eigen::Matrix<TI,2,1>&)> f) {
    typename detail::PointT<TO,2>::Vector v;
    v.reserve(iv.size());
    for (const auto &e : iv) {
        v.emplace_back(f(Slice(e)));
    }
    return v;
}

/// Slice a vector of 3-element items to 2 elements
template <typename T>
typename detail::PointT<T,2>::Vector Slice(const typename detail::PointT<T,3>::Vector &iv) {
    return SliceF<T,T>(iv, [](auto x){ return x; });
}

/// Slice and convert type
template <typename TI,typename TO>
typename detail::PointT<TO,2>::Vector Slice(const typename detail::PointT<TI,3>::Vector &iv) {
    return SliceF<TI,TO>(iv, [](const Eigen::Matrix<TI,2,1>& x){ return x.template cast<TO>(); });
}

// Extend with arbitrary (inlined) function
template <typename TI,typename TO>
typename detail::PointT<TO,3>::Vector Pad(const typename detail::PointT<TI,2>::Vector &iv,
         std::function<Eigen::Matrix<TO,2,1>(const Eigen::Matrix<TI,2,1>&)> f, TO z = 0) {
    typename detail::PointT<TO,3>::Vector v;
    v.reserve(iv.size());
    for (const auto &e : iv) {
        v.emplace_back(Pad(f(e), z));
    }
    return v;
}

/// Extend a vector of 2-element items to 3 elements
template <typename T>
typename detail::PointT<T,3>::Vector Pad(const typename detail::PointT<T,2>::Vector &iv, T z = 0) {
    return Pad(iv, [](auto x){ return x; }, z);
}

/// Extend and convert type
template <typename TI,typename TO>
typename detail::PointT<TO,3>::Vector Pad(const typename detail::PointT<TI,2>::Vector &iv, TO z = 0) {
    return Pad(iv, [](auto x){ return x.template cast<TO>(); }, z);
}

}
