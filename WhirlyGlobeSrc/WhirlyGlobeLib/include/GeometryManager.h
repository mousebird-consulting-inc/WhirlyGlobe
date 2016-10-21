/*
 *  GeometryManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/25/14.
 *  Copyright 2012-2015 mousebird consulting
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
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "Scene.h"
#import "SelectionManager.h"

@class WhirlyKitGeomInfo;

namespace WhirlyKit
{
    
/** The geometry scene representation keeps track of drawables and other
 resources we've created to represent generic geometry passed in by the
 user.
 */
class GeomSceneRep : public Identifiable
{
public:
    GeomSceneRep() : fade(0.0) { }
    GeomSceneRep(SimpleIdentity theID) : Identifiable(theID) { }
    
    // Drawables created for this geometry
    SimpleIDSet drawIDs;
    
    // IDs kept with the selection manager
    SimpleIDSet selectIDs;
    
    // Bounding box (for use in instances of a base model)
    Point3d ll,ur;
    
    // If set, the amount of time to fade out before deletion
    float fade;
    
    // Remove the contents of this scene rep
    void clearContents(SelectionManager *selectManager,ChangeSet &changes);
    
    // Enable/disable contents
    void enableContents(SelectionManager *selectManager,bool enable,ChangeSet &changes);
};
    
typedef std::set<GeomSceneRep *,IdentifiableSorter> GeomSceneRepSet;

/// Types supported for raw geometry
typedef enum {WhirlyKitGeometryNone,WhirlyKitGeometryLines,WhirlyKitGeometryTriangles} WhirlyKitGeometryRawType;
    
/// Raw Geometry object.  Fill it in and pass it to the layer.
class GeometryRaw
{
public:
    GeometryRaw();
    GeometryRaw(const GeometryRaw &that);

    /// Simple triangle representation
    class RawTriangle
    {
    public:
        RawTriangle() { }
        /// Construct with three vertex indices
        RawTriangle(int v0,int v1,int v2) { verts[0] = v0; verts[1] = v1; verts[2] = v2; }
        /// Vertices are indices into a vertex array
        int verts[3];
    };
    
    // Compares type and texture ID
    bool operator == (const GeometryRaw &that) const;

    // Runs a consistency check
    bool isValid() const;

    /// Apply the given tranformation matrix to the geometry (and normals)
    void applyTransform(const Eigen::Matrix4d &mat);
    
    // How big the geometry is likely to be in a drawable
    void estimateSize(int &numPts,int &numTris);
    
    // Calculate bounding box
    void calcBounds(Point3d &ll,Point3d &ur);
    
    // Build geometry into a drawable, using the given transform
    void buildDrawables(std::vector<BasicDrawable *> &draws,const Eigen::Matrix4d &mat,const RGBAColor *colorOverride,WhirlyKitGeomInfo *geomInfo);

public:
    /// What sort of geometry this is
    WhirlyKitGeometryRawType type;
    /// The points (vertices)
    std::vector<WhirlyKit::Point3d> pts;
    /// Normals to go with the points
    std::vector<WhirlyKit::Point3d> norms;
    /// Texture coordinates, one for each point
    std::vector<WhirlyKit::TexCoord> texCoords;
    /// Colors to go with the points
    std::vector<WhirlyKit::RGBAColor> colors;
    /// The triangles, which reference points
    std::vector<RawTriangle> triangles;
    /// A texture ID for the geometry
    int texId;
};

/// Represents a single Geometry Instance
class GeometryInstance : public Identifiable
{
public:
    GeometryInstance() : mat(mat.Identity()), colorOverride(false), selectable(false), duration(0.0) { }
    
    // Center for the instance
    Point3d center;
    // End center for the instance
    Point3d endCenter;
    // Duration for the animation
    NSTimeInterval duration;
    // Rotation etc... for the instance
    Eigen::Matrix4d mat;
    // Set if we're forcing the colors in an instance
    bool colorOverride;
    RGBAColor color;
    // True if this is selectable
    bool selectable;
};
    
// Data types for geometry attributes
typedef enum {GeomRawIntType,GeomRawFloatType,GeomRawFloat2Type,GeomRawFloat3Type,GeomRawFloat4Type,GeomRawDouble2Type,GeomRawDouble3Type,GeomRawTypeMax} GeomRawDataType;

// Geometry attribute base class
class GeomPointAttrData
{
public:
    GeomPointAttrData(GeomRawDataType dataType) : dataType(dataType) { }
    std::string name;
    GeomRawDataType dataType;
    virtual int getNumVals() = 0;
    virtual ~GeomPointAttrData() { }
};

// Int geometry attribute
class GeomPointAttrDataInt : public GeomPointAttrData
{
public:
    GeomPointAttrDataInt() : GeomPointAttrData(GeomRawIntType) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataInt() { }
    std::vector<int> vals;
};

// Float geometry attribute
class GeomPointAttrDataFloat : public GeomPointAttrData
{
public:
    GeomPointAttrDataFloat() : GeomPointAttrData(GeomRawFloatType) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataFloat() { }
    std::vector<float> vals;
};

// Float2 geometry attribute
class GeomPointAttrDataPoint2f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint2f() : GeomPointAttrData(GeomRawFloat2Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint2f() { }
    std::vector<Point2f> vals;
};

// Double2 geometry attribute
class GeomPointAttrDataPoint2d : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint2d() : GeomPointAttrData(GeomRawDouble2Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint2d() { }
    std::vector<Point2d> vals;
};

// Float3 geometry attribute
class GeomPointAttrDataPoint3f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint3f() : GeomPointAttrData(GeomRawFloat3Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint3f() { }
    std::vector<Point3f> vals;
};

// Double3 geometry attribute
class GeomPointAttrDataPoint3d : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint3d() : GeomPointAttrData(GeomRawDouble3Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint3d() { }
    std::vector<Point3d> vals;
};

// Double4 geometry attribute
class GeomPointAttrDataPoint4f : public GeomPointAttrData
{
public:
    GeomPointAttrDataPoint4f() : GeomPointAttrData(GeomRawFloat4Type) { }
    int getNumVals() { return (int)vals.size(); }
    virtual ~GeomPointAttrDataPoint4f() { }
    std::vector<Eigen::Vector4f> vals;
};
    
/// An optimized version of raw geometry for points only
class GeometryRawPoints
{
public:
    GeometryRawPoints();
    ~GeometryRawPoints();
    
    // Check if we've got a consistent set of attributes
    bool valid() const;
    
    // Add an integer to the list of attributes
    void addValue(int idx,int val);
    
    // Add a single float to a list of attributes
    void addValue(int idx,float val);
    
    // Add two floats to a list of attributes
    void addPoint(int idx,const Point2f &pt);
    
    // Add three floats to a list of attributes
    void addPoint(int idx,const Point3f &pt);
    
    // Add three doubles to a list of attributes
    void addPoint(int idx,const Point3d &pt);
    
    // Add four floats to a list of attributes
    void addPoint(int idx,const Eigen::Vector4f &pt);
    
    // Add an attribute type to the point geometry
    int addAttribute(const std::string &name,GeomRawDataType dataType);
    
    // Find an attribute by name
    int findAttribute(const std::string &name) const;
    
public:
    void buildDrawables(std::vector<BasicDrawable *> &draws,const Eigen::Matrix4d &mat,WhirlyKitGeomInfo *geomInfo) const;
    
    std::vector<WhirlyKit::GeomPointAttrData *> attrData;
};

#define kWKGeometryManager "WKGeometryManager"
    
/** The Geometry manager displays of simple geometric objects,
 such as spheres, lines, and polygons.
 */
class GeometryManager : public SceneManager
{
public:
    GeometryManager();
    virtual ~GeometryManager();
    
    /// Add raw geometry at the given location
    SimpleIdentity addGeometry(std::vector<GeometryRaw> &geom,const std::vector<GeometryInstance> &instances,NSDictionary *desc,ChangeSet &changes);
    
    /// Add geometry we're planning to reuse (as a model, for example)
    SimpleIdentity addBaseGeometry(std::vector<GeometryRaw> &geom,ChangeSet &changes);
    
    /// Add instances that reuse base geometry
    SimpleIdentity addGeometryInstances(SimpleIdentity baseGeomID,const std::vector<GeometryInstance> &instances,NSDictionary *desc,ChangeSet &changes);
    
    /// Add raw geometry points.
    SimpleIdentity addGeometryPoints(const GeometryRawPoints &geomPoints,const Eigen::Matrix4d &mat,NSDictionary *desc,ChangeSet &changes);

    /// Enable/disable active billboards
    void enableGeometry(SimpleIDSet &billIDs,bool enable,ChangeSet &changes);
    
    /// Remove a group of billboards named by the given ID
    void removeGeometry(SimpleIDSet &billIDs,ChangeSet &changes);
    
protected:
    NSObject *canary;
    pthread_mutex_t geomLock;
    GeomSceneRepSet sceneReps;
};

}
