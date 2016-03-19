/*
 *  ShapeManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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
#include "ShapeManager.h"
#include "GlobeMath.h"
#include "ShapeDrawableBuilder.h"
#include "SelectionManager.h"
#include <set>
#include <vector>
#include "Identifiable.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit {

void ShapeSceneRep::enableContents(WhirlyKit::SelectionManager *selectManager, bool enable, ChangeSet &changes)
{
    for (const SimpleIdentity idIt : drawIDs){
        changes.push_back(new OnOffChangeRequest(idIt, enable));
        if (selectManager)
            for (const SimpleIdentity it : selectIDs)
                selectManager->enableSelectable(it, enable);
    }
}

void ShapeSceneRep::clearContents(SelectionManager *selectManager, ChangeSet &changes)
{
    for (const SimpleIdentity idIt : drawIDs){
        changes.push_back(new RemDrawableReq(idIt));
        if (selectManager)
            for (const SimpleIdentity it : selectIDs)
                selectManager->removeSelectable(it);
    }
}

WhirlyKitShape::WhirlyKitShape()
    : isSelectable(false), selectID(EmptyIdentity), useColor(false), color(255,255,255,255)
{
}

WhirlyKitShape::~WhirlyKitShape()
{
}

// Base shape doesn't make anything
void WhirlyKitShape::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
}

Point3d WhirlyKitShape::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, WhirlyKitShapeInfo *shapeInfo)
{
	return Point3d(0,0,0);
}

WhirlyKitSphere::WhirlyKitSphere()
    : loc(0,0), height(0.0), radius(0.0), sampleX(10), sampleY(10)
{
}

WhirlyKitSphere::~WhirlyKitSphere()
{
}

Point3d WhirlyKitSphere::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, WhirlyKit::WhirlyKitShapeInfo *shapeInfo)
{
    if (shapeInfo->getHasCenter())
        return shapeInfo->getCenter();

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);

    return dispPt;
}

static const float sqrt2 = 1.4142135623;

void WhirlyKitSphere::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    auto theColor = getUseColor() ? getColor() : regBuilder->getShapeInfo()->getColor();

    // Get the location in display coordinates
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    Point3f norm = coordAdapter->normalForLocal(localPt);

    // Run it up a bit by the height
    dispPt = dispPt + norm * height;

    // It's lame, but we'll use lat/lon coordinates to tesselate the sphere
    // Note: Replace this with something less lame
    std::vector<Point3f> locs,norms;
    locs.reserve((getSampleX()+1)*(getSampleY())+1);
    norms.reserve((getSampleX()+1)*(getSampleY()+1));
    std::vector<RGBAColor> colors;
    colors.reserve((getSampleX()+1)*(getSampleY()+1));
    Point2f geoIncr(2*M_PI/getSampleX(),M_PI/getSampleY());
	for (unsigned int iy=0;iy<getSampleY()+1;iy++) {
        for (unsigned int ix=0;ix<getSampleX()+1;ix++) {
            GeoCoord geoLoc(-M_PI+ix*geoIncr.x(),-M_PI/2.0 + iy*geoIncr.y());
            if (geoLoc.x() < -M_PI)  geoLoc.x() = -M_PI;
            if (geoLoc.x() > M_PI) geoLoc.x() = M_PI;
            if (geoLoc.y() < -M_PI/2.0)  geoLoc.y() = -M_PI/2.0;
            if (geoLoc.y() > M_PI/2.0) geoLoc.y() = M_PI/2.0;

            Point3f spherePt = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f(geoLoc.lon(),geoLoc.lat(),0.0));
            Point3f thisPt = dispPt + spherePt * radius;

            norms.push_back(spherePt);
            locs.push_back(thisPt);
            colors.push_back(theColor);
        }
    }

    // Two triangles per cell
    std::vector<BasicDrawable::Triangle> tris;
    tris.reserve(2*getSampleX()*getSampleY());
	for (unsigned int iy=0;iy<getSampleY();iy++) {
        for (unsigned int ix=0;ix<getSampleX();ix++) {
            BasicDrawable::Triangle triA,triB;
            if (regBuilder->shapeInfo->getInsideOut()) {
                // Flip the triangles
                triA.verts[0] = iy*(getSampleX()+1)+ix;
                triA.verts[2] = iy*(getSampleX()+1)+(ix+1);
                triA.verts[1] = (iy+1)*(getSampleX()+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[2] = triA.verts[1];
                triB.verts[1] = (iy+1)*(getSampleX()+1)+ix;
            }
			else {
                triA.verts[0] = iy*(getSampleX()+1)+ix;
                triA.verts[1] = iy*(getSampleX()+1)+(ix+1);
                triA.verts[2] = (iy+1)*(getSampleX()+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[1] = triA.verts[2];
                triB.verts[2] = (iy+1)*(getSampleX()+1)+ix;
            }
            tris.push_back(triA);
            tris.push_back(triB);
        }
    }

    triBuilder->addTriangles(locs,norms,colors,tris);

    // Add a selection region
    if (getSelectable() && selectManager && sceneRep) {
        Point3f pts[8];
        float dist = radius * sqrt2;
        pts[0] = dispPt + dist * Point3f(-1,-1,-1);
        pts[1] = dispPt + dist * Point3f(1,-1,-1);
        pts[2] = dispPt + dist * Point3f(1,1,-1);
        pts[3] = dispPt + dist * Point3f(-1,1,-1);
        pts[4] = dispPt + dist * Point3f(-1,-1,1);
        pts[5] = dispPt + dist * Point3f(1,-1,1);
        pts[6] = dispPt + dist * Point3f(1,1,1);
        pts[7] = dispPt + dist * Point3f(-1,1,1);
        selectManager->addSelectableRectSolid(getSelectID(),pts,triBuilder->getShapeInfo()->minVis,triBuilder->getShapeInfo()->maxVis,regBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(getSelectID());
    }
}


ShapeManager::ShapeManager(Scene *scene)
{
    pthread_mutex_init(&shapeLock, NULL);
    setScene(scene);
}

ShapeManager::~ShapeManager()
{
    for (ShapeSceneRepSet::iterator it = shapeReps.begin(); it != shapeReps.end(); ++it)
        delete *it;

    shapeReps.clear();

    pthread_mutex_destroy(&shapeLock);
}

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
SimpleIdentity ShapeManager::addShapes(std::vector<WhirlyKitShape*> shapes, WhirlyKitShapeInfo *shapeInfo, ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    ShapeSceneRep *sceneRep = new ShapeSceneRep(shapeInfo->getShapeId());
    sceneRep->fade = shapeInfo->fade;

    // Figure out a good center
    Point3d center(0,0,0);
    int numObjects = 0;
    for (auto shape : shapes) {
        center += shape->displayCenter(getScene()->getCoordAdapter(), shapeInfo);
        numObjects++;
    }
    if (numObjects > 0)
        center /= numObjects;

    ShapeDrawableBuilderTri drawBuildTri(getScene()->getCoordAdapter(),shapeInfo,center);
    ShapeDrawableBuilder drawBuildReg(getScene()->getCoordAdapter(),shapeInfo,true,center);

    // Work through the shapes
    for (auto shape : shapes) {
        shape->makeGeometryWithBuilder(&drawBuildReg, &drawBuildTri, getScene(), selectManager, sceneRep);
    }

	// Flush out remaining geometry
    drawBuildReg.flush();
    drawBuildReg.getChanges(changes, sceneRep->drawIDs);
    drawBuildTri.flush();
    drawBuildTri.getChanges(changes, sceneRep->drawIDs);

    pthread_mutex_lock(&shapeLock);
    shapeReps.insert(sceneRep);
    pthread_mutex_unlock(&shapeLock);

    return shapeInfo->getShapeId();
}

void ShapeManager::enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    pthread_mutex_lock(&shapeLock);

    for (auto shapeID : shapeIDs) {
        ShapeSceneRep dummyRep(shapeID);
        auto sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end()) {
            ShapeSceneRep *shapeRep = *sit;
            shapeRep->enableContents(selectManager, enable, changes);
        }
    }

    pthread_mutex_unlock(&shapeLock);
}

/// Remove a group of shapes named by the given ID
void ShapeManager::removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    pthread_mutex_lock(&shapeLock);

    for (auto shapeID : shapeIDs) {
        ShapeSceneRep dummyRep(shapeID);
        auto sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end()) {
            ShapeSceneRep *shapeRep = *sit;

// TODO Support fade
// Meanwhile...
			shapeRep->clearContents(selectManager, changes);
			shapeReps.erase(sit);
			delete shapeRep;
/*
            TimeInterval curTime = TimeGetCurrent();
            if (shapeRep->fade > 0.0) {
                for (SimpleIDSet::iterator idIt = shapeRep->drawIDs.begin(); idIt != shapeRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt, curTime, curTime+shapeRep->fade));

                __block void * __weak thisCanary = canary;

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, shapeRep->fade * NSEC_PER_SEC),
                                   scene->getDispatchQueue(), ^{
                    if (thisCanary) {
                        SimpleIDSet theseShapeIDs;
                        theseShapeIDs.insert(shapeID);
                        ChangeSet delChanges;
                        removeShapes(theseShapeIDs, delChanges);
                        scene->addChangeRequests(delChanges);
                    }
                });
                shapeRep->fade = 0.0;
            }
            else {
                shapeRep->clearContents(selectManager, changes);
                shapeReps.erase(sit);
                delete shapeRep;
            }
 */
        }
    }

    pthread_mutex_unlock(&shapeLock);
}

}
