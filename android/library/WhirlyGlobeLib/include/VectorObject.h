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

namespace WhirlyKit
{

typedef enum {
    MaplyVectorNoneType = 0,
    MaplyVectorPointType = 1,
    MaplyVectorLinearType = 2,
    MaplyVectorLinear3dType = 3,
    MaplyVectorArealType = 4,
    MaplyVectorMultiType = 5,
} MaplyVectorObjectType;

/** @brief The C++ object we use to wrap a group of vectors and consolidate the various methods for manipulating vectors.
    @details The VectorObject stores a list of reference counted VectorShape objects.
  */
class VectorObject
{
public:
    /// @brief Construct empty
    VectorObject();
    
    /// @brief Add objects form the given GeoJSON string.
    /// @param json The GeoJSON data as a std::string
    /// @return True on success, false on failure.
    bool fromGeoJSON(const std::string &json);
    
    /// @brief Read objects from the given shapefile
    /// @param fileName The filename of the Shapefile
    /// @return True on success, false on failure.
    bool fromShapeFile(const std::string &fileName);

    /// @brief Assemblies are just concattenated JSON
    static bool FromGeoJSONAssembly(const std::string &json,std::map<std::string,VectorObject *> &vecData);
    
    /// @brief Return the attributes for the first shape or NULL
    Dictionary *getAttributes();
    
    /// @brief Returns one shape per VectorObject
    void splitVectors(std::vector<VectorObject *> &vecs);
    
    /// @brief Calculate the centroid
    bool centroid(Point2f &center);
    
    /// @brief Calculate the center of the largest loop
    bool largestLoopCenter(Point2f &center,Point2f &ll,Point2f &ur);
    
    /// @brief Find the middle of a linear feature and return a rotation along that feature
    bool linearMiddle(Point2f &mid,float &rot);
    
    /// @brief Point inside polygon test
    bool pointInside(const Point2d &pt);
    
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


    /// @brief Read from a file
    bool fromFile(const std::string &fileName);
    
    /// @brief Write to a file
    bool toFile(const std::string &file);

    /// @brief Return the type of vector
    MaplyVectorObjectType getVectorType();
    
public:
    void subdivideToInternal(float epsilon,WhirlyKit::CoordSystemDisplayAdapter *adapter,bool edgeMode);
    
    ShapeSet shapes;
};

}
