/*
 *  ShapeManager.h
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting. All rights reserved.
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

/// Used internally to track shape related resources
class ShapeSceneRep : public Identifiable
{
public:
    ShapeSceneRep(){};
    ShapeSceneRep(SimpleIdentity inId): Identifiable(inId){};
    ~ShapeSceneRep(){};

    // Enable/disable the contents
    void enableContents(WhirlyKit::SelectionManager *selectManager,bool enable,ChangeSet &changes);

    // Clear the contents out of the scene
    void clearContents(WhirlyKit::SelectionManager *selectManager,ChangeSet &changes);

    SimpleIDSet drawIDs;  // Drawables created for this
    SimpleIDSet selectIDs;  // IDs in the selection layer
    float fade;  // Time to fade away for removal
};
    
typedef std::set<ShapeSceneRep *,IdentifiableSorter> ShapeSceneRepSet;
    
class WhirlyKitShape
{
public:
    WhirlyKitShape();
    virtual ~WhirlyKitShape();

	void setSelectable(bool value) { isSelectable = value; }
	bool getSelectable() { return isSelectable; }

	void setSelectID(WhirlyKit::SimpleIdentity value) { selectID = value; }
	WhirlyKit::SimpleIdentity getSelectID() { return selectID; }

	void setUseColor(bool value) { useColor = value; }
	bool getUseColor() { return useColor; }

	void setColor(WhirlyKit::RGBAColor value) { color = value; }
	WhirlyKit::RGBAColor getColor() { return color; }

	virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManager *selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, WhirlyKitShapeInfo *shapeInfo);

private:
    bool isSelectable;
    WhirlyKit::SimpleIdentity selectID;
    bool useColor;
    WhirlyKit::RGBAColor color;
};

class WhirlyKitSphere : public WhirlyKitShape
{
public:
    WhirlyKitSphere();
    virtual ~WhirlyKitSphere();

	void setLoc(WhirlyKit::GeoCoord value) { loc = value; }
	WhirlyKit::GeoCoord getLoc() { return loc; }

	void setHeight(float value) { height = value; }
	float getHeight() { return height; }

	void setRadius(float value) { radius = value; }
	float getRadius() { return radius; }

	void setSampleX(int value) { sampleX = value; }
	int getSampleX() { return sampleX; }
	void setSampleY(int value) { sampleY = value; }
	int getSampleY() { return sampleY; }

    virtual void makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, SelectionManager *selectManager, ShapeSceneRep *sceneRep);
    virtual Point3d displayCenter(CoordSystemDisplayAdapter *coordAdapter, WhirlyKitShapeInfo *shapeInfo);

private:
    WhirlyKit::GeoCoord loc;
    float height;
    float radius;
    int sampleX, sampleY;
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

    /// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
    SimpleIdentity addShapes(std::vector<WhirlyKitShape*> shapes, WhirlyKitShapeInfo * shapeInfo,ChangeSet &changes);

    /// Remove a group of shapes named by the given ID
    void removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes);

    /// Enable/disable a group of shapes
    void enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes);

protected:
    pthread_mutex_t shapeLock;
    ShapeSceneRepSet shapeReps;
};

}
