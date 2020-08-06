/*
 *  VectorObject.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/17/11.
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

#import "VectorData.h"
#import "WhirlyKitView.h"

namespace WhirlyKit
{

typedef enum {
    VectorNoneType = 0,
    VectorPointType = 1,
    VectorLinearType = 2,
    VectorLinear3dType = 3,
    VectorArealType = 4,
    VectorMultiType = 5,
} VectorObjectType;


class VectorObject;
typedef std::shared_ptr<VectorObject> VectorObjectRef;

/** @brief The C++ object we use to wrap a group of vectors and consolidate the various methods for manipulating vectors.
    @details The VectorObject stores a list of reference counted VectorShape objects.
  */
class VectorObject : public Identifiable
{
public:
    /// Construct empty
    VectorObject();

    /// Construct with an ID
    VectorObject(SimpleIdentity theId);
    
    /// @brief Return the type of vector
    VectorObjectType getVectorType();
    
    /// Set if the data is selectable
    bool isSelectable();
    void setIsSelectable(bool newSelect);
    
    /// @brief Return the attributes for the first shape or NULL
    MutableDictionaryRef getAttributes();
    void setAttributes(MutableDictionaryRef newDict);
    
    /// Make a complete company (nothing shared) and return it
    VectorObjectRef deepCopy();
    
    /// Dump everything to a string for debugging
    std::string log();
    
    /// Add a hole if this contains a single areal feature
    void addHole(const VectorRing &hole);
    
    /// Merge in vectors from the other object
    void mergeVectorsFrom(VectorObject *other);
    
    /// @brief Returns one shape per VectorObject
    void splitVectors(std::vector<VectorObject *> &vecs);
    
    /// Calculate the center (not the centroid)
    bool center(Point2d &center);
    
    /// @brief Calculate the centroid
    bool centroid(Point2d &center);
    
    /// @brief Calculate the center of the largest loop
    bool largestLoopCenter(Point2d &center,Point2d &ll,Point2d &ur);
    
    /// @brief Find the middle of a linear feature and return a rotation along that feature
    bool linearMiddle(Point2d &mid,double &rot);
    
    /// Converts to the display coordinate system before calculating
    bool linearMiddle(Point2d &mid,double &rot,CoordSystem *coordSys);
    
    /// Return the point right in the middle (index-wise) of a linear feature
    bool middleCoordinate(Point2d &mid);
    
    /// @brief Point inside polygon test
    bool pointInside(const Point2d &pt);
    
    // Fuzzy matching for selecting Linear features
    // This will project the features to the screen
    bool pointNearLinear(const Point2d &coord,float maxDistance,ViewStateRef viewState,const Point2f &frameBufferSize);
    
    /// Calculate the area of all the loops together
    double areaOfOuterLoops();
    
    /// Bounding box of all the various features together
    bool boundingBox(Point2d &ll,Point2d &ur);
    
    /**
     Subdivide the edges in this feature to a given tolerance.
     */
    void subdivideToGlobe(float epsilon);
    
    /**
     Subdivide the edges in this feature to a given tolerance, using great circle math.
     */
    void subdivideToGlobeGreatCircle(float epsilon);
    
    /**
     Subdivide the edges in this feature to a given tolerance, using great circle math.
     This version samples a great circle to display on a flat map.
     */
    void subdivideToFlatGreatCircle(float epsilon);

    /// Tesselate areal features and return a new vector object
    VectorObjectRef tesselate();

    /**
     Clip the given (presumably areal) feature(s) to a grid in radians of the given size.
     
     This will run through the loops in the input vectors and clip them against a grid.  The grid size is given in radians.
     
     @return New areal features broken up along the grid.
     */
    VectorObjectRef clipToGrid(const Point2d &gridSize);

    /**
     
     Clip the given (probably areal) features to the given bounding box.
     
     This will run through the loops of the areal features and clip them against a bounding box.
     
     The bounding box should be in the same coordinate system as the grid, probably radians.
     
     @return The new areal features will be clipped along the bounding box.
     */
    VectorObjectRef clipToMbr(const Point2d &ll,const Point2d &ur);

    /// Reproject the vectors from the source system into the destination
    /// We don't recognize units, so pass in a scaling factor
    void reproject(CoordSystem *srcSystem,double scale,CoordSystem *destSystem);

    /**
     Filter out edges created from clipping areal features on the server.
     
     In some very specific cases (OSM water) we get polygons that are obviously clipped
     along internal boundaries.  We can clear this up with some very, very specific logic.
     
     Input must be closed areals and output is linears.
     */
    VectorObjectRef filterClippedEdges();

    // Convert any linear features into areals and return a new vector object
    VectorObjectRef linearsToAreals();
    
    // Convert any areal features into linears and return a new vector object
    VectorObjectRef arealsToLinears();
    
    /// @brief Add objects form the given GeoJSON string.
    /// @param json The GeoJSON data as a std::string
    /// @return True on success, false on failure.
    bool fromGeoJSON(const std::string &json,std::string &crs);
    
    /// @brief Read objects from the given shapefile
    /// @param fileName The filename of the Shapefile
    /// @return True on success, false on failure.
    bool fromShapeFile(const std::string &fileName);
    
    /// @brief Assemblies are just concattenated JSON
    static bool FromGeoJSONAssembly(const std::string &json,std::map<std::string,VectorObject *> &vecData);

public:
    void subdivideToInternal(float epsilon,WhirlyKit::CoordSystemDisplayAdapter *adapter,bool edgeMode);
    
    bool selectable;
    ShapeSet shapes;
};

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircle(const Point2d &startPt,const Point2d &endPt,double height,Point3dVector &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,double eps);

// Sample a great circle and throw in an interpolated height at each point
void SampleGreatCircleStatic(const Point2d &startPt,const Point2d &endPt,double height,Point3dVector &pts,WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,double samples);
    
}
