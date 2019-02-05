/*
 *  LoftManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/30/13.
 *  Copyright 2011-2017 mousebird consulting. All rights reserved.
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

#import "LoftManager.h"
#import "VectorData.h"
#import "GridClipper.h"
#import "Tesselator.h"
#import "BaseInfo.h"
#import "SharedAttributes.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
LoftedPolyInfo::LoftedPolyInfo(const Dictionary &dict)
    : BaseInfo(dict)
{
    color = dict.getColor(MaplyColor, RGBAColor(255,255,255,255));
    height = dict.getDouble(MaplyLoftedPolyHeight, 0.01);
    base = dict.getDouble(MaplyLoftedPolyBase, 0.0);
    top = dict.getBool(MaplyLoftedPolyTop, true);
    side = dict.getBool(MaplyLoftedPolySide, true);
    outline = dict.getBool(MaplyLoftedPolyOutline, true);
    outlineColor = dict.getColor(MaplyLoftedPolyOutlineColor, RGBAColor(255,255,255,255));
    outlineWidth = dict.getDouble(MaplyLoftedPolyOutlineWidth, 1.0);
    outlineDrawPriority = dict.getInt(MaplyLoftedPolyOutlineDrawPriority,drawPriority+1);
    outlineSide = dict.getBool(MaplyLoftedPolyOutlineSide,false);
    outlineBottom = dict.getBool(MaplyLoftedPolyOutlineBottom,false);
    centered = dict.getBool(MaplyVecCentered,false);
    hasCenter = false;
    if (centered) {
        hasCenter = dict.hasField(MaplyVecCenterX) && dict.hasField(MaplyVecCenterY);
        center.x() = dict.getDouble(MaplyVecCenterX,0.0);
        center.y() = dict.getDouble(MaplyVecCenterY,0.0);
    }
}

/* Drawable Builder
 Used to construct drawables with multiple shapes in them.
 Eventually, will move this out to be a more generic object.
 */
class DrawableBuilder2
{
public:
    DrawableBuilder2(Scene *scene,ChangeSet &changes,LoftedPolySceneRep *sceneRep,
                     const LoftedPolyInfo &polyInfo,int primType,const GeoMbr &inDrawMbr)
    : scene(scene), sceneRep(sceneRep), polyInfo(polyInfo), drawable(NULL), primType(primType), changes(changes), centerValid(false), center(0,0,0), geoCenter(0,0)
    {
        drawMbr = inDrawMbr;
    }
    
    ~DrawableBuilder2()
    {
        flush();
    }
    
    void setCenter(const Point3d &newCenter,const Point2d &inGeoCenter)
    {
        centerValid = true;
        center = newCenter;
        geoCenter = inGeoCenter;
    }
    
    // Initialize or flush a drawable, as needed
    void setupDrawable(int numToAdd)
    {
        if (!drawable || (drawable->getNumPoints()+numToAdd > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            drawable = new BasicDrawable("Lofted Poly");
            drawable->setType(primType);
            // Adjust according to the vector info
            //            drawable->setOnOff(polyInfo.enable);
            //            drawable->setDrawOffset(vecInfo->drawOffset);
            drawable->setColor(((primType == GL_TRIANGLES) ? polyInfo.color : polyInfo.outlineColor));
            polyInfo.setupBasicDrawable(drawable);
            if (primType == GL_LINES)
                drawable->setLineWidth(polyInfo.outlineWidth);
            drawable->setRequestZBuffer(polyInfo.zBufferRead);
            drawable->setWriteZBuffer(polyInfo.zBufferWrite);
        }
    }
    
    // Add a triangle, keeping track of limits
    void addLoftTriangle(Point2f verts[3],float height)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        setupDrawable(3);
        
        int startVert = drawable->getNumPoints();
        for (unsigned int ii=0;ii<3;ii++)
        {
            // Get some real world coordinates and corresponding normal
            Point2f &geoPt = verts[ii];
            Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
            Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
            Point3d dispPt = coordAdapter->localToDisplay(localPt);
            Point3d norm = coordAdapter->normalForLocal(localPt);
            Point3d pt1 = dispPt + norm * height - center;
            
            drawable->addPoint(pt1);
            drawable->addNormal(norm);
        }
        
        BasicDrawable::Triangle tri;
        tri.verts[0] = startVert;
        tri.verts[1] = startVert+1;
        tri.verts[2] = startVert+2;
        drawable->addTriangle(tri);
    }
    
    // Add a whole mess of triangles, adding
    //  in the height
    void addPolyGroup(VectorTrianglesRef mesh)
    {
        for (unsigned int ii=0;ii<mesh->tris.size();ii++)
        {
            VectorRing tri;
            mesh->getTriangle(ii, tri);
            if (tri.size() == 3)
            {
                Point2f verts[3];
                verts[2] = tri[0];  verts[1] = tri[1];  verts[0] = tri[2];
                addLoftTriangle(verts,polyInfo.height);
                // If they've got a base, we want to see it from the underside, probably
                if (polyInfo.base > 0.0)
                {
                    Point2f verts[3];
                    verts[1] = tri[0];  verts[2] = tri[1];  verts[0] = tri[2];
                    addLoftTriangle(verts,polyInfo.base);
                }
            }
        }
    }
    
    // Add a set of outlines
    void addOutline(std::vector<VectorRing> &rings,bool useHeight)
    {
        if (primType != GL_LINES)
            return;
        double height = (useHeight ? polyInfo.height : 0.0);
        
        setupDrawable(0);
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        for (unsigned int ii=0;ii<rings.size();ii++)
        {
            VectorRing &verts = rings[ii];
            Point3d prevPt,prevNorm,firstPt,firstNorm;
            for (unsigned int jj=0;jj<verts.size();jj++)
            {
                // Convert to real world coordinates and offset from the globe
                Point2f &geoPt = verts[jj];
                Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
                Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
                Point3d dispPt = coordAdapter->localToDisplay(localPt);
                Point3d norm = coordAdapter->normalForLocal(localPt);
                Point3d pt = dispPt + norm * height - center;
                
                // Add to drawable
                // Depending on the type, we do this differently
                if (jj > 0)
                {
                    drawable->addPoint(prevPt);
                    drawable->addPoint(pt);
                    drawable->addNormal(prevNorm);
                    drawable->addNormal(norm);
                } else {
                    firstPt = pt;
                    firstNorm = norm;
                }
                prevPt = pt;
                prevNorm = norm;
            }
            
            // Close the loop
            {
                drawable->addPoint(prevPt);
                drawable->addPoint(firstPt);
                drawable->addNormal(prevNorm);
                drawable->addNormal(firstNorm);
            }
        }
    }
    
    void addSkirtPoints(VectorRing &pts)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = (int)(4*(pts.size()+1));
        setupDrawable(ptCount);
        
        Point3d prevPt0,prevPt1,prevNorm,firstPt0,firstPt1,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Get some real world coordinates and corresponding normal
            Point2f &geoPt = pts[jj];
            Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
            Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
            Point3d norm = coordAdapter->normalForLocal(localPt);
            Point3d pt0 = coordAdapter->localToDisplay(localPt);
            Point3d pt1 = pt0 + norm * polyInfo.height;
            if (polyInfo.base > 0.0)
                pt0 = pt0 + norm * polyInfo.base;
            
            // Add to drawable
            if (jj > 0)
            {
                int startVert = drawable->getNumPoints();
                drawable->addPoint((Point3d)(prevPt0-center));
                drawable->addPoint((Point3d)(prevPt1-center));
                drawable->addPoint((Point3d)(pt1-center));
                drawable->addPoint((Point3d)(pt0-center));
                
                // Normal points out
                Point3d crossNorm = norm.cross(pt1-prevPt1);
                crossNorm.normalize();
                crossNorm *= -1;
                
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                drawable->addNormal(crossNorm);
                
                BasicDrawable::Triangle triA,triB;
                triA.verts[0] = startVert+0;
                triA.verts[1] = startVert+1;
                triA.verts[2] = startVert+3;
                triB.verts[0] = startVert+1;
                triB.verts[1] = startVert+2;
                triB.verts[2] = startVert+3;
                
                drawable->addTriangle(triA);
                drawable->addTriangle(triB);
            } else {
                firstPt0 = pt0;
                firstPt1 = pt1;
                firstNorm = norm;
            }
            prevPt0 = pt0;  prevPt1 = pt1;
            prevNorm = norm;
        }
        
        // Close the loop
        if (primType == GL_LINES)
        {
            int startVert = drawable->getNumPoints();
            drawable->addPoint((Point3d)(prevPt0-center));
            drawable->addPoint((Point3d)(prevPt1-center));
            drawable->addPoint((Point3d)(firstPt1-center));
            drawable->addPoint((Point3d)(firstPt0-center));
            
            Point3d crossNorm = prevNorm.cross(firstPt1-prevPt1);
            crossNorm *= -1;
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);
            drawable->addNormal(crossNorm);
            
            BasicDrawable::Triangle triA,triB;
            triA.verts[0] = startVert+0;
            triA.verts[1] = startVert+1;
            triA.verts[2] = startVert+3;
            triB.verts[0] = startVert+1;
            triB.verts[1] = startVert+2;
            triB.verts[2] = startVert+3;
        }
    }
    
    void addUprights(VectorRing &pts)
    {
        if (primType != GL_LINES)
            return;
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = (int)(2*pts.size());
        setupDrawable(ptCount);
        
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Get some real world coordinates and corresponding normal
            Point2f &geoPt = pts[jj];
            Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
            Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
            Point3d norm = coordAdapter->normalForLocal(localPt);
            Point3d pt0 = coordAdapter->localToDisplay(localPt);
            Point3d pt1 = pt0 + norm * polyInfo.height;
            if (polyInfo.base > 0.0)
                pt0 = pt0 + norm * polyInfo.base;
            
            // Just do the uprights as lines
            drawable->addPoint(pt0);
            drawable->addNormal(pt0);
            drawable->addPoint(pt1);
            drawable->addNormal(pt0);
        }
    }
    
    void flush()
    {
        if (drawable)
        {
            if (drawable->getNumPoints() > 0)
            {
                drawable->setLocalMbr(Mbr(Point2f(drawMbr.ll().x(),drawMbr.ll().y()),Point2f(drawMbr.ur().x(),drawMbr.ur().y())));
                if (centerValid)
                {
                    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
                if (polyInfo.fade > 0)
                {
                    TimeInterval curTime = TimeGetCurrent();
                    drawable->setFade(curTime,curTime+polyInfo.fade);
                }
                sceneRep->drawIDs.insert(drawable->getId());
                changes.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:   
    Scene *scene;
    LoftedPolySceneRep *sceneRep;
    ChangeSet &changes;
    GeoMbr drawMbr;
    BasicDrawable *drawable;
    const LoftedPolyInfo &polyInfo;
    GLenum primType;
    Point3d center;
    Point2d geoCenter;
    bool applyCenter;
    bool centerValid;
};
    
    
LoftManager::LoftManager()
{
    pthread_mutex_init(&loftLock, NULL);
}
LoftManager::~LoftManager()
{
    pthread_mutex_destroy(&loftLock);
    
    for (LoftedPolySceneRepSet::iterator it = loftReps.begin();
         it != loftReps.end(); ++it)
        delete *it;
    loftReps.clear();
}
    
// From a scene rep and a description, add the given polygons to the drawable builder
void LoftManager::addGeometryToBuilder(LoftedPolySceneRep *sceneRep,const LoftedPolyInfo &polyInfo,GeoMbr &drawMbr,Point3d &center,bool centerValid,Point2d &geoCenter,ChangeSet &changes)
{
    int numShapes = 0;
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    DrawableBuilder2 drawBuild(scene,changes,sceneRep,polyInfo,GL_TRIANGLES,drawMbr);
    if (centerValid)
        drawBuild.setCenter(center,geoCenter);
    
    // Toss in the polygons for the sides
    if (polyInfo.height != 0.0)
    {
        DrawableBuilder2 drawBuild2(scene,changes,sceneRep,polyInfo,GL_LINES,drawMbr);

        for (ShapeSet::iterator it = sceneRep->shapes.begin();
             it != sceneRep->shapes.end(); ++it)
        {
            VectorArealRef theAreal = std::dynamic_pointer_cast<VectorAreal>(*it);
            if (theAreal.get())
            {
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    if (polyInfo.side)
                    {
                        drawBuild.addSkirtPoints(theAreal->loops[ri]);
                        numShapes++;
                        
                        // Do the uprights around the side
                        if (polyInfo.outlineSide)
                            drawBuild2.addUprights(theAreal->loops[ri]);
                    }
                }
            }
        }
    }
    
    // Tweak the mesh polygons and toss 'em in
    if (polyInfo.top)
        drawBuild.addPolyGroup(sceneRep->triMesh);
        
    // And do the top outline if it's there
    if (polyInfo.outline || polyInfo.outlineBottom)
    {
        DrawableBuilder2 drawBuild2(scene,changes,sceneRep,polyInfo,GL_LINES,drawMbr);
        if (centerValid)
            drawBuild2.setCenter(center,geoCenter);
        if (polyInfo.outline)
            drawBuild2.addOutline(sceneRep->outlines,true);
        if (polyInfo.outlineBottom)
            drawBuild2.addOutline(sceneRep->outlines,false);
        
        sceneRep->outlines.clear();
    }
    
    //    printf("Added %d shapes and %d triangles from mesh\n",(int)numShapes,(int)sceneRep->triMesh.size());        
}

    
/// Add lofted polygons
SimpleIdentity LoftManager::addLoftedPolys(WhirlyKit::ShapeSet *shapes,const LoftedPolyInfo &polyInfo,float gridSize,ChangeSet &changes)
{
    SimpleIdentity loftID = EmptyIdentity;

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *coordSys = coordAdapter->getCoordSystem();
    LoftedPolySceneRep *sceneRep = new LoftedPolySceneRep();
    loftID = sceneRep->getId();
    sceneRep->fade = polyInfo.fade;
    
    Point3d center(0,0,0);
    bool centerValid = false;
    Point2d geoCenter(0,0);
    if (polyInfo.centered)
    {
        // We might pass in a center
        if (polyInfo.hasCenter)
        {
            geoCenter = polyInfo.center;
            Point3d dispPt = coordAdapter->localToDisplay(coordSys->geographicToLocal(geoCenter));
            center = dispPt;
            centerValid = true;
        } else {
            // Calculate the center
            GeoMbr geoMbr;
            for (ShapeSet::iterator it = shapes->begin();it != shapes->end(); ++it)
                geoMbr.expand((*it)->calcGeoMbr());
            if (geoMbr.valid())
            {
                Point3d p0 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ll()));
                Point3d p1 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ur()));
                center = (p0+p1)/2.0;
                centerValid = true;
            }
        }
    }

    
    for (ShapeSet::iterator it = shapes->begin();it != shapes->end(); ++it)
    {
        VectorArealRef theAreal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (theAreal.get())
        {
            // Work through the loops
            for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
            {
                VectorRing &ring = theAreal->loops[ri];
                
                sceneRep->shapeMbr.addGeoCoords(ring);
                
                if (coordAdapter->isFlat())
                {
                    // No grid to worry about, just tesselate
                    TesselateRing(ring, sceneRep->triMesh);
                } else {
                    // Clip the polys for the top
                    std::vector<VectorRing> clippedMesh;
                    ClipLoopToGrid(ring,Point2f(0.f,0.f),Point2f(gridSize,gridSize),clippedMesh);
                    
                    // May need to add the outline as well
                    if (polyInfo.outline)
                        sceneRep->outlines.push_back(ring);
                    
                    for (unsigned int ii=0;ii<clippedMesh.size();ii++)
                    {
                        VectorRing &ring = clippedMesh[ii];
                        // Tesselate the ring, even if it's concave (it's concave a lot)
                        TesselateRing(ring,sceneRep->triMesh);
                    }
                }
            }
        }
    }
            
    //    printf("runAddPoly: handing off %d clipped loops to addGeometry\n",(int)sceneRep->triMesh.size());
    
    addGeometryToBuilder(sceneRep, polyInfo, sceneRep->shapeMbr, center, centerValid, geoCenter, changes);
    
    pthread_mutex_lock(&loftLock);

    loftReps.insert(sceneRep);
    
    pthread_mutex_unlock(&loftLock);
    
    return loftID;
}

/// Enable/disable lofted polys
void LoftManager::enableLoftedPolys(const SimpleIDSet &polyIDs,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&loftLock);
    
    for (SimpleIDSet::iterator idIt = polyIDs.begin(); idIt != polyIDs.end(); ++idIt)
    {
        LoftedPolySceneRep dummyRep(*idIt);
        LoftedPolySceneRepSet::iterator it = loftReps.find(&dummyRep);
        if (it != loftReps.end())
        {
            LoftedPolySceneRep *sceneRep = *it;
            for (SimpleIDSet::iterator dIt = sceneRep->drawIDs.begin();
                 dIt != sceneRep->drawIDs.end(); ++dIt)
                changes.push_back(new OnOffChangeRequest(*dIt,enable));
        }
    }

    pthread_mutex_unlock(&loftLock);
}

/// Remove lofted polygons
void LoftManager::removeLoftedPolys(const SimpleIDSet &polyIDs,ChangeSet &changes)
{
    pthread_mutex_lock(&loftLock);
    
    for (SimpleIDSet::iterator idIt = polyIDs.begin(); idIt != polyIDs.end(); ++idIt)
    {
        LoftedPolySceneRep dummyRep(*idIt);
        LoftedPolySceneRepSet::iterator it = loftReps.find(&dummyRep);
        if (it != loftReps.end())
        {
            LoftedPolySceneRep *sceneRep = *it;

            TimeInterval removeTime = 0.0;
            if (sceneRep->fade > 0.0)
            {
                TimeInterval curTime = TimeGetCurrent();
                                
                for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                     idIt != sceneRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+sceneRep->fade));

                removeTime = curTime + sceneRep->fade;
            }
            
            
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                changes.push_back(new RemDrawableReq(*idIt,removeTime));
            loftReps.erase(it);
            delete sceneRep;
        }
    }
    
    pthread_mutex_unlock(&loftLock);
}
    
}
