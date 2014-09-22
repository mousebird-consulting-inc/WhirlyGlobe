/*
 *  MarkerManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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
#import "MarkerGenerator.h"
#import "ScreenSpaceGenerator.h"
#import "LayoutManager.h"
#import "SharedAttributes.h"
#import "TextureAtlas.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
MarkerSceneRep::MarkerSceneRep()
{
}
    
void MarkerSceneRep::enableContents(SelectionManager *selectManager,LayoutManager *layoutManager,SimpleIdentity generatorId,SimpleIdentity screenGenId,bool enable,ChangeSet &changes)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changes.push_back(new OnOffChangeRequest(*idIt,enable));
    if (!markerIDs.empty())
    {
        std::vector<SimpleIdentity> markerIDVec;
        for (SimpleIDSet::iterator idIt = markerIDs.begin();
             idIt != markerIDs.end(); ++idIt)
            markerIDVec.push_back(*idIt);
        changes.push_back(new MarkerGeneratorEnableRequest(generatorId,markerIDVec,enable));
    }
    if (!screenShapeIDs.empty())
    {
        std::vector<SimpleIdentity> screenIDVec;
        for (SimpleIDSet::iterator idIt = screenShapeIDs.begin();
             idIt != screenShapeIDs.end(); ++idIt)
            screenIDVec.push_back(*idIt);
        changes.push_back(new ScreenSpaceGeneratorEnableRequest(screenGenId, screenIDVec, enable));
    }
    
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
    
    if (!markerIDs.empty())
    {
        std::vector<SimpleIdentity> markerIDVec;
        for (SimpleIDSet::iterator idIt = markerIDs.begin();
             idIt != markerIDs.end(); ++idIt)
            markerIDVec.push_back(*idIt);
        changes.push_back(new MarkerGeneratorRemRequest(generatorId,markerIDVec));
    }
    markerIDs.clear();
    
    if (!screenShapeIDs.empty())
    {
        std::vector<SimpleIdentity> screenIDVec;
        for (SimpleIDSet::iterator idIt = screenShapeIDs.begin();
             idIt != screenShapeIDs.end(); ++idIt)
            screenIDVec.push_back(*idIt);
        changes.push_back(new ScreenSpaceGeneratorRemRequest(screenGenId, screenIDVec));
    }
    screenShapeIDs.clear();
    
    if (selectManager && !selectIDs.empty())
        selectManager->removeSelectables(selectIDs);
    
    if (layoutManager)
        layoutManager->removeLayoutObjects(screenShapeIDs);
}

Marker::Marker()
    : isSelectable(false), selectID(EmptyIdentity), loc(0,0), color(255,255,255,255), colorSet(false),
    lockRotation(false), height(0), width(0), rotation(0), offset(0,0), period(0),
    timeOffset(0), layoutImportance(MAXFLOAT)
{
}

void Marker::addTexID(SimpleIdentity texID)
{
    texIDs.push_back(texID);
}

MarkerInfo::MarkerInfo()
    : color(255,255,255,255), drawOffset(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid),
    screenObject(false), width(0.001), height(0.001), drawPriority(MarkerDrawPriority),
    fade(0.0), enable(true), programId(EmptyIdentity), markerId(EmptyIdentity)
{
}

void MarkerInfo::parseDict(const Dictionary &dict)
{
    color = dict.getColor(MaplyColor, RGBAColor(255,255,255,255));
    drawOffset = dict.getDouble(MaplyDrawOffset,0);
    minVis = dict.getDouble(MaplyMinVis,DrawVisibleInvalid);
    maxVis = dict.getDouble(MaplyMaxVis,DrawVisibleInvalid);
    drawPriority = dict.getInt(MaplyDrawPriority,MarkerDrawPriority);
    screenObject = dict.getBool("screen",false);
    width = dict.getDouble(MaplyLabelWidth,(screenObject ? 16.0 : 0.001));
    height = dict.getDouble(MaplyLabelHeight,(screenObject ? 16.0 : 0.001));
    fade = dict.getDouble(MaplyFade,0.0);
    enable = dict.getBool(MaplyEnable,true);
    programId = dict.getInt(MaplyShaderString,EmptyIdentity);
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
    markerRep->fade = markerInfo.fade;
    markerRep->setId(markerInfo.markerId);
    
    // For static markers, sort by texture
    DrawableMap drawables;
    std::vector<MarkerGenerator::Marker *> markersToAdd;
    
    // Screen space markers
    std::vector<ScreenSpaceGenerator::ConvexShape *> screenShapes;
    
    // Objects to be controlled by the layout layer
    std::vector<LayoutObject> layoutObjects;
    
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
        
        if (markerInfo.screenObject)
        {
            pts[0] = Point3f(-width2,-height2,0.0);
            pts[1] = Point3f(width2,-height2,0.0);
            pts[2] = Point3f(width2,height2,0.0);
            pts[3] = Point3f(-width2,height2,0.0);
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
        }
        
        // While we're at it, let's add this to the selection layer
        if (selectManager && marker->isSelectable)
        {
            // If the marker doesn't already have an ID, it needs one
            if (!marker->selectID)
                marker->selectID = Identifiable::genId();
            
            markerRep->selectIDs.insert(marker->selectID);
            if (markerInfo.screenObject)
            {
                Point2f pts2d[4];
                for (unsigned int jj=0;jj<4;jj++)
                    pts2d[jj] = Point2f(pts[jj].x(),pts[jj].y());
                selectManager->addSelectableScreenRect(marker->selectID,pts2d,markerInfo.minVis,markerInfo.maxVis,markerInfo.enable);
                if (!markerInfo.enable)
                    selectManager->enableSelectable(marker->selectID, false);
            } else {
                selectManager->addSelectableRect(marker->selectID,pts,markerInfo.minVis,markerInfo.maxVis,markerInfo.enable);
                if (!markerInfo.enable)
                    selectManager->enableSelectable(marker->selectID, false);
            }
        }
        
        // If the marker has just one texture, we can treat it as static
        if (marker->texIDs.size() <= 1)
        {
            // Look for a texture sub mapping
            SimpleIdentity texID = (marker->texIDs.empty() ? EmptyIdentity : marker->texIDs.at(0));
            SubTexture subTex = scene->getSubTexture(texID);
            
            // Build one set of texture coordinates
            std::vector<TexCoord> texCoord;
            texCoord.resize(4);
            texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
            texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
            texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
            texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;
            subTex.processTexCoords(texCoord);
            
            if (markerInfo.screenObject)
            {
                ScreenSpaceGenerator::SimpleGeometry smGeom;
                smGeom.texID = subTex.texId;
                smGeom.programID = markerInfo.programId;
                smGeom.color = markerInfo.color;
                if (marker->colorSet)
                    smGeom.color = marker->color;
                
                for (unsigned int ii=0;ii<4;ii++)
                {
                    smGeom.coords.push_back(Point2d(pts[ii].x(),pts[ii].y()));
                    smGeom.texCoords.push_back(texCoord[ii]);
                }
                ScreenSpaceGenerator::ConvexShape *shape = new ScreenSpaceGenerator::ConvexShape();
                if (marker->isSelectable && marker->selectID != EmptyIdentity)
                    shape->setId(marker->selectID);
                    shape->worldLoc = coordAdapter->localToDisplay(localPt);
                    if (marker->lockRotation)
                    {
                        shape->useRotation = true;
                        shape->rotation = marker->rotation;
                    }
                if (markerInfo.fade > 0.0)
                {
                    shape->fadeDown = curTime;
                    shape->fadeUp = curTime+markerInfo.fade;
                }
                shape->minVis = markerInfo.minVis;
                shape->maxVis = markerInfo.maxVis;
                shape->drawPriority = markerInfo.drawPriority;
                shape->enable = markerInfo.enable;
                shape->geom.push_back(smGeom);
                screenShapes.push_back(shape);
                markerRep->screenShapeIDs.insert(shape->getId());
                
                // Set up for the layout layer
                if (layoutManager && marker->layoutImportance != MAXFLOAT)
                {
                    WhirlyKit::LayoutObject layoutObj(shape->getId());
                    layoutObj.dispLoc = shape->worldLoc;
                    // Note: This means they won't take up space
                    layoutObj.size = Point2f(2*width2,2*height2);
                    layoutObj.iconSize = Point2f(0.0,0.0);
                    layoutObj.importance = marker->layoutImportance;
                    layoutObj.minVis = markerInfo.minVis;
                    layoutObj.maxVis = markerInfo.maxVis;
                    // No moving it around
                    layoutObj.acceptablePlacement = 1;
                    layoutObj.enable = markerInfo.enable;
                    layoutObjects.push_back(layoutObj);
                    
                    // Start out off, let the layout layer handle the rest
                    shape->enable = markerInfo.enable;
                    shape->offset = Point2d(MAXFLOAT,MAXFLOAT);
                } else
                    shape->offset = Point2d(marker->offset.x(),marker->offset.y());
                
            } else {
                // We're sorting the static drawables by texture, so look for that
                DrawableMap::iterator it = drawables.find(subTex.texId);
                BasicDrawable *draw = NULL;
                if (it != drawables.end())
                    draw = it->second;
                    else {
                        draw = new BasicDrawable("Marker Layer");
                        draw->setType(GL_TRIANGLES);
                        draw->setDrawOffset(markerInfo.drawOffset);
                        draw->setColor(markerInfo.color);
                        draw->setDrawPriority(markerInfo.drawPriority);
                        draw->setVisibleRange(markerInfo.minVis, markerInfo.maxVis);
                        draw->setTexId(0,subTex.texId);
                        draw->setOnOff(markerInfo.enable);
                        drawables[subTex.texId] = draw;
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
            }
        } else {
            // The marker changes textures, so we need to pass it to the generator
            MarkerGenerator::Marker *newMarker = new MarkerGenerator::Marker();
            newMarker->enable = true;
            newMarker->color = RGBAColor(255,255,255,255);
            newMarker->loc = marker->loc;
            newMarker->enable = markerInfo.enable;
            for (unsigned int ii=0;ii<4;ii++)
                newMarker->pts[ii] = pts[ii];
                newMarker->norm = Vector3dToVector3f(norm);
                newMarker->period = marker->period;
                newMarker->start = marker->timeOffset;
                newMarker->drawOffset = markerInfo.drawOffset;
                newMarker->minVis = markerInfo.minVis;
                newMarker->maxVis = markerInfo.maxVis;
                newMarker->drawPriority = markerInfo.drawPriority;
                if (markerInfo.fade > 0.0)
                {
                    newMarker->fadeDown = curTime;
                    newMarker->fadeUp = curTime+markerInfo.fade;
                } else {
                    newMarker->fadeDown = newMarker->fadeUp= 0.0;
                }
            
            // Each set of texture coordinates may be different
            std::vector<TexCoord> texCoord;
            texCoord.resize(4);
            texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
            texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
            texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
            texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;
            for (unsigned int ii=0;ii<marker->texIDs.size();ii++)
            {
                SubTexture subTex = scene->getSubTexture(marker->texIDs.at(ii));
                std::vector<TexCoord> theseTexCoord = texCoord;
                subTex.processTexCoords(theseTexCoord);
                newMarker->texCoords.push_back(theseTexCoord);
                newMarker->texIDs.push_back(subTex.texId);
            }
            
            // Send it off to the generator
            markerRep->markerIDs.insert(newMarker->getId());
            markersToAdd.push_back(newMarker);
        }
    }
    
    // Add all the new markers at once
    if (!markersToAdd.empty())
        changes.push_back(new MarkerGeneratorAddRequest(generatorId,markersToAdd));
        
    // Flush out any drawables for the static geometry
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
    {
        if (markerInfo.fade > 0.0)
        {
            TimeInterval curTime = TimeGetCurrent();
            it->second->setFade(curTime,curTime+markerInfo.fade);
        }
        changes.push_back(new AddDrawableReq(it->second));
    }
    drawables.clear();
    
    // Add all the screen space markers at once
    if (!screenShapes.empty())
        changes.push_back(new ScreenSpaceGeneratorAddRequest(screenGenId,screenShapes));
    screenShapes.clear();
            
    // And any layout constraints to the layout engine
    if (layoutManager && !layoutObjects.empty())
        layoutManager->addLayoutObjects(layoutObjects);
    
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
    
    // Set up the generator we'll pass markers to
    MarkerGenerator *gen = new MarkerGenerator();
    generatorId = gen->getId();
    scene->addChangeRequest(new AddGeneratorReq(gen));
}

}
