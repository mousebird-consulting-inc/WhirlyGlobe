/*
 *  MarkerManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
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

#import "MarkerManager.h"
#import "ScreenSpaceGenerator.h"
#import "LayoutManager.h"
#import "ScreenSpaceBuilder.h"
#import "SharedAttributes.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
MarkerSceneRep::MarkerSceneRep()
    : useLayout(false)
{
}
    
void MarkerSceneRep::enableContents(SelectionManager *selectManager,LayoutManager *layoutManager,SimpleIdentity generatorId,SimpleIdentity screenGenId,bool enable,ChangeSet &changes)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changes.push_back(new OnOffChangeRequest(*idIt,enable));
    
    if (selectManager && !selectIDs.empty())
        selectManager->enableSelectables(selectIDs, enable);
    
    if (layoutManager)
        layoutManager->enableLayoutObjects(screenShapeIDs, enable);
}
    
void MarkerSceneRep::clearContents(SelectionManager *selectManager,LayoutManager *layoutManager,SimpleIdentity generatorId,SimpleIdentity screenGenId,ChangeSet &changes)
{
    // Just delete everything
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changes.push_back(new RemDrawableReq(*idIt));
    drawIDs.clear();
    
    if (selectManager && !selectIDs.empty())
        selectManager->removeSelectables(selectIDs);
    
    if (layoutManager && useLayout)
        layoutManager->removeLayoutObjects(screenShapeIDs);

    screenShapeIDs.clear();
}

Marker::Marker()
    : isSelectable(false), selectID(EmptyIdentity), loc(0,0), hasMotion(false), endLoc(0,0), startTime(0), endTime(0),
    color(255,255,255,255), colorSet(false),
    lockRotation(false), height(0), width(0), layoutHeight(0), layoutWidth(0), rotation(0), offset(0,0), period(0),
    timeOffset(0), layoutImportance(MAXFLOAT)
{
}

void Marker::addTexID(SimpleIdentity texID)
{
    texIDs.push_back(texID);
}

MarkerInfo::MarkerInfo(const Dictionary &dict)
    : BaseInfo(dict), color(255,255,255,255),
    screenObject(false), width(0.001), height(0.001),
    markerId(EmptyIdentity)
{
    color = dict.getColor(MaplyColor, RGBAColor(255,255,255,255));
    screenObject = dict.getBool("screen",false);
    width = dict.getDouble(MaplyLabelWidth,(screenObject ? 16.0 : 0.001));
    height = dict.getDouble(MaplyLabelHeight,(screenObject ? 16.0 : 0.001));
}

MarkerManager::MarkerManager()
{
    pthread_mutex_init(&markerLock,NULL);
}

MarkerManager::~MarkerManager()
{
    for (MarkerSceneRepSet::iterator it = markerReps.begin();
         it != markerReps.end(); ++it)
        delete *it;
    markerReps.clear();
    
    pthread_mutex_destroy(&markerLock);
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableMap;

SimpleIdentity MarkerManager::addMarkers(const std::vector<Marker *> &markers,const MarkerInfo &markerInfo,ChangeSet &changes)
{

    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    TimeInterval curTime = TimeGetCurrent();
    
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    MarkerSceneRep *markerRep = new MarkerSceneRep();
    markerRep->fadeOut = markerInfo.fadeOut;
    markerRep->setId(markerInfo.markerId);
    
    // For static markers, sort by texture
    DrawableMap drawables;

    // Screen space markers
    std::vector<ScreenSpaceObject *> screenShapes;
    
    // Objects to be controlled by the layout layer
    std::vector<LayoutObject *> layoutObjects;
    
    for (unsigned int ii=0;ii<markers.size();ii++)
    {
        Marker *marker = markers[ii];
        // Build the rectangle for this one
        Point3f pts[4];
        Vector3d norm;
        float width2 = (marker->width == 0.0 ? markerInfo.width : marker->width)/2.0;
        float height2 = (marker->height == 0.0 ? markerInfo.height : marker->height)/2.0;
        
        Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(marker->loc);
        norm = coordAdapter->normalForLocal(localPt);
        
        // Note: Not supporting more than one texture at the moment
        
        // Look for a texture sub mapping
        std::vector<SubTexture> subTexs;
        for (unsigned int ii=0; ii<marker->texIDs.size();ii++)
        {
            SimpleIdentity texID = marker->texIDs.at(ii);
            SubTexture subTex = scene->getSubTexture(texID);
            subTexs.push_back(subTex);
        }
        
        // Build one set of texture coordinates
        std::vector<TexCoord> texCoord;
        texCoord.resize(4);
        texCoord[3].u() = 0.0;  texCoord[3].v() = 0.0;
        texCoord[2].u() = 1.0;  texCoord[2].v() = 0.0;
        texCoord[1].u() = 1.0;  texCoord[1].v() = 1.0;
        texCoord[0].u() = 0.0;  texCoord[0].v() = 1.0;
        // Note: This assume they all have the same (or no) sub texture mapping
        if (!subTexs.empty())
            subTexs[0].processTexCoords(texCoord);
        
        if (markerInfo.screenObject)
        {
            pts[0] = Point3f(-width2+marker->offset.x(),-height2+marker->offset.y(),0.0);
            pts[1] = Point3f(width2+marker->offset.x(),-height2+marker->offset.y(),0.0);
            pts[2] = Point3f(width2+marker->offset.x(),height2+marker->offset.y(),0.0);
            pts[3] = Point3f(-width2+marker->offset.x(),height2+marker->offset.y(),0.0);

            ScreenSpaceObject *shape = NULL;
            LayoutObject *layoutObj = NULL;
            if (layoutManager && marker->layoutImportance != MAXFLOAT)
            {
                markerRep->useLayout = true;
                layoutObj = new LayoutObject();
                shape = layoutObj;
            } else
                shape = new ScreenSpaceObject();
            shape->setPeriod(marker->period);
            
            ScreenSpaceObject::ConvexGeometry smGeom;
            for (unsigned int ii=0;ii<subTexs.size();ii++)
                smGeom.texIDs.push_back(subTexs[ii].texId);
            smGeom.progID = markerInfo.programID;
            smGeom.color = markerInfo.color;
            smGeom.vertexAttrs = marker->vertexAttrs;
            if (marker->colorSet)
                smGeom.color = marker->color;
            for (unsigned int ii=0;ii<4;ii++)
            {
                smGeom.coords.push_back(Point2d(pts[ii].x(),pts[ii].y()));
                smGeom.texCoords.push_back(texCoord[ii]);
            }
            if (marker->isSelectable && marker->selectID != EmptyIdentity)
                shape->setId(marker->selectID);
            shape->setWorldLoc(coordAdapter->localToDisplay(localPt));
            if (marker->lockRotation)
                shape->setRotation(marker->rotation);
            if (markerInfo.fadeIn > 0.0)
                shape->setFade(curTime+markerInfo.fadeIn, curTime);
            else if (markerInfo.fadeOut > 0.0 && markerInfo.fadeOutTime > 0.0)
                shape->setFade(markerInfo.fadeOutTime, markerInfo.fadeOutTime+markerInfo.fadeOut);
            shape->setVisibility(markerInfo.minVis, markerInfo.maxVis);
            shape->setDrawPriority(markerInfo.drawPriority);
            shape->setEnable(markerInfo.enable);
            if (markerInfo.startEnable != markerInfo.endEnable)
                shape->setEnableTime(markerInfo.startEnable, markerInfo.endEnable);
            shape->addGeometry(smGeom);
            markerRep->screenShapeIDs.insert(shape->getId());
            
            // Set up for the layout layer
            if (layoutManager && marker->layoutImportance != MAXFLOAT)
            {
                if (marker->layoutWidth >= 0.0)
                {
                    layoutObj->layoutPts.push_back(Point2d(-marker->layoutWidth/2.0+marker->offset.x(),-marker->layoutHeight/2.0+marker->offset.y()));
                    layoutObj->layoutPts.push_back(Point2d(marker->layoutWidth/2.0+marker->offset.x(),-marker->layoutHeight/2.0+marker->offset.y()));
                    layoutObj->layoutPts.push_back(Point2d(marker->layoutWidth/2.0+marker->offset.x(),marker->layoutHeight/2.0+marker->offset.y()));
                    layoutObj->layoutPts.push_back(Point2d(-marker->layoutWidth/2.0+marker->offset.x(),marker->layoutHeight/2.0+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(-width2+marker->offset.x(),-height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(width2+marker->offset.x(),-height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(width2+marker->offset.x(),height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(-width2+marker->offset.x(),height2+marker->offset.y()));
                } else {
                    layoutObj->selectPts.push_back(Point2d(-width2+marker->offset.x(),-height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(width2+marker->offset.x(),-height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(width2+marker->offset.x(),height2+marker->offset.y()));
                    layoutObj->selectPts.push_back(Point2d(-width2+marker->offset.x(),height2+marker->offset.y()));
                    layoutObj->layoutPts = layoutObj->selectPts;
                }
                layoutObj->importance = marker->layoutImportance;
                // No moving it around
                layoutObj->acceptablePlacement = 1;
                
                // Start out off, let the layout layer handle the rest
                shape->setEnable(markerInfo.enable);
                if (markerInfo.startEnable != markerInfo.endEnable)
                    shape->setEnableTime(markerInfo.startEnable, markerInfo.endEnable);
                shape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
            }
            
            if (layoutObj)
                layoutObjects.push_back(layoutObj);
            else if (shape)
            {
                if (selectManager)
                {
                    // If the marker doesn't already have an ID, it needs one
                    if (!marker->selectID)
                        marker->selectID = Identifiable::genId();
                    
                    markerRep->selectIDs.insert(marker->selectID);
                    Point2f pts2d[4];
                    for (unsigned int jj=0;jj<4;jj++)
                        pts2d[jj] = Point2f(pts[jj].x(),pts[jj].y());
                    if (marker->hasMotion)
                        selectManager->addSelectableMovingScreenRect(marker->selectID, shape->getWorldLoc(), shape->getEndWorldLoc(), shape->getStartTime(), shape->getEndTime(), pts2d,markerInfo.minVis,markerInfo.maxVis,markerInfo.enable);
                    else
                        selectManager->addSelectableScreenRect(marker->selectID,shape->getWorldLoc(),pts2d,markerInfo.minVis,markerInfo.maxVis,markerInfo.enable);
                    if (!markerInfo.enable)
                        selectManager->enableSelectable(marker->selectID, false);
                }

                screenShapes.push_back(shape);
            }
        } else {
            Point3d center = coordAdapter->localToDisplay(localPt);
            Vector3d up(0,0,1);
            Point3d horiz,vert;
            if (coordAdapter->isFlat())
            {
                horiz = Point3d(1,0,0);
                vert = Point3d(0,1,0);
            } else {
                horiz = up.cross(norm).normalized();
                vert = norm.cross(horiz).normalized();;
            }
            
            Point3d ll = center - width2*horiz - height2*vert;
            pts[0] = Vector3dToVector3f(ll);
            pts[1] = Vector3dToVector3f(ll + 2 * width2 * horiz);
            pts[2] = Vector3dToVector3f(ll + 2 * width2 * horiz + 2 * height2 * vert);
            pts[3] = Vector3dToVector3f(ll + 2 * height2 * vert);

            // We're sorting the static drawables by texture, so look for that
            SimpleIdentity subTexID = (subTexs.empty() ? EmptyIdentity : subTexs[0].texId);
            DrawableMap::iterator it = drawables.find(subTexID);
            BasicDrawable *draw = NULL;
            if (it != drawables.end())
                draw = it->second;
                else {
                    draw = new BasicDrawable("Marker Layer");
                    draw->setType(GL_TRIANGLES);
                    markerInfo.setupBasicDrawable(draw);
                    draw->setColor(markerInfo.color);
                    draw->setTexId(0,subTexID);
                    if (markerInfo.programID != EmptyIdentity)
                       draw->setProgram(markerInfo.programID);
                    drawables[subTexID] = draw;
                    markerRep->drawIDs.insert(draw->getId());
                }
            
            // Toss the geometry into the drawable
            int vOff = draw->getNumPoints();
            for (unsigned int ii=0;ii<4;ii++)
            {
                Point3f &pt = pts[ii];
                draw->addPoint(pt);
                draw->addNormal(Vector3dToVector3f(norm));
                draw->addTexCoord(0,texCoord[ii]);
                Mbr localMbr = draw->getLocalMbr();
                Point3f localLoc = coordAdapter->getCoordSystem()->geographicToLocal(marker->loc);
                localMbr.addPoint(Point2f(localLoc.x(),localLoc.y()));
                draw->setLocalMbr(localMbr);
            }
            
            draw->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
            draw->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));

            if (selectManager)
            {
                selectManager->addSelectableRect(marker->selectID,pts,markerInfo.minVis,markerInfo.maxVis,markerInfo.enable);
                markerRep->selectIDs.insert(marker->selectID);
            }
        }
    }

    // Flush out any drawables for the static geometry
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
    {
        if (markerInfo.fadeIn > 0.0)
        {
            TimeInterval curTime = TimeGetCurrent();
            it->second->setFade(curTime,curTime+markerInfo.fadeIn);
        }
        changes.push_back(new AddDrawableReq(it->second));
    }
    drawables.clear();
    
    // Add any simple 2D markers to the scene
    if (!screenShapes.empty())
    {
        ScreenSpaceBuilder ssBuild(coordAdapter,renderer->getScale());
        for (unsigned int ii=0;ii<screenShapes.size();ii++)
        {
            ssBuild.addScreenObject(*(screenShapes[ii]));
            delete screenShapes[ii];
        }
        ssBuild.flushChanges(changes, markerRep->drawIDs);
    }
    
    // And any layout constraints to the layout engine
    if (layoutManager && !layoutObjects.empty())
        layoutManager->addLayoutObjects(layoutObjects);
    for (unsigned int ii=0;ii<layoutObjects.size();ii++)
        delete layoutObjects[ii];
    
    pthread_mutex_lock(&markerLock);
    markerReps.insert(markerRep);
    pthread_mutex_unlock(&markerLock);
    
    return markerInfo.markerId;
}

void MarkerManager::enableMarkers(SimpleIDSet &markerIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);

    pthread_mutex_lock(&markerLock);
    
    for (SimpleIDSet::iterator mit = markerIDs.begin();mit != markerIDs.end(); ++mit)
    {
        MarkerSceneRep dummyRep;
        dummyRep.setId(*mit);
        MarkerSceneRepSet::iterator it = markerReps.find(&dummyRep);
        if (it != markerReps.end())
        {
            MarkerSceneRep *markerRep = *it;
            markerRep->enableContents(selectManager, layoutManager, generatorId, screenGenId, enable, changes);
        }
    }
    
    pthread_mutex_unlock(&markerLock);
}

void MarkerManager::removeMarkers(SimpleIDSet &markerIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);

    pthread_mutex_lock(&markerLock);
    
    for (SimpleIDSet::iterator mit = markerIDs.begin();mit != markerIDs.end(); ++mit)
    {
        SimpleIdentity markerID = *mit;
        MarkerSceneRep dummyRep;
        dummyRep.setId(markerID);
        MarkerSceneRepSet::iterator it = markerReps.find(&dummyRep);
        if (it != markerReps.end())
        {
            MarkerSceneRep *markerRep = *it;
            
            // Note: Porting
//            if (markerRep->fade > 0.0)
//            {
//                TimeInterval curTime = CFAbsoluteTimeGetCurrent();
//                for (SimpleIDSet::iterator idIt = markerRep->drawIDs.begin();
//                     idIt != markerRep->drawIDs.end(); ++idIt)
//                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+markerRep->fade));
//                
//                if (!markerRep->markerIDs.empty())
//                {
//                    std::vector<SimpleIdentity> markerIDs;
//                    for (SimpleIDSet::iterator idIt = markerRep->markerIDs.begin();
//                         idIt != markerRep->markerIDs.end(); ++idIt)
//                        markerIDs.push_back(*idIt);
//                    changes.push_back(new MarkerGeneratorFadeRequest(generatorId,markerIDs,curTime,curTime+markerRep->fade));
//                }
//                
//                if (!markerRep->screenShapeIDs.empty())
//                {
//                    std::vector<SimpleIdentity> screenIDs;
//                    for (SimpleIDSet::iterator idIt = markerRep->screenShapeIDs.begin();
//                         idIt != markerRep->screenShapeIDs.end(); ++idIt)
//                        screenIDs.push_back(*idIt);
//                    changes.push_back(new ScreenSpaceGeneratorFadeRequest(screenGenId,screenIDs,curTime, curTime+markerRep->fade));
//                }
//                
//                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, markerRep->fade * NSEC_PER_SEC),
//                               scene->getDispatchQueue(),
//                               ^{
//                                   SimpleIDSet theseMarkerIDs;
//                                   theseMarkerIDs.insert(markerID);
//                                   ChangeSet delChanges;
//                                   removeMarkers(theseMarkerIDs,delChanges);
//                                   scene->addChangeRequests(delChanges);
//                               }
//                               );
//
//                markerRep->fade = 0.0;
//            } else {
                markerRep->clearContents(selectManager, layoutManager, generatorId, screenGenId, changes);
                
                markerReps.erase(it);
                delete markerRep;
//            }
        }
    }
    
    pthread_mutex_unlock(&markerLock);
}

void MarkerManager::setScene(Scene *inScene)
{
    SceneManager::setScene(inScene);

    screenGenId = scene->getScreenSpaceGeneratorID();
}

}
