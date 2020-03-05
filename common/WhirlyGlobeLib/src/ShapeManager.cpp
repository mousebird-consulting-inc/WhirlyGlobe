/*
 *  ShapeManager.cpp
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
#import "ShapeManager.h"
#import "GlobeMath.h"
#import "ShapeDrawableBuilder.h"
#import "SelectionManager.h"
#import <set>
#import <vector>
#import "Identifiable.h"
#import "VectorData.h"
#import "Tesselator.h"
#import "GeometryManager.h"
#import "FlatMath.h"
#import "WhirlyKitLog.h"

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

void ShapeSceneRep::clearContents(SelectionManager *selectManager, ChangeSet &changes,TimeInterval when)
{
    for (const SimpleIdentity idIt : drawIDs){
        changes.push_back(new RemDrawableReq(idIt,when));
        if (selectManager)
            for (const SimpleIdentity it : selectIDs)
                selectManager->removeSelectable(it);
    }
}

Shape::Shape()
    : isSelectable(false), selectID(EmptyIdentity), useColor(false), color(255,255,255,255), clipCoords(false)
{
}

Shape::~Shape()
{
}

// Base shape doesn't make anything
void Shape::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
}

Point3d Shape::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
	return Point3d(0,0,0);
}

Circle::Circle()
: loc(0,0), radius(0.0), height(0.0), sampleX(10)    {
}
    
Circle::~Circle()
{
}
    
Point3d Circle::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}
    
static const float sqrt2 = 1.4142135623;

void Circle::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    dispPt += coordAdapter->normalForLocal(localPt) * height;
    Point3d norm = coordAdapter->normalForLocal(localPt);
    
    // Construct a set of axes to build the circle around
    Point3d up = norm;
    Point3d xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3d(1,0,0);
        yAxis = Point3d(0,1,0);
    } else {
        Point3d north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }
    
    // Calculate the locations, using the axis from the center
    Point3dVector samples;
    samples.resize(sampleX);
    for (unsigned int ii=0;ii<sampleX;ii++)
        samples[ii] =  xAxis * radius * sinf(2*M_PI*ii/(float)(sampleX-1)) + radius * yAxis * cosf(2*M_PI*ii/(float)(sampleX-1)) + dispPt;
    
    // We need the bounding box in the local coordinate system
    Point3d bot,top;
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3d thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        if (ii==0)
        {
            bot = top = thisLocalPt;
        } else {
            bot.x() = std::min(thisLocalPt.x(),bot.x());
            bot.y() = std::min(thisLocalPt.y(),bot.y());
            bot.z() = std::min(thisLocalPt.z(),bot.z());
            top.x() = std::max(thisLocalPt.x(),top.x());
            top.y() = std::max(thisLocalPt.y(),top.y());
            top.z() = std::max(thisLocalPt.z(),top.z());
        }
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    triBuilder->addConvexOutline(samples,norm,theColor,shapeMbr);
    
    // Add a selection region
    if (isSelectable && selectManager && sceneRep)
    {
        Point3d pts[8];
        pts[0] = Point3d(bot.x(),bot.y(),bot.z());
        pts[1] = Point3d(top.x(),bot.y(),bot.z());
        pts[2] = Point3d(top.x(),top.y(),bot.z());
        pts[3] = Point3d(bot.x(),top.y(),bot.z());
        pts[4] = Point3d(bot.x(),bot.y(),top.z());
        pts[5] = Point3d(top.x(),bot.y(),top.z());
        pts[6] = Point3d(top.x(),top.y(),top.z());
        pts[7] = Point3d(bot.x(),top.y(),top.z());
        selectManager->addSelectableRectSolid(selectID,pts,triBuilder->getShapeInfo()->minVis,triBuilder->getShapeInfo()->maxVis,regBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(selectID);
    }
}
    
Sphere::Sphere()
    : loc(0,0), height(0.0), radius(0.0), sampleX(10), sampleY(10)
{
}

Sphere::~Sphere()
{
}

Point3d Sphere::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);

    return dispPt;
}

void Sphere::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;

    // Get the location in display coordinates
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    Point3d norm = coordAdapter->normalForLocal(localPt);

    // Run it up a bit by the height
    dispPt = dispPt + norm * height;

    // It's lame, but we'll use lat/lon coordinates to tesselate the sphere
    Point3dVector locs,norms;
    locs.reserve((sampleX+1)*(sampleY+1));
    norms.reserve((sampleX+1)*(sampleY+1));
    std::vector<RGBAColor> colors;
    colors.reserve((sampleX+1)*(sampleY+1));
    Point2f geoIncr(2*M_PI/sampleX,M_PI/sampleY);
	for (unsigned int iy=0;iy<sampleY+1;iy++) {
        for (unsigned int ix=0;ix<sampleX+1;ix++) {
            GeoCoord geoLoc(-M_PI+ix*geoIncr.x(),-M_PI/2.0 + iy*geoIncr.y());
            if (geoLoc.x() < -M_PI)  geoLoc.x() = -M_PI;
            if (geoLoc.x() > M_PI) geoLoc.x() = M_PI;
            if (geoLoc.y() < -M_PI/2.0)  geoLoc.y() = -M_PI/2.0;
            if (geoLoc.y() > M_PI/2.0) geoLoc.y() = M_PI/2.0;

            Point3d spherePt = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d(geoLoc.lon(),geoLoc.lat(),0.0));
            Point3d thisPt = dispPt + spherePt * radius;

            norms.push_back(spherePt);
            locs.push_back(thisPt);
            colors.push_back(theColor);
        }
    }

    // Two triangles per cell
    std::vector<BasicDrawable::Triangle> tris;
    tris.reserve(2*sampleX*sampleY);
	for (unsigned int iy=0;iy<sampleY;iy++) {
        for (unsigned int ix=0;ix<sampleX;ix++) {
            BasicDrawable::Triangle triA,triB;
            if (regBuilder->shapeInfo.insideOut) {
                // Flip the triangles
                triA.verts[0] = iy*(sampleX+1)+ix;
                triA.verts[2] = iy*(sampleX+1)+(ix+1);
                triA.verts[1] = (iy+1)*(sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[2] = triA.verts[1];
                triB.verts[1] = (iy+1)*(sampleX+1)+ix;
            }
			else {
                triA.verts[0] = iy*(sampleX+1)+ix;
                triA.verts[1] = iy*(sampleX+1)+(ix+1);
                triA.verts[2] = (iy+1)*(sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[1] = triA.verts[2];
                triB.verts[2] = (iy+1)*(sampleX+1)+ix;
            }
            tris.push_back(triA);
            tris.push_back(triB);
        }
    }

    triBuilder->addTriangles(locs,norms,colors,tris);

    // Add a selection region
    if (isSelectable && selectManager && sceneRep) {
        Point3d pts[8];
        float dist = radius * sqrt2;
        pts[0] = dispPt + dist * Point3d(-1,-1,-1);
        pts[1] = dispPt + dist * Point3d(1,-1,-1);
        pts[2] = dispPt + dist * Point3d(1,1,-1);
        pts[3] = dispPt + dist * Point3d(-1,1,-1);
        pts[4] = dispPt + dist * Point3d(-1,-1,1);
        pts[5] = dispPt + dist * Point3d(1,-1,1);
        pts[6] = dispPt + dist * Point3d(1,1,1);
        pts[7] = dispPt + dist * Point3d(-1,1,1);
        selectManager->addSelectableRectSolid(selectID,pts,triBuilder->getShapeInfo()->minVis,triBuilder->getShapeInfo()->maxVis,regBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(selectID);
    }
}
    
Cylinder::Cylinder()
: loc(0,0), baseHeight(0.0), radius(0.0), height(0.0), sampleX(10)    {
}

Cylinder::~Cylinder()
{
}
    
Point3d Cylinder::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

void Cylinder::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    Point3d norm = coordAdapter->normalForLocal(localPt);
    
    // Move up by baseHeight
    dispPt += norm * baseHeight;
    
    // Construct a set of axes to build the circle around
    Point3d up = norm;
    Point3d xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3d(1,0,0);
        yAxis = Point3d(0,1,0);
    } else {
        Point3d north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(up);  xAxis.normalize();
        yAxis = up.cross(xAxis);  yAxis.normalize();
    }

    // Generate the circle based on the axes
    Point3dVector circleSamples(sampleX);
    for (unsigned int ii=0;ii<sampleX;ii++)
        circleSamples[ii] = xAxis * sinf(2*M_PI*ii/(float)(sampleX-1)) + yAxis * cosf(2*M_PI*ii/(float)(sampleX-1));
    
    // Calculate samples around the bottom
    Point3dVector samples;
    samples.resize(sampleX);
    for (unsigned int ii=0;ii<sampleX;ii++)
        samples[ii] =  radius * circleSamples[ii] + dispPt;
    
    // We need the bounding box in the local coordinate system
    // Note: This is not handling height correctly
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3d thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
        // Note: If this shape has height, this is insufficient
        shapeMbr.addPoint(Point2f(thisLocalPt.x(),thisLocalPt.y()));
    }
    
    // For the top we just offset
    Point3dVector top = samples;
    for (unsigned int ii=0;ii<top.size();ii++)
    {
        Point3d &pt = top[ii];
        pt = pt + height * norm;
    }
    triBuilder->addConvexOutline(top,norm,theColor,shapeMbr);
    
    // For the sides we'll just run things bottom to top
    for (unsigned int ii=0;ii<sampleX;ii++)
    {
        Point3dVector pts(4);
        pts[0] = samples[ii];
        pts[1] = samples[(ii+1)%samples.size()];
        pts[2] = top[(ii+1)%top.size()];
        pts[3] = top[ii];
        Point3d thisNorm = (pts[0]-pts[1]).cross(pts[2]-pts[1]);
        thisNorm.normalize();
        triBuilder->addConvexOutline(pts, thisNorm, theColor, shapeMbr);
    }
    
    circleSamples.clear();
    
    // Add a selection region
    if (isSelectable && selectManager && sceneRep)
    {
        Point3d pts[8];
        float dist1 = radius * sqrt2;
        pts[0] = dispPt - dist1 * xAxis - dist1 * yAxis;
        pts[1] = dispPt + dist1 * xAxis - dist1 * yAxis;
        pts[2] = dispPt + dist1 * xAxis + dist1 * yAxis;
        pts[3] = dispPt - dist1 * xAxis + dist1 * yAxis;
        pts[4] = pts[0] + height * norm;
        pts[5] = pts[1] + height * norm;
        pts[6] = pts[2] + height * norm;
        pts[7] = pts[3] + height * norm;
        selectManager->addSelectableRectSolid(selectID,pts,triBuilder->getShapeInfo()->minVis,triBuilder->getShapeInfo()->maxVis,triBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(selectID);
    }
}
    
Linear::Linear()
: lineWidth(0.0)
{
}

Linear::~Linear()
{
}

Point3d Linear::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;
    
    if (!pts.empty())
    {
        const Point3d &pt = pts[pts.size()/2];
        return Point3d(pt.x(),pt.y(),pt.z());
    } else
        return Point3d(0,0,0);
}

void Linear::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;

    if (isSelectable)
    {
        selectManager->addSelectableLinear(selectID,pts,regBuilder->getShapeInfo()->minVis,regBuilder->getShapeInfo()->maxVis,regBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(selectID);
    }
    
    regBuilder->addPoints(pts, theColor, mbr, lineWidth, false);
}
    
Extruded::Extruded()
: scale(1.0/EarthRadius), loc(0.0,0.0,0.0), thickness(0.0)
{
    transform = Eigen::Matrix4d::Identity();
}

Extruded::~Extruded()
{
}

Point3d Extruded::displayCenter(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter, const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(loc.x(),loc.y()));
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

void Extruded::makeGeometryWithBuilder(WhirlyKit::ShapeDrawableBuilder *regBuilder, WhirlyKit::ShapeDrawableBuilderTri *triBuilder, WhirlyKit::Scene *scene, WhirlyKit::SelectionManager *selectManager, WhirlyKit::ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(loc.x(),loc.y()));
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    Point3d norm = coordAdapter->normalForLocal(localPt);
    
    // Construct a set of axes to build the shape around
    Point3d xAxis,yAxis;
    if (coordAdapter->isFlat())
    {
        xAxis = Point3d(1,0,0);
        yAxis = Point3d(0,1,0);
    } else {
        Point3d north(0,0,1);
        // Note: Also check if we're at a pole
        xAxis = north.cross(norm);  xAxis.normalize();
        yAxis = norm.cross(xAxis);  yAxis.normalize();
    }
    
    // Set up a shift matrix that moves coordinate to the right orientation on the globe (or not)
    Matrix4d shiftMat;
    shiftMat(0,0) = xAxis.x();
    shiftMat(0,1) = yAxis.x();
    shiftMat(0,2) = norm.x();
    shiftMat(0,3) = 0.0;
    
    shiftMat(1,0) = xAxis.y();
    shiftMat(1,1) = yAxis.y();
    shiftMat(1,2) = norm.y();
    shiftMat(1,3) = 0.0;
    
    shiftMat(2,0) = xAxis.z();
    shiftMat(2,1) = yAxis.z();
    shiftMat(2,2) = norm.z();
    shiftMat(2,3) = 0.0;
    
    shiftMat(3,0) = 0.0;
    shiftMat(3,1) = 0.0;
    shiftMat(3,2) = 0.0;
    shiftMat(3,3) = 1.0;
    
    // Now add in the transform for orientation
    shiftMat = shiftMat * transform;
    
    // Note: Should make this bigger
    Mbr shapeMbr;
    shapeMbr.addPoint(Point2f(loc.x(),loc.y()));
    
    
    // Run around the outline, building top and bottom lists
    VectorRing ring(pts.size());
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        const Point2d &pt = pts[ii];
        ring[ii] = Point2f(pt.x(),pt.y());
    }
    VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
    TesselateRing(ring,trisRef);
    
    std::vector<Point3dVector> polytope;
    double z = loc.z()*scale;
    double theThickness = thickness*scale;
    for (unsigned int ii=0;ii<trisRef->tris.size();ii++)
    {
        VectorTriangles::Triangle &tri = trisRef->tris[ii];
        Point3dVector bot(3),top(3);
        for (unsigned int jj=0;jj<3;jj++)
        {
            const Point3f &pt = trisRef->pts[tri.pts[jj]];
            Vector4d bot4d = shiftMat * Vector4d(pt.x()*scale,pt.y()*scale,z-theThickness/2.0,1.0);
            bot[jj] = Point3d(bot4d.x(),bot4d.y(),bot4d.z())/bot4d.w() + dispPt;
            Vector4d top4d = shiftMat * Vector4d(pt.x()*scale,pt.y()*scale,z+theThickness/2.0,1.0);
            top[jj] = Point3d(top4d.x(),top4d.y(),top4d.z())/top4d.w() + dispPt;
        }
        triBuilder->addTriangle(top[0],norm,theColor,top[1],norm,theColor,top[2],norm,theColor,shapeMbr);
        triBuilder->addTriangle(bot[2],norm,theColor,bot[1],norm,theColor,bot[0],norm,theColor,shapeMbr);
        
        polytope.push_back(top);
        polytope.push_back(bot);
    }
    
    // Work around the outside doing sides
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        const Point2d &p0 = pts[ii];
        const Point2d &p1 = pts[(ii+1)%pts.size()];
        
        Vector4dVector pts4d(4);
        pts4d[0] = shiftMat * Vector4d(p0.x()*scale,p0.y()*scale,z - theThickness/2.0,1.0);
        pts4d[1] = shiftMat * Vector4d(p1.x()*scale,p1.y()*scale,z - theThickness/2.0,1.0);
        pts4d[2] = shiftMat * Vector4d(p1.x()*scale,p1.y()*scale,z + theThickness/2.0,1.0);
        pts4d[3] = shiftMat * Vector4d(p0.x()*scale,p0.y()*scale,z + theThickness/2.0,1.0);
        Point3dVector locPts(4);
        for (unsigned int jj=0;jj<4;jj++)
            locPts[jj] = Point3d(pts4d[jj].x(),pts4d[jj].y(),pts4d[jj].z())/pts4d[jj].w() + dispPt;
        polytope.push_back(locPts);
        
        Point3d thisNorm = (locPts[0]-locPts[1]).cross(locPts[2]-locPts[1]);
        thisNorm.normalize();
        triBuilder->addConvexOutline(locPts, thisNorm, theColor, shapeMbr);
    }
    
    // Add a selection region
    if (isSelectable && selectManager && sceneRep)
    {
        selectManager->addPolytope(selectID,polytope,triBuilder->getShapeInfo()->minVis,triBuilder->getShapeInfo()->maxVis,triBuilder->getShapeInfo()->enable);
        sceneRep->selectIDs.insert(selectID);
    }

}

Rectangle::Rectangle()
: ll(0,0,0), ur(0,0,0)
{
}
    
Rectangle::~Rectangle()
{    
}

Point3d Rectangle::displayCenter(CoordSystemDisplayAdapter *coordAdapter,const ShapeInfo &shapeInfo)
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    // Note: Do a proper center at some point
    
    return Point3d(0,0,0);
}

// Build the geometry for a circle in display space
void Rectangle::makeGeometryWithBuilder(ShapeDrawableBuilder *regBuilder,ShapeDrawableBuilderTri *triBuilder,Scene *scene,SelectionManager *selectManager,ShapeSceneRep *sceneRep)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    auto theColor = useColor ? color : regBuilder->getShapeInfo()->color;

    triBuilder->setTexIDs(texIDs);

    // Source points
    Point3dVector pts(4);
    std::vector<TexCoord> texCoords(4);
    pts[0] = Point3d(ll.x(),ll.y(),ll.z());
    pts[3] = Point3d(ur.x(),ll.y(),ur.z());
    pts[2] = Point3d(ur.x(),ur.y(),ur.z());
    pts[1] = Point3d(ll.x(),ur.y(),ll.z());

    if (clipCoords && regBuilder->sceneRender->getType() == SceneRenderer::RenderMetal) {
        // We use these for rendering from render targets and those are flipped in Metal
        texCoords[0] = TexCoord(0,1);
        texCoords[3] = TexCoord(1,1);
        texCoords[2] = TexCoord(1,0);
        texCoords[1] = TexCoord(0,0);
    } else {
        texCoords[0] = TexCoord(0,0);
        texCoords[3] = TexCoord(1,0);
        texCoords[2] = TexCoord(1,1);
        texCoords[1] = TexCoord(0,1);
    }
    
    Point3d norm;
    if (!clipCoords)
    {
        for (unsigned int ii=0;ii<4;ii++)
        {
            const Point3d &pt = pts[ii];
            Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(pt.x(),pt.y()));
            localPt.z() = pt.z();
            pts[ii] = coordAdapter->localToDisplay(localPt);
        }
        norm = coordAdapter->normalForLocal(pts[0]);
    } else {
        norm = Point3d(0,0,1);
    }
    
    // Note: Need the bounding box eventually
    Mbr shapeMbr;
    triBuilder->addConvexOutline(pts,texCoords,norm,theColor,shapeMbr);

    // Note: Should do selection too
}

ShapeManager::ShapeManager()
{
}

ShapeManager::~ShapeManager()
{
    for (ShapeSceneRepSet::iterator it = shapeReps.begin(); it != shapeReps.end(); ++it)
        delete *it;

    shapeReps.clear();
}
    
void ShapeManager::convertShape(Shape &shape,std::vector<WhirlyKit::GeometryRaw> &rawGeom)
{
    ShapeInfo shapeInfo;
    
    Point3d center(0,0,0);
    ShapeDrawableBuilderTri drawBuildTri(scene->getCoordAdapter(),renderer,shapeInfo,center);
    ShapeDrawableBuilder drawBuildReg(scene->getCoordAdapter(),renderer,shapeInfo,true,center);
    
    // Some special shapes are already in OpenGL clip space
    if (shape.clipCoords)
    {
        drawBuildTri.clipCoords = true;
    }
    shape.makeGeometryWithBuilder(&drawBuildReg,&drawBuildTri,scene,NULL,NULL);
    
    // Scrape out the triangles
    drawBuildTri.flush();
    rawGeom.resize(1);
    GeometryRaw &outGeom = rawGeom.front();
    outGeom.type = WhirlyKitGeometryTriangles;
    for (const BasicDrawableBuilderRef &draw : drawBuildTri.drawables)
    {
        int basePts = (int)outGeom.pts.size();
        outGeom.pts.reserve(draw->points.size());
        for (const Point3f &pt : draw->points)
            outGeom.pts.push_back(Point3d(pt.x(),pt.y(),pt.z()));
        outGeom.triangles.reserve(draw->tris.size());
        for (const BasicDrawable::Triangle &tri : draw->tris)
            outGeom.triangles.push_back(GeometryRaw::RawTriangle(tri.verts[0]+basePts,tri.verts[1]+basePts,tri.verts[2]+basePts));
        if (draw->basicDraw->colorEntry >= 0)
        {
            outGeom.colors.reserve(draw->points.size());
            VertexAttribute *vertAttr = draw->basicDraw->vertexAttributes[draw->basicDraw->colorEntry];
            for (int ii=0;ii<vertAttr->numElements();ii++)
            {
                RGBAColor *color = (RGBAColor *)vertAttr->addressForElement(ii);
                outGeom.colors.push_back(*color);
            }
        }
        if (draw->basicDraw->normalEntry >= 0)
        {
            outGeom.norms.reserve(draw->points.size());
            VertexAttribute *vertAttr = draw->basicDraw->vertexAttributes[draw->basicDraw->normalEntry];
            for (int ii=0;ii<vertAttr->numElements();ii++)
            {
                Point3f *norm = (Point3f *)vertAttr->addressForElement(ii);
                outGeom.norms.push_back(Point3d(norm->x(),norm->y(),norm->z()));
            }
        }
    }
}

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
SimpleIdentity ShapeManager::addShapes(std::vector<Shape*> shapes, const ShapeInfo &shapeInfo, ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    ShapeSceneRep *sceneRep = new ShapeSceneRep();
    sceneRep->fade = shapeInfo.fade;

    // Figure out a good center
    Point3d center(0,0,0);
    int numObjects = 0;
    for (auto shape : shapes) {
        center += shape->displayCenter(getScene()->getCoordAdapter(), shapeInfo);
        numObjects++;
    }
    if (numObjects > 0)
        center /= numObjects;

    ShapeDrawableBuilderTri drawBuildTri(getScene()->getCoordAdapter(),renderer,shapeInfo,center);
    ShapeDrawableBuilder drawBuildReg(getScene()->getCoordAdapter(),renderer,shapeInfo,true,center);

    // Work through the shapes
    for (auto shape : shapes) {
        if (shape->clipCoords)
            drawBuildTri.setClipCoords(true);
        else
            drawBuildTri.setClipCoords(false);
        shape->makeGeometryWithBuilder(&drawBuildReg, &drawBuildTri, getScene(), selectManager, sceneRep);
    }

	// Flush out remaining geometry
    drawBuildReg.flush();
    drawBuildReg.getChanges(changes, sceneRep->drawIDs);
    drawBuildTri.flush();
    drawBuildTri.getChanges(changes, sceneRep->drawIDs);

    SimpleIdentity shapeID = sceneRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(shapeLock);
        shapeReps.insert(sceneRep);
    }

    return shapeID;
}

void ShapeManager::enableShapes(SimpleIDSet &shapeIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    std::lock_guard<std::mutex> guardLock(shapeLock);

    for (auto shapeID : shapeIDs) {
        ShapeSceneRep dummyRep(shapeID);
        auto sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end()) {
            ShapeSceneRep *shapeRep = *sit;
            shapeRep->enableContents(selectManager, enable, changes);
        }
    }
}

/// Remove a group of shapes named by the given ID
void ShapeManager::removeShapes(SimpleIDSet &shapeIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)getScene()->getManager(kWKSelectionManager);

    std::lock_guard<std::mutex> guardLock(shapeLock);

    TimeInterval curTime = scene->getCurrentTime();
    for (auto shapeID : shapeIDs) {
        ShapeSceneRep dummyRep(shapeID);
        auto sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end()) {
            ShapeSceneRep *shapeRep = *sit;

            TimeInterval removeTime = 0.0;
            if (shapeRep->fade > 0.0) {
                for (SimpleIDSet::iterator idIt = shapeRep->drawIDs.begin(); idIt != shapeRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt, curTime, curTime+shapeRep->fade));
            }
            
			shapeRep->clearContents(selectManager, changes, removeTime);
			shapeReps.erase(sit);
			delete shapeRep;
        }
    }
}
    
void ShapeManager::setUniformBlock(const SimpleIDSet &shapeIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(shapeLock);

    for (auto shapeID : shapeIDs) {
        ShapeSceneRep dummyRep(shapeID);
        auto sit = shapeReps.find(&dummyRep);
        if (sit != shapeReps.end()) {
            for (auto drawID : (*sit)->drawIDs)
                changes.push_back(new UniformBlockSetRequest(drawID,uniBlock,bufferID));
        }
    }
}

}
