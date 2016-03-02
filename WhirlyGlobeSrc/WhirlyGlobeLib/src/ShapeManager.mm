/*
 *  ShapeManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
 *  Copyright 2011-2015 mousebird consulting
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
#import "Tesselator.h"
#import "GeometryManager.h"

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

// Return the center of this object in display space.  Used for offsetting drawables.
- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    return Point3d(0,0,0);
}

// Base shape doesn't make anything
- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
}

@end

// Number of samples for a circle.
// Note: Make this a parameter
static int CircleSamples = 10;

static const float sqrt2 = 1.4142135623;

@implementation WhirlyKitCircle

- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(_loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

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
    Point3f bot,top;
    Mbr shapeMbr;
    for (unsigned int ii=0;ii<samples.size();ii++)
    {
        Point3f thisLocalPt = coordAdapter->displayToLocal(samples[ii]);
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
    if (super.isSelectable && selectManager && sceneRep)
    {
        Point3f pts[8];
        pts[0] = Point3f(bot.x(),bot.y(),bot.z());
        pts[1] = Point3f(top.x(),bot.y(),bot.z());
        pts[2] = Point3f(top.x(),top.y(),bot.z());
        pts[3] = Point3f(bot.x(),top.y(),bot.z());
        pts[4] = Point3f(bot.x(),bot.y(),top.z());
        pts[5] = Point3f(top.x(),bot.y(),top.z());
        pts[6] = Point3f(top.x(),top.y(),top.z());
        pts[7] = Point3f(bot.x(),top.y(),top.z());
        selectManager->addSelectableRectSolid(super.selectID,pts,triBuilder->getShapeInfo().minVis,triBuilder->getShapeInfo().maxVis,regBuilder->getShapeInfo().enable);
        sceneRep->selectIDs.insert(super.selectID);
    }
}

@end

@implementation WhirlyKitSphere

//static const float SphereTessX = 10;
//static const float SphereTessY = 10;

- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;
    
    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(_loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

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
    locs.reserve((_sampleX+1)*(_sampleY+1));
    norms.reserve((_sampleX+1)*(_sampleY+1));
    std::vector<RGBAColor> colors;
    colors.reserve((_sampleX+1)*(_sampleY+1));
    Point2f geoIncr(2*M_PI/_sampleX,M_PI/_sampleY);
    for (unsigned int iy=0;iy<_sampleY+1;iy++)
        for (unsigned int ix=0;ix<_sampleX+1;ix++)
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
    tris.reserve(2*_sampleX*_sampleY);
    for (unsigned int iy=0;iy<_sampleY;iy++)
        for (unsigned int ix=0;ix<_sampleX;ix++)
        {
			BasicDrawable::Triangle triA,triB;
            if (regBuilder->shapeInfo.insideOut)
            {
                // Flip the triangles
                triA.verts[0] = iy*(_sampleX+1)+ix;
                triA.verts[2] = iy*(_sampleX+1)+(ix+1);
                triA.verts[1] = (iy+1)*(_sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[2] = triA.verts[1];
                triB.verts[1] = (iy+1)*(_sampleX+1)+ix;
            } else {
                triA.verts[0] = iy*(_sampleX+1)+ix;
                triA.verts[1] = iy*(_sampleX+1)+(ix+1);
                triA.verts[2] = (iy+1)*(_sampleX+1)+(ix+1);
                triB.verts[0] = triA.verts[0];
                triB.verts[1] = triA.verts[2];
                triB.verts[2] = (iy+1)*(_sampleX+1)+ix;
            }
            tris.push_back(triA);
            tris.push_back(triB);
        }
    
    triBuilder->addTriangles(locs,norms,colors,tris);
    
    // Add a selection region
    if (super.isSelectable && selectManager && sceneRep)
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

- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(_loc);
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

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
    if (super.isSelectable && selectManager && sceneRep)
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

- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    if (!_pts.empty())
    {
        const Point3f &pt = _pts[_pts.size()/2];
        return Point3d(pt.x(),pt.y(),pt.z());
    } else
        return Point3d(0,0,0);
}

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);
    
    if (super.isSelectable)
    {
        selectManager->addSelectableLinear(super.selectID,_pts,regBuilder->getShapeInfo().minVis,regBuilder->getShapeInfo().maxVis,regBuilder->getShapeInfo().enable);
        sceneRep->selectIDs.insert(super.selectID);
    }
    
    regBuilder->addPoints(_pts, theColor, _mbr, _lineWidth, false);
}

@end

@implementation WhirlyKitShapeExtruded

- (id)init
{
    self = [super init];
    
    _transform = Eigen::Matrix4d::Identity();
    
    return self;
}

- (Point3d)displayCenter:(CoordSystemDisplayAdapter *)coordAdapter shapeInfo:(WhirlyKitShapeInfo *)shapeInfo
{
    if (shapeInfo.hasCenter)
        return shapeInfo.center;

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(_loc.x(),_loc.y()));
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    
    return dispPt;
}

- (void)makeGeometryWithBuilder:(WhirlyKit::ShapeDrawableBuilder *)regBuilder triBuilder:(WhirlyKit::ShapeDrawableBuilderTri *)triBuilder scene:(WhirlyKit::Scene *)scene selectManager:(SelectionManager *)selectManager sceneRep:(ShapeSceneRep *)sceneRep
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    RGBAColor theColor = (super.useColor ? super.color : [regBuilder->getShapeInfo().color asRGBAColor]);

    Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(_loc.x(),_loc.y()));
    Point3d dispPt = coordAdapter->localToDisplay(localPt);
    Point3d norm = coordAdapter->normalForLocal(localPt);
    Point3f norm3f(norm.x(),norm.y(),norm.z());
    
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
    shiftMat = shiftMat * _transform;
    
    // Note: Should make this bigger
    Mbr shapeMbr;
    shapeMbr.addPoint(Point2f(_loc.x(),_loc.y()));
    

    // Run around the outline, building top and bottom lists
    // Note: Should deal with doubles rather than floats
    VectorRing ring(_pts.size());
    for (unsigned int ii=0;ii<_pts.size();ii++)
    {
        const Point2d &pt = _pts[ii];
        ring[ii] = Point2f(pt.x(),pt.y());
    }
    VectorTrianglesRef trisRef = VectorTriangles::createTriangles();
    TesselateRing(ring,trisRef);
    
    std::vector<std::vector<Point3d> > polytope;
    for (unsigned int ii=0;ii<trisRef->tris.size();ii++)
    {
        VectorTriangles::Triangle &tri = trisRef->tris[ii];
        std::vector<Point3d> bot(3),top(3);
        for (unsigned int jj=0;jj<3;jj++)
        {
            const Point3f &pt = trisRef->pts[tri.pts[jj]];
            Vector4d bot4d = shiftMat * Vector4d(pt.x(),pt.y(),_loc.z()-_thickness/2.0,1.0);
            bot[jj] = Point3d(bot4d.x(),bot4d.y(),bot4d.z())/bot4d.w() + dispPt;
            Vector4d top4d = shiftMat * Vector4d(pt.x(),pt.y(),_loc.z()+_thickness/2.0,1.0);
            top[jj] = Point3d(top4d.x(),top4d.y(),top4d.z())/top4d.w() + dispPt;
        }
        triBuilder->addTriangle(top[0],norm,theColor,top[1],norm,theColor,top[2],norm,theColor,shapeMbr);
        triBuilder->addTriangle(bot[2],norm,theColor,bot[1],norm,theColor,bot[0],norm,theColor,shapeMbr);
        
        polytope.push_back(top);
        polytope.push_back(bot);
    }
    
    // Work around the outside doing sides
    for (unsigned int ii=0;ii<_pts.size();ii++)
    {
        const Point2d &p0 = _pts[ii];
        const Point2d &p1 = _pts[(ii+1)%_pts.size()];
        
        std::vector<Vector4d> pts4d(4);
        pts4d[0] = shiftMat * Vector4d(p0.x(),p0.y(),_loc.z() - _thickness/2.0,1.0);
        pts4d[1] = shiftMat * Vector4d(p1.x(),p1.y(),_loc.z() - _thickness/2.0,1.0);
        pts4d[2] = shiftMat * Vector4d(p1.x(),p1.y(),_loc.z() + _thickness/2.0,1.0);
        pts4d[3] = shiftMat * Vector4d(p0.x(),p0.y(),_loc.z() + _thickness/2.0,1.0);
        std::vector<Point3d> pts(4);
        for (unsigned int jj=0;jj<4;jj++)
            pts[jj] = Point3d(pts4d[jj].x(),pts4d[jj].y(),pts4d[jj].z())/pts4d[jj].w() + dispPt;
        polytope.push_back(pts);
        
        Point3d thisNorm = (pts[0]-pts[1]).cross(pts[2]-pts[1]);
        thisNorm.normalize();
        triBuilder->addConvexOutline(pts, thisNorm, theColor, shapeMbr);
    }
    
    // Add a selection region
    if (super.isSelectable && selectManager && sceneRep)
    {
        selectManager->addPolytope(super.selectID,polytope,triBuilder->getShapeInfo().minVis,triBuilder->getShapeInfo().maxVis,triBuilder->getShapeInfo().enable);
        sceneRep->selectIDs.insert(super.selectID);
    }
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

/// Conver the shape to form that can be used by the geometry models
void ShapeManager::convertShape(WhirlyKitShape *shape,std::vector<WhirlyKit::GeometryRaw> &rawGeom)
{
    WhirlyKitShapeInfo *shapeInfo = [[WhirlyKitShapeInfo alloc] initWithShapes:nil desc:nil];

    Point3d center(0,0,0);
    ShapeDrawableBuilderTri drawBuildTri(scene->getCoordAdapter(),shapeInfo,center);
    ShapeDrawableBuilder drawBuildReg(scene->getCoordAdapter(),shapeInfo,true,center);
    
    [shape makeGeometryWithBuilder:&drawBuildReg triBuilder:&drawBuildTri scene:scene selectManager:nil sceneRep:nil];
    
    // Scrape out the triangles
    drawBuildTri.flush();
    rawGeom.resize(1);
    GeometryRaw &outGeom = rawGeom.front();
    outGeom.type = WhirlyKitGeometryTriangles;
    outGeom.texId = EmptyIdentity;
    for (BasicDrawable *draw : drawBuildTri.drawables)
    {
        int basePts = outGeom.pts.size();
        for (const Point3f &pt : draw->points)
            outGeom.pts.push_back(Point3d(pt.x(),pt.y(),pt.z()));
        for (const BasicDrawable::Triangle &tri : draw->tris)
            outGeom.triangles.push_back(GeometryRaw::RawTriangle(tri.verts[0]+basePts,tri.verts[1]+basePts,tri.verts[2]+basePts));
    }
}

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
SimpleIdentity ShapeManager::addShapes(NSArray *shapes,NSDictionary * desc,ChangeSet &changes)
{
    WhirlyKitShapeInfo *shapeInfo = [[WhirlyKitShapeInfo alloc] initWithShapes:shapes desc:desc];
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    
    ShapeSceneRep *sceneRep = new ShapeSceneRep(shapeInfo.shapeId);
    sceneRep->fade = shapeInfo.fade;
    
    // Figure out a good center
    Point3d center(0,0,0);
    int numObjects = 0;
    for (WhirlyKitShape *shape in shapeInfo.shapes)
    {
        center += [shape displayCenter:scene->getCoordAdapter() shapeInfo:shapeInfo];
        numObjects++;
    }
    if (numObjects > 0)
        center /= numObjects;

    ShapeDrawableBuilderTri drawBuildTri(scene->getCoordAdapter(),shapeInfo,center);
    ShapeDrawableBuilder drawBuildReg(scene->getCoordAdapter(),shapeInfo,true,center);
    
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
                
                __block NSObject * __weak thisCanary = canary;

                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, shapeRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   if (thisCanary)
                                   {
                                       SimpleIDSet theseShapeIDs;
                                       theseShapeIDs.insert(shapeID);
                                       ChangeSet delChanges;
                                       removeShapes(theseShapeIDs, delChanges);
                                       scene->addChangeRequests(delChanges);
                                   }
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

