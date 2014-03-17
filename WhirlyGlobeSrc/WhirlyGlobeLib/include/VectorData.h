/*
 *  VectorData.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/11.
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

#import <math.h>
#import <vector>
#import <set>
#import <map>
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "CoordSystem.h"
#import "Dictionary.h"

namespace WhirlyKit
{
	
/// The base class for vector shapes.  All shapes
///  have attribute and an MBR.
class VectorShape : public Identifiable
{
public:	
	/// Set the attribute dictionary
	void setAttrDict(const Dictionary &newDict);
	
	/// Return the attr dict
	Dictionary *getAttrDict();
    /// Return the geoMbr
    virtual GeoMbr calcGeoMbr() = 0;
	
protected:
	VectorShape();
	virtual ~VectorShape();

	Dictionary attrDict;
};

class VectorAreal;
class VectorLinear;
class VectorPoints;
class VectorTriangles;

/// Reference counted version of the base vector shape
typedef boost::shared_ptr<VectorShape> VectorShapeRef;
/// Reference counted Areal
typedef boost::shared_ptr<VectorAreal> VectorArealRef;
/// Reference counted Linear
typedef boost::shared_ptr<VectorLinear> VectorLinearRef;
/// Reference counted Points
typedef boost::shared_ptr<VectorPoints> VectorPointsRef;
/// Reference counted triangle mesh
typedef boost::shared_ptr<VectorTriangles> VectorTrianglesRef;

/// Vector Ring is just a vector of 2D points
typedef Point2fVector VectorRing;

/// Comparison function for the vector shape.
/// This is here to ensure we don't put in the same pointer twice
struct VectorShapeRefCmp
{
    bool operator()(const VectorShapeRef &a,const VectorShapeRef &b)
    { return a.get() < b.get(); }
};
  
/// We pass the shape set around when returing a group of shapes.
/// It's a set of reference counted shapes.  You have to dynamic
///  cast to get the specfic type.  Don't forget to use the boost dynamic cast
typedef std::set<VectorShapeRef,VectorShapeRefCmp> ShapeSet;
    
/// Calculate area of a loop
float CalcLoopArea(const VectorRing &);
/// Calculate area of a loop
double CalcLoopArea(const Point2dVector &);
/// Calculate the centroid of a loop
Point2f CalcLoopCentroid(const VectorRing &loop);
    
/// Collection of triangles forming a mesh
class VectorTriangles : public VectorShape
{
public:
    /// Creation function.  Use this instead of new.
    static VectorTrianglesRef createTriangles();
    ~VectorTriangles();
    
    /// Simple triangle with three points (obviously)
    typedef struct
    {
    public:
        int pts[3];
    } Triangle;
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();
    
    // Return the given triangle as a VectorRing
    void getTriangle(int which,VectorRing &ring);
    
    // Bounding box in 2D
	GeoMbr geoMbr;

    // Shared points
    Point3fVector pts;
    // Triangles
    std::vector<Triangle> tris;
    
protected:
    VectorTriangles();
};

/// Areal feature is a list of loops.  The first is an outer loop
///  and all the rest are inner loops
class VectorAreal : public VectorShape
{
public:
    /// Creation function.  Use this instead of new
    static VectorArealRef createAreal();
    ~VectorAreal();
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();
    
    /// True if the given point is within one of the loops
    bool pointInside(GeoCoord coord);
    
    /// Sudivide to the given tolerance (in degrees)
    void subdivide(float tolerance);
        
    /// Bounding box in geographic coordinates.
	GeoMbr geoMbr;
	std::vector<VectorRing> loops;
    
protected:
    VectorAreal();
};

/// Linear feature is just a list of points that form
///  a set of edges
class VectorLinear : public VectorShape
{
public:
    /// Creation function.  Use instead of new
    static VectorLinearRef createLinear();
    ~VectorLinear();
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();

    /// Sudivide to the given tolerance (in degrees)
    void subdivide(float tolerance);

	GeoMbr geoMbr;
	VectorRing pts;
    
protected:
    VectorLinear();
};

/// The Points feature is a list of points that share attributes
///  and are otherwise unrelated.  In most cases you'll get one
///  point, but be prepared for multiple.
class VectorPoints : public VectorShape
{
public:
    /// Creation function.  Use instead of new
    static VectorPointsRef createPoints();
    ~VectorPoints();
    
    /// Return the bounding box
    virtual GeoMbr calcGeoMbr();
    
    /// Calculate the bounding box from data
    void initGeoMbr();

	GeoMbr geoMbr;
	VectorRing pts;
    
protected:
    VectorPoints();
};
    
/// A set of strings
typedef std::set<std::string> StringSet;
    
/// Break any edge longer than the given length.
/// Returns true if it broke anything
void SubdivideEdges(const VectorRing &inPts,VectorRing &outPts,bool closed,float maxLen);

/// Break any edge that deviates by the given epsilon from the surface described in
/// the display adapter;
void SubdivideEdgesToSurface(const VectorRing &inPts,VectorRing &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps);

/// Break any edge that deviates by the given epsilon from the surface described in
///  the display adapter.  But rather than using lat lon values, we'll output in
///  display coordinates and build points along the great circle.
void SubdivideEdgesToSurfaceGC(const VectorRing &inPts,Point3fVector &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps,float sphereOffset = 0.0,int minPts = 0);

/** Base class for loading a vector data file.
    Fill this into hand data over to whomever wants it.
 */
class VectorReader
{
public:
    VectorReader() { }
    virtual ~VectorReader() { }
	
    /// Return false if we failed to load
    virtual bool isValid() = 0;
	
    /// Returns one of the vector types.
    /// Keep enough state to figure out what the next one is.
    /// You can skip any attributes not named in the filter.  Or just ignore it.
    virtual VectorShapeRef getNextObject(const StringSet *filter) = 0;
    
    /// Return true if this vector reader can seek and read
    virtual bool canReadByIndex() { return false; }
    
    /// Return the total number of vectors objects
    virtual unsigned int getNumObjects() { return 0; }
    
    /// Return an object that corresponds to the given index.
    /// You need to be able to seek in your file format for this.
    /// The filter works the same as for getNextObect()
    virtual VectorShapeRef getObjectByIndex(unsigned int vecIndex,const StringSet *filter)  { return VectorShapeRef(); }
};

/** Helper routine to parse geoJSON into a collection of vectors.
    We don't know for sure what we'll get back, so you have to go
    looking through it.  Return false on parse failure.
 */
bool VectorParseGeoJSON(ShapeSet &shapes,const std::string &str);
 
/** Helper routine to parse geoJSON into a collection of vectors.
    We don't know for sure what we'll get back, so you have to go
    looking through it.  Return false on parse failure.
 */
// Note: Porting
//bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict);
    
/** Helper routine to parse a GeoJSON assembly into an array of
    collections of vectors.  This format is returned by the experimental
    OSM server for vectors.
  */
bool VectorParseGeoJSONAssembly(const std::string &str,std::map<std::string,ShapeSet> &shapes);
    
bool VectorReadFile(const std::string &fileName,ShapeSet &shapes);
bool VectorWriteFile(const std::string &fileName,ShapeSet &shapes);
    
}

