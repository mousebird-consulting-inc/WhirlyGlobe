/*
 *  ShapeManager.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2019 mousebird consulting.
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
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "SelectionManager.h"
#import "Scene.h"
#import "ShapeDrawableBuilder.h"
#include <vector>
#include <set>


namespace WhirlyKit {
    
class GeometryRaw;

/// Used internally to track shape related resources
class ShapeSceneRep : public Identifiable
{
public:
    ShapeSceneRep(){};
    ShapeSceneRep(SimpleIdentity inId): Identifiable(inId){};
    ~ShapeSceneRep(){};

    // Enable/disable the contents
    void enableContents(WhirlyKit::SelectionManagerRef &selectManager,bool enable,ChangeSet &changes);

    // Clear the contents out of the scene
    void clearContents(WhirlyKit::SelectionManagerRef &selectManager,ChangeSet &changes,TimeInterval when);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // IDs in the selection layer
    float fade;  // Time to fade away for removal
};
    
typedef std::set<ShapeSceneRep *,IdentifiableSorter> ShapeSceneRepSet;
    
/** The base class for simple shapes we'll draw on top of a globe or map.
  */
class Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Shape();
    virtual ~Shape();

	virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    bool isSelectable;
    WhirlyKit::SimpleIdentity selectID;
    bool useColor;
    WhirlyKit::RGBAColor color;
    bool clipCoords;
};


/// Ground hugging circle at a given location
class Circle : public Shape {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Circle();
    virtual ~Circle();
    
    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);
    
public:
    /// The location for the origin of the shape
    GeoCoord loc;
    /// Radius is in display units
    double radius;
    /// An offset from the globe in display units (radius of the globe = 1.0)
    double height;
    /// Number of samples to use in the circle
    int sampleX;
};

/// This puts a sphere around the location
class Sphere : public Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    Sphere();
    virtual ~Sphere();

    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    WhirlyKit::GeoCoord loc;
    float height;
    float radius;
    int sampleX, sampleY;
};
    
/// This puts a cylinder with its base at the locaton
class Cylinder : public Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Cylinder();
    virtual ~Cylinder();
    
    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    /// The location for the origin of the shape
    GeoCoord loc;
    /// Height offset from the ground (in display units)
    double baseHeight;
    /// Radius in display units
    double radius;
    /// Height in display units
    double height;
    /// Samples around the outside
    int sampleX;
};

/** A linear feature (with width) that we'll draw on
 top of a globe or map.  This is different from the
 vector layer features in that it has exact locations.
 */
class Linear : public Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Linear();
    virtual ~Linear();
    
    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    /// Bounding box in local coordinates.
    /// Note: Doesn't take height into account
    Mbr mbr;
    /// These locations are in display coordinates
    Point3dVector pts;
    /// Line width in pixels
    float lineWidth;
};
    
/** An extruded shape
 */
class Extruded : public Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    Extruded();
    virtual ~Extruded();
    
    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    /// Height and outline points are scaled by this (1/EarthRadius by default)
    double scale;
    /// The location for the origin of the shape
    Point3d loc;
    /// Points around the origin defining the shape
    Point2dVector pts;
    /// Thickness of the shape
    double thickness;
    /// Transform to apply to this extruded shape before placement
    Eigen::Matrix4d transform;
};

/** A simple rectangle.
 */
class Rectangle : public Shape
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    Rectangle();
    virtual ~Rectangle();

	void setLL(const Point3d &inLL) { ll = inLL; }
	Point3d getLL() { return ll; }

	void setUR(const Point3d &inUR) { ur = inUR; }
	Point3d getUR() { return ur; }
	
    void setTexIDs(std::vector<SimpleIdentity> inTexIDs) { texIDs = inTexIDs; }

    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManagerRef &selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo);

public:
    Point3d ll,ur;
    std::vector<WhirlyKit::SimpleIdentity> texIDs;
};

#define kWKShapeManager "WKShapeManager"


/** The Shape Manager is used to create and destroy geometry for shapes like circles, cylinders,
 and so forth.  It's entirely thread safe (except for destruction).
 */
class ShapeManager : public SceneManager
{
public:
    ShapeManager();
    virtual ~ShapeManager();
    
    /// Convert shape to raw geometry
    void convertShape(Shape &shape,std::vector<WhirlyKit::GeometryRaw> &rawGeom);

    /// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
    SimpleIdentity addShapes(std::vector<Shape*> shapes, const ShapeInfo &shapeInfo,ChangeSet &changes);

    /// Remove a group of shapes named by the given ID
    void removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes);

    /// Enable/disable a group of shapes
    void enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes);
    
    /// Pass through a uniform block to use on the given shapes
    void setUniformBlock(const SimpleIDSet &shapeIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes);

protected:
    ShapeSceneRepSet shapeReps;
};
typedef std::shared_ptr<ShapeManager> ShapeManagerRef;

}
