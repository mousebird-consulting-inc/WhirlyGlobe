/*
 *  ShapeManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
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

#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "ShapeManager.h"
#import "ShapeDrawableBuilder.h"
#import "SelectionManager.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
ShapeSceneRep::ShapeSceneRep()
{
}

ShapeSceneRep::ShapeSceneRep(SimpleIdentity inId)
: Identifiable(inId)
{
}

ShapeSceneRep::~ShapeSceneRep()
{
}

void ShapeSceneRep::enableContents(WhirlyKit::SelectionManager *selectManager,bool enable,ChangeSet &changeRequests)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changeRequests.push_back(new OnOffChangeRequest(*idIt,enable));
    if (selectManager)
        for (SimpleIDSet::iterator it = selectIDs.begin();it != selectIDs.end(); ++it)
            selectManager->enableSelectable(*it, enable);
}
    
void ShapeSceneRep::clearContents(SelectionManager *selectManager,ChangeSet &changeRequests)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changeRequests.push_back(new RemDrawableReq(*idIt));
    if (selectManager)
        for (SimpleIDSet::iterator it = selectIDs.begin();it != selectIDs.end(); ++it)
            selectManager->removeSelectable(*it);
}

}

@interface WhirlyKitShape()


@end

@interface WhirlyKitShape()

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep;

@end

@implementation WhirlyKitShape

// Base shape doesn't make anything
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
}

@end

// Number of samples for a circle.
// Note: Make this a parameter
static int CircleSamples = 10;

@implementation WhirlyKitCircle

// Build the geometry for a circle in display space
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);
    
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(_loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    dispPt += coordAdapter->normalForLocal(localPt) * _height;
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Construct a set of axes to build the circle around
    Point3f up = norm;
    Point3f xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3f(1,0,0);
        yAxis = Point3f(0,1,0);
    } else {
        Point3f north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }
    
    // Calculate the locations, using the axis from the center
    std::vector<Point3f> samples;
    samples.resize(CircleSamples);
    for (unsigned int ii=0;ii<CircleSamples;ii++)
        samples[ii] =  xAxis * _radius * sinf(2*M_PI*ii/(float)(CircleSamples-1)) + _radius * yAxis * cosf(2*M_PI*ii/(float)(CircleSamples-1)) + dispPt;
    
    // We need the bounding box in the local coordinate system
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3f thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    triBuilder->addConvexOutline(samples,norm,theColor,shapeMbr);
}

@end

static const float sqrt2 = 1.4142135623;

@implementation WhirlyKitSphere

// Note: We could make these parameters
static const float SphereTessX = 10;
static const float SphereTessY = 10;

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);
    
    // Get the location in display coordinates
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(_loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Run it up a bit by the height
    dispPt = dispPt + norm*_height;
    
    // It's lame, but we'll use lat/lon coordinates to tesselate the sphere
    // Note: Replace this with something less lame
    std::vector<Point3f> locs,norms;
    locs.reserve((SphereTessX+1)*(SphereTessX+1));
    norms.reserve((SphereTessX+1)*(SphereTessY+1));
    std::vector<RGBAColor> colors;
    colors.reserve((SphereTessX+1)*(SphereTessX+1));
    Point2f geoIncr(2*M_PI/SphereTessX,M_PI/SphereTessY);
    for (unsigned int iy=0;iy<SphereTessY+1;iy++)
        for (unsigned int ix=0;ix<SphereTessX+1;ix++)
        {
            GeoCoord geoLoc(-M_PI+ix*geoIncr.x(),-M_PI/2.0 + iy*geoIncr.y());
			if (geoLoc.x() < -M_PI)  geoLoc.x() = -M_PI;
			if (geoLoc.x() > M_PI) geoLoc.x() = M_PI;
			if (geoLoc.y() < -M_PI/2.0)  geoLoc.y() = -M_PI/2.0;
			if (geoLoc.y() > M_PI/2.0) geoLoc.y() = M_PI/2.0;
            
            Point3f spherePt = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3f(geoLoc.lon(),geoLoc.lat(),0.0));
            Point3f thisPt = dispPt + spherePt * _radius;
            
            norms.push_back(spherePt);
            locs.push_back(thisPt);
            colors.push_back(theColor);
        }
    
    // Two triangles per cell
    std::vector<BasicDrawable::Triangle> tris;
    tris.reserve(2*SphereTessX*SphereTessY);
    for (unsigned int iy=0;iy<SphereTessY;iy++)
        for (unsigned int ix=0;ix<SphereTessX;ix++)
        {
			BasicDrawable::Triangle triA,triB;
			triA.verts[0] = iy*(SphereTessX+1)+ix;
			triA.verts[1] = iy*(SphereTessX+1)+(ix+1);
			triA.verts[2] = (iy+1)*(SphereTessX+1)+(ix+1);
			triB.verts[0] = triA.verts[0];
			triB.verts[1] = triA.verts[2];
			triB.verts[2] = (iy+1)*(SphereTessX+1)+ix;
            tris.push_back(triA);
            tris.push_back(triB);
        }
    
    triBuilder->addTriangles(locs,norms,colors,tris);
    
    // Add a selection region
    if (super.isSelectable)
    {
        Point3f pts[8];
        float dist = _radius * sqrt2;
        pts[0] = dispPt + dist * Point3f(-1,-1,-1);
        pts[1] = dispPt + dist * Point3f(1,-1,-1);
        pts[2] = dispPt + dist * Point3f(1,1,-1);
        pts[3] = dispPt + dist * Point3f(-1,1,-1);
        pts[4] = dispPt + dist * Point3f(-1,-1,1);
        pts[5] = dispPt + dist * Point3f(1,-1,1);
        pts[6] = dispPt + dist * Point3f(1,1,1);
        pts[7] = dispPt + dist * Point3f(-1,1,1);
        selectManager->addSelectableRectSolid(super.selectID,pts,triBuilder->getShapeInfo().minVis,triBuilder->getShapeInfo().maxVis,regBuilder->getShapeInfo().enable);
        sceneRep->selectIDs.insert(super.selectID);
    }
}

@end

@implementation WhirlyKitCylinder

static std::vector<Point3f> circleSamples;

// Build the geometry for a circle in display space
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);
    
    Point3f localPt = coordAdapter->getCoordSystem()->geographicToLocal(_loc);
    Point3f dispPt = coordAdapter->localToDisplay(localPt);
    Point3f norm = coordAdapter->normalForLocal(localPt);
    
    // Move up by baseHeight
    dispPt += norm * _baseHeight;
    
    // Construct a set of axes to build the circle around
    Point3f up = norm;
    Point3f xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3f(1,0,0);
        yAxis = Point3f(0,1,0);
    } else {
        Point3f north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }
    
    // Generate the circle ones
    if (circleSamples.empty())
    {
        circleSamples.resize(CircleSamples);
        for (unsigned int ii=0;ii<CircleSamples;ii++)
            circleSamples[ii] = xAxis * sinf(2*M_PI*ii/(float)(CircleSamples-1)) + yAxis * cosf(2*M_PI*ii/(float)(CircleSamples-1));
    }
    
    // Calculate samples around the bottom
    std::vector<Point3f> samples;
    samples.resize(CircleSamples);
    for (unsigned int ii=0;ii<CircleSamples;ii++)
        samples[ii] =  _radius * circleSamples[ii] + dispPt;
    
    // We need the bounding box in the local coordinate system
    // Note: This is not handling height correctly
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3f thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    // For the top we just offset
    std::vector<Point3f> top = samples;
    for (unsigned int ii=0;ii<top.size();ii++)
    {
        Point3f &pt = top[ii];
        pt = pt + _height * norm;
    }
    triBuilder->addConvexOutline(top,norm,theColor,shapeMbr);
    
    // For the sides we'll just run things bottom to top
    for (unsigned int ii=0;ii<CircleSamples;ii++)
    {
        std::vector<Point3f> pts(4);
        pts[0] = samples[ii];
        pts[1] = samples[(ii+1)%samples.size()];
        pts[2] = top[(ii+1)%top.size()];
        pts[3] = top[ii];
        Point3f thisNorm = (pts[0]-pts[1]).cross(pts[2]-pts[1]);
        thisNorm.normalize();
        triBuilder->addConvexOutline(pts, thisNorm, theColor, shapeMbr);
    }
    
    // Note: Would be nice to keep these around
    circleSamples.clear();
    
    // Add a selection region
    if (super.isSelectable)
    {
        Point3f pts[8];
        float dist1 = _radius * sqrt2;
        pts[0] = dispPt - dist1 * xAxis - dist1 * yAxis;
        pts[1] = dispPt + dist1 * xAxis - dist1 * yAxis;
        pts[2] = dispPt + dist1 * xAxis + dist1 * yAxis;
        pts[3] = dispPt - dist1 * xAxis + dist1 * yAxis;
        pts[4] = pts[0] + _height * norm;
        pts[5] = pts[1] + _height * norm;
        pts[6] = pts[2] + _height * norm;
        pts[7] = pts[3] + _height * norm;
        selectManager->addSelectableRectSolid(super.selectID,pts,triBuilder->getShapeInfo().minVis,triBuilder->getShapeInfo().maxVis,triBuilder->getShapeInfo().enable);
        sceneRep->selectIDs.insert(super.selectID);
    }
}

@end

@implementation WhirlyKitShapeLinear

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);
    
    regBuilder->addPoints(_pts, theColor, _mbr, _lineWidth, false);
}

@end

ShapeManager::ShapeManager()
{
    pthread_mutex_init(&shapeLock, NULL);
}

ShapeManager::~ShapeManager()
{
    for (ShapeSceneRepSet::iterator it = shapeReps.begin();
         it != shapeReps.end(); ++it)
        delete *it;
    shapeReps.clear();

    pthread_mutex_destroy(&shapeLock);
}

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
SimpleIdentity ShapeManager::addShapes(NSArray *shapes,NSDictionary * desc,ChangeSet &changes)
{
    WhirlyKitShapeInfo *shapeInfo = [[WhirlyKitShapeInfo alloc] initWithShapes:shapes desc:desc];
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    
    ShapeSceneRep *sceneRep = new ShapeSceneRep(shapeInfo.shapeId);
    sceneRep->fade = shapeInfo.fade;
    
    ShapeDrawableBuilderTri drawBuildTri(scene->getCoordAdapter(),shapeInfo);
    ShapeDrawableBuilder drawBuildReg(scene->getCoordAdapter(),shapeInfo,true);
    
    // Work through the shapes
    for (WhirlyKitShape *shape in shapeInfo.shapes)
        [shape makeGeometryWithBuilder:&drawBuildReg triBuilder:&drawBuildTri scene:scene selectManager:selectManager sceneRep:sceneRep];
    
    // Flush out remaining geometry
    drawBuildReg.flush();
    drawBuildReg.getChanges(changes, sceneRep->drawIDs);
    drawBuildTri.flush();
    drawBuildTri.getChanges(changes, sceneRep->drawIDs);

    pthread_mutex_lock(&shapeLock);
    shapeReps.insert(sceneRep);
    pthread_mutex_unlock(&shapeLock);
    
    return shapeInfo.shapeId;
}

void ShapeManager::enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    
    pthread_mutex_lock(&shapeLock);

    for (SimpleIDSet::iterator it = shapeIDs.begin(); it != shapeIDs.end();++it)
    {
        SimpleIdentity shapeID = *it;
        ShapeSceneRep dummyRep(shapeID);
        ShapeSceneRepSet::iterator sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end())
        {
            ShapeSceneRep *shapeRep = *sit;
            shapeRep->enableContents(selectManager, enable, changes);
        }
    }
    
    pthread_mutex_unlock(&shapeLock);
}

/// Remove a group of shapes named by the given ID
void ShapeManager::removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);

    pthread_mutex_lock(&shapeLock);
    
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    for (SimpleIDSet::iterator it = shapeIDs.begin(); it != shapeIDs.end();++it)
    {
        SimpleIdentity shapeID = *it;
        ShapeSceneRep dummyRep(shapeID);
        ShapeSceneRepSet::iterator sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end())
        {
            ShapeSceneRep *shapeRep = *sit;
            
            if (shapeRep->fade > 0.0)
            {
                for (SimpleIDSet::iterator idIt = shapeRep->drawIDs.begin();
                     idIt != shapeRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt, curTime, curTime+shapeRep->fade));
                
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, shapeRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   SimpleIDSet theseShapeIDs;
                                   theseShapeIDs.insert(shapeID);
                                   ChangeSet delChanges;
                                   removeShapes(theseShapeIDs, delChanges);
                                   scene->addChangeRequests(delChanges);
                               }
                               );
                shapeRep->fade = 0.0;
            } else {
                shapeRep->clearContents(selectManager, changes);
                shapeReps.erase(sit);
                delete shapeRep;
            }
        }
    }
    
    pthread_mutex_unlock(&shapeLock);
}

