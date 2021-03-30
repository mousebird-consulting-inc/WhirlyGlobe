/*  MarkerManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
 *  Copyright 2011-2021 mousebird consulting.
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
 */

#import "MarkerManager.h"
#import "LayoutManager.h"
#import "ScreenSpaceBuilder.h"
#import "SharedAttributes.h"
#import "CoordSystem.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
MarkerInfo::MarkerInfo(bool screenObject)
    : color(255,255,255), screenObject(screenObject), width(0.0), height(0.0),
    layoutImportance(MAXFLOAT),clusterGroup(-1)
{
    if (screenObject) {
        width = 16.0;
        height = 16.0;
    } else {
        width = 0.001;
        height = 0.001;
    }
}
    
MarkerInfo::MarkerInfo(const Dictionary &dict,bool screenObject)
: BaseInfo(dict), screenObject(screenObject)
{
    color = dict.getColor(MaplyColor, RGBAColor(255,255,255,255));
    width = dict.getDouble(MaplyLabelWidth,(screenObject ? 16.0 : 0.001));
    height = dict.getDouble(MaplyLabelHeight,(screenObject ? 16.0 : 0.001));
    layoutImportance = dict.getDouble(MaplyLayoutImportance,MAXFLOAT);
    clusterGroup = dict.getInt(MaplyClusterGroupID,-1);
}
    
MarkerSceneRep::MarkerSceneRep()
    : useLayout(false)
{
}
    
void MarkerSceneRep::enableContents(SelectionManagerRef &selectManager,LayoutManagerRef &layoutManager,bool enable,ChangeSet &changes)
{
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changes.push_back(new OnOffChangeRequest(*idIt,enable));
    
    if (selectManager && !selectIDs.empty())
        selectManager->enableSelectables(selectIDs, enable);
    
    if (layoutManager)
        layoutManager->enableLayoutObjects(screenShapeIDs, enable);
}
    
void MarkerSceneRep::clearContents(SelectionManagerRef &selectManager,LayoutManagerRef &layoutManager,ChangeSet &changes,TimeInterval when)
{
    // Just delete everything
    for (SimpleIDSet::iterator idIt = drawIDs.begin();
         idIt != drawIDs.end(); ++idIt)
        changes.push_back(new RemDrawableReq(*idIt,when));
    drawIDs.clear();
    
    if (selectManager && !selectIDs.empty())
        selectManager->removeSelectables(selectIDs);
    
    if (layoutManager && useLayout)
        layoutManager->removeLayoutObjects(screenShapeIDs);

    screenShapeIDs.clear();
}

Marker::Marker()
    : isSelectable(false), selectID(EmptyIdentity),
    loc(0,0), hasMotion(false), endLoc(0,0),
    startTime(0), endTime(0),
    color(255,255,255,255), colorSet(false),
    lockRotation(false),
    height(0), width(0),
    layoutHeight(-1.0), layoutWidth(-1.0),
    rotation(0), offset(0,0), period(0),
    timeOffset(0), layoutImportance(MAXFLOAT), orderBy(-1),
    maskID(EmptyIdentity), maskRenderTargetID(EmptyIdentity)
{
}

Marker::~Marker()
{
    
}

void Marker::addTexID(SimpleIdentity texID)
{
    texIDs.push_back(texID);
}

MarkerManager::MarkerManager()
: maskProgID(EmptyIdentity)
{
}

MarkerManager::~MarkerManager()
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (MarkerSceneRepSet::iterator it = markerReps.begin();
         it != markerReps.end(); ++it)
        delete *it;
    markerReps.clear();
}

typedef std::map<SimpleIDSet,BasicDrawableBuilderRef> DrawableMap;

SimpleIdentity MarkerManager::addMarkers(const std::vector<Marker *> &markers,const MarkerInfo &markerInfo,ChangeSet &changes)
{
    auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);
    const TimeInterval curTime = scene->getCurrentTime();

    if (maskProgID == EmptyIdentity) {
        Program *prog = scene->findProgramByName(MaplyScreenSpaceMaskShader);
        if (prog)
            maskProgID = prog->getId();
    }
    
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    MarkerSceneRep *markerRep = new MarkerSceneRep();
    markerRep->fadeOut = markerInfo.fadeOut;
    
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
            float layoutImport = markerInfo.layoutImportance;
            if (layoutManager && marker->layoutImportance < MAXFLOAT)
                layoutImport = marker->layoutImportance;
            if (layoutImport < MAXFLOAT)
            {
                markerRep->useLayout = true;
                layoutObj = new LayoutObject();
                shape = layoutObj;
            } else
                shape = new ScreenSpaceObject();
            
            if (!marker->uniqueID.empty() && layoutObj)
                layoutObj->uniqueID = marker->uniqueID;

            if (marker->orderBy >= 0)
                shape->setOrderBy(marker->orderBy);

            shape->setPeriod(marker->period);
            
            ScreenSpaceConvexGeometry smGeom;
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
            if (marker->hasMotion)
                shape->setMovingLoc(coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(marker->endLoc)), marker->startTime, marker->endTime);
            if (marker->lockRotation)
                shape->setRotation(marker->rotation);
            if (markerInfo.fadeIn > 0.0)
                shape->setFade(curTime+markerInfo.fadeIn, curTime);
            else if (markerInfo.fadeOut > 0.0 && markerInfo.fadeOutTime > 0.0)
                shape->setFade(markerInfo.fadeOutTime, markerInfo.fadeOutTime+markerInfo.fadeOut);
            shape->setVisibility(markerInfo.minVis, markerInfo.maxVis);
            shape->setZoomInfo(markerInfo.zoomSlot, markerInfo.minZoomVis, markerInfo.maxZoomVis);
            shape->setOpacityExp(markerInfo.opacityExp);
            shape->setColorExp(markerInfo.colorExp);
            shape->setScaleExp(markerInfo.scaleExp);
            shape->setDrawOrder(markerInfo.drawOrder);
            shape->setDrawPriority(markerInfo.drawPriority);
            shape->setEnable(markerInfo.enable);
            if (markerInfo.startEnable != markerInfo.endEnable)
                shape->setEnableTime(markerInfo.startEnable, markerInfo.endEnable);
            shape->addGeometry(smGeom);
            markerRep->screenShapeIDs.insert(shape->getId());
            
            // Handle the mask rendering if it's there
            if (marker->maskID != EmptyIdentity && marker->maskRenderTargetID != EmptyIdentity) {
                // Make a copy of the geometry, but target it to the mask render target
                const std::vector<ScreenSpaceConvexGeometry> *geom = shape->getGeometry();
                for (auto entry: *geom) {
                    entry.vertexAttrs.insert(SingleVertexAttribute(a_maskNameID, renderer->getSlotForNameID(a_maskNameID), (int)marker->maskID));
                    entry.renderTargetID = marker->maskRenderTargetID;
                    entry.progID = maskProgID;
                    shape->addGeometry(entry);
                }
            }
            
            // Set up for the layout layer
            if (layoutImport < MAXFLOAT)
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
                layoutObj->clusterGroup = markerInfo.clusterGroup;
                layoutObj->importance = layoutImport;
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
            SimpleIDSet texIDs;
            for (const auto &subTex : subTexs)
                texIDs.insert(subTex.texId);
            if (texIDs.empty())
                texIDs.insert(EmptyIdentity);
            DrawableMap::iterator it = drawables.find(texIDs);
            BasicDrawableBuilderRef draw;
            if (it != drawables.end())
                draw = it->second;
            else {
                draw = renderer->makeBasicDrawableBuilder("Marker Layer");
                draw->setType(Triangles);
                markerInfo.setupBasicDrawable(draw);
                draw->setColor(markerInfo.color);
                draw->setTexId(0,*(texIDs.begin()));
                drawables[texIDs] = draw;
                markerRep->drawIDs.insert(draw->getDrawableID());

                // If we've got more than one texture ID and a period, we need a tweaker
                if (texIDs.size() > 1 && marker->period != 0.0)
                {
                    TimeInterval now = scene->getCurrentTime();
                    std::vector<SimpleIdentity> texIDVec;
                    std::copy(texIDs.begin(), texIDs.end(), std::back_inserter(texIDVec));
                    auto tweak = std::make_shared<BasicDrawableTexTweaker>(texIDVec,now,marker->period);
                    draw->addTweaker(tweak);
                }
            }
            
            // Toss the geometry into the drawable
            int vOff = draw->getNumPoints();
            for (unsigned int ii=0;ii<4;ii++)
            {
                Point3f &pt = pts[ii];
                draw->addPoint(pt);
                draw->addNormal(Vector3dToVector3f(norm));
                for (unsigned int jj=0;jj<texIDs.size();jj++)
                    draw->addTexCoord(jj,texCoord[ii]);
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
    for (auto it = drawables.begin(); it != drawables.end(); ++it)
    {
        if (markerInfo.fadeIn > 0.0)
        {
            TimeInterval curTime = scene->getCurrentTime();
            it->second->setFade(curTime,curTime+markerInfo.fadeIn);
        }
        changes.push_back(new AddDrawableReq(it->second->getDrawable()));
    }
    drawables.clear();
    
    // Add any simple 2D markers to the scene
    if (!screenShapes.empty())
    {
        ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->getScale());
        ssBuild.addScreenObjects(screenShapes);
        for (unsigned int ii=0;ii<screenShapes.size();ii++)
            delete screenShapes[ii];
        ssBuild.flushChanges(changes, markerRep->drawIDs);
    }
    
    // And any layout constraints to the layout engine
    if (layoutManager && !layoutObjects.empty())
        layoutManager->addLayoutObjects(layoutObjects);
    for (unsigned int ii=0;ii<layoutObjects.size();ii++)
        delete layoutObjects[ii];
    
    SimpleIdentity markerID = markerRep->getId();
    
    {
        std::lock_guard<std::mutex> guardLock(lock);
        markerReps.insert(markerRep);
    }
    
    return markerID;
}

void MarkerManager::enableMarkers(SimpleIDSet &markerIDs,bool enable,ChangeSet &changes)
{
    SelectionManagerRef selectManager = std::dynamic_pointer_cast<SelectionManager>(scene->getManager(kWKSelectionManager));
    LayoutManagerRef layoutManager = std::dynamic_pointer_cast<LayoutManager>(scene->getManager(kWKLayoutManager));

    std::lock_guard<std::mutex> guardLock(lock);

    for (SimpleIDSet::iterator mit = markerIDs.begin();mit != markerIDs.end(); ++mit)
    {
        MarkerSceneRep dummyRep;
        dummyRep.setId(*mit);
        MarkerSceneRepSet::iterator it = markerReps.find(&dummyRep);
        if (it != markerReps.end())
        {
            MarkerSceneRep *markerRep = *it;
            markerRep->enableContents(selectManager, layoutManager, enable, changes);
        }
    }
}

void MarkerManager::removeMarkers(SimpleIDSet &markerIDs,ChangeSet &changes)
{
    SelectionManagerRef selectManager = std::dynamic_pointer_cast<SelectionManager>(scene->getManager(kWKSelectionManager));
    LayoutManagerRef layoutManager = std::dynamic_pointer_cast<LayoutManager>(scene->getManager(kWKLayoutManager));

    std::lock_guard<std::mutex> guardLock(lock);

    TimeInterval curTime = scene->getCurrentTime();
    for (SimpleIDSet::iterator mit = markerIDs.begin();mit != markerIDs.end(); ++mit)
    {
        SimpleIdentity markerID = *mit;
        MarkerSceneRep dummyRep;
        dummyRep.setId(markerID);
        MarkerSceneRepSet::iterator it = markerReps.find(&dummyRep);
        if (it != markerReps.end())
        {
            MarkerSceneRep *markerRep = *it;
            
            TimeInterval removeTime = 0.0;
            if (markerRep->fadeOut > 0.0)
            {
                for (SimpleIDSet::iterator idIt = markerRep->drawIDs.begin();
                     idIt != markerRep->drawIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+markerRep->fadeOut));
                
                removeTime = curTime + markerRep->fadeOut;
            }
            
            
            markerRep->clearContents(selectManager, layoutManager, changes, removeTime);
            
            markerReps.erase(it);
            delete markerRep;
        }
    }
}

void MarkerManager::setScene(Scene *inScene)
{
    SceneManager::setScene(inScene);
}

}
