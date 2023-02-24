/*  MarkerManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/16/13.
 *  Copyright 2011-2023 mousebird consulting.
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
#import "WhirlyKitLog.h"
#import "MapboxVectorStyleSetC.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
    
MarkerInfo::MarkerInfo(bool screenObject) :
    color(RGBAColor::white()),
    screenObject(screenObject),
    width(screenObject ? 16.0 : 0.001),
    height(screenObject ? 16.0 : 0.001),
    layoutImportance(MAXFLOAT),
    clusterGroup(-1)
{
}

MarkerInfo::MarkerInfo(const Dictionary &dict,bool screenObject) :
    BaseInfo(dict),
    screenObject(screenObject)
{
    color = dict.getColor(MaplyColor, RGBAColor::white());
    width = (float)dict.getDouble(MaplyLabelWidth,(screenObject ? 16.0 : 0.001));
    height = (float)dict.getDouble(MaplyLabelHeight,(screenObject ? 16.0 : 0.001));
    layoutImportance = (float)dict.getDouble(MaplyLayoutImportance,MAXFLOAT);
    clusterGroup = dict.getInt(MaplyClusterGroupID,-1);
    layoutDebug = dict.getInt(MaplyTextLayoutDebug,false);
    layoutRepeat = dict.getInt(MaplyTextLayoutRepeat,-1);
    layoutSpacing = (float)dict.getDouble(MaplyTextLayoutSpacing,24.0);
    layoutOffset = (float)dict.getDouble(MaplyTextLayoutOffset,0.0);

    if (const auto entry = dict.getEntry(MaplyOpacity))
    {
        if (entry->getType() == DictionaryType::DictTypeDictionary)
        {
            if (const auto expr = MapboxVectorStyleSetImpl::transDouble(entry, MaplyOpacity, 1.0))
            {
                opacityExp = expr->expression();
            }
        }
    }

    if (const auto entry = dict.getEntry(MaplyColor))
    {
        if (entry->getType() == DictionaryType::DictTypeDictionary)
        {
            if (const auto expr = MapboxVectorStyleSetImpl::transColor(entry, MaplyColor, nullptr))
            {
                colorExp = expr->expression();
            }
        }
    }

    if (const auto entry = dict.getEntry(MaplyMarkerScale))
    {
        if (entry->getType() == DictionaryType::DictTypeDictionary)
        {
            if (const auto expr = MapboxVectorStyleSetImpl::transDouble(entry, MaplyOpacity, 1.0))
            {
                scaleExp = expr->expression();
            }
        }

        // Since we don't have a simple scale member, allow them to specify
        // a scale by producing an expression that always produces that value.
        if (!scaleExp)
        {
            const double scale = entry->getDouble();
            if (scale != 0.0 && scale != 1.0)
            {
                scaleExp = std::make_shared<FloatExpressionInfo>();
                scaleExp->base = 1.0f;
                scaleExp->type = ExpressionInfoType::ExpressionLinear;
                scaleExp->stopInputs = { 1.0f, 1.0f };
                scaleExp->stopOutputs = { (float)scale };
            }
        }
    }
    
    hasExp = scaleExp || colorExp || opacityExp;
}
    
MarkerSceneRep::MarkerSceneRep() :
    useLayout(false),
    fadeOut(0.0f)
{
}
    
void MarkerSceneRep::enableContents(const SelectionManagerRef &selectManager,
                                    const LayoutManagerRef &layoutManager,
                                    bool enable,ChangeSet &changes)
{
    for (const auto id : drawIDs)
    {
        changes.push_back(new OnOffChangeRequest(id, enable));
    }
    
    if (selectManager && !selectIDs.empty())
        selectManager->enableSelectables(selectIDs, enable);
    
    if (layoutManager)
        layoutManager->enableLayoutObjects(screenShapeIDs, enable);
}
    
void MarkerSceneRep::clearContents(const SelectionManagerRef &selectManager,
                                   const LayoutManagerRef &layoutManager,
                                   ChangeSet &changes,TimeInterval when)
{
    // Just delete everything
    for (const auto id : drawIDs)
    {
        changes.push_back(new RemDrawableReq(id, when));
    }
    drawIDs.clear();
    
    if (selectManager && !selectIDs.empty())
        selectManager->removeSelectables(selectIDs);
    
    if (layoutManager && useLayout)
        layoutManager->removeLayoutObjects(screenShapeIDs);

    screenShapeIDs.clear();
}

Marker::Marker() :
    loc(0,0),
    endLoc(0,0),
    offset(0,0)
{
}

void Marker::addTexID(SimpleIdentity texID)
{
    texIDs.push_back(texID);
}

MarkerManager::MarkerManager() :
    maskProgID(EmptyIdentity)
{
}

MarkerManager::~MarkerManager()
{
    // destructors must never throw, wrap stuff that might fail
    try
    {
        std::unique_lock<std::mutex> guardLock(lock);
        auto reps = std::move(markerReps);
        guardLock.unlock();
        for (auto markerRep : reps)
        {
            delete markerRep;
        }
    }
    WK_STD_DTOR_CATCH()
}

typedef std::map<SimpleIDSet,BasicDrawableBuilderRef> DrawableMap;

Point3dVector MarkerManager::convertGeoPtsToModelSpace(const VectorRing &inPts)
{
    CoordSystemDisplayAdapter *coordAdapt = scene->getCoordAdapter();
    const CoordSystem *coordSys = coordAdapt->getCoordSystem();

    Point3dVector outPts;
    outPts.reserve(inPts.size());
    
    for (const auto &pt: inPts)
    {
        const auto localPt = coordSys->geographicToLocal3d(pt);
        outPts.push_back(coordAdapt->localToDisplay(localPt));
    }
    
    return outPts;
}

SimpleIdentity MarkerManager::addMarkers(const std::vector<Marker> &markers,const MarkerInfo &markerInfo,ChangeSet &changes)
{
    std::vector<Marker*> pointers;
    std::transform(markers.begin(), markers.end(), std::back_inserter(pointers),
                   [](auto &m){ return const_cast<Marker*>(&m); });
    return addMarkers(pointers, markerInfo, changes);
}

SimpleIdentity MarkerManager::addMarkers(const std::vector<Marker *> &markers,const MarkerInfo &markerInfo,ChangeSet &changes)
{
    if (shutdown || !scene)
    {
        return EmptyIdentity;
    }

    auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);
    if (!selectManager || !layoutManager)
    {
        return EmptyIdentity;
    }

    const TimeInterval curTime = scene->getCurrentTime();

    if (maskProgID == EmptyIdentity)
    {
        if (const Program *prog = scene->findProgramByName(MaplyScreenSpaceMaskShader))
        {
            maskProgID = prog->getId();
        }
    }
    
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    const CoordSystem *coordSys = coordAdapter ? coordAdapter->getCoordSystem() : nullptr;
    if (!coordSys)
    {
        return EmptyIdentity;
    }

    auto markerRep = std::make_unique<MarkerSceneRep>();
    markerRep->fadeOut = (float)markerInfo.fadeOut;
    
    // For static markers, sort by texture
    DrawableMap drawables;

    // Screen space markers
    std::vector<ScreenSpaceObjectRef> screenShapes;
    screenShapes.reserve(markers.size());
    
    // Objects to be controlled by the layout layer
    std::vector<LayoutObjectRef> layoutObjects;
    layoutObjects.reserve(markers.size());

    bool cancel = false;
    for (auto &marker : markers)
    {
        if (isShuttingDown() || !renderer)
        {
            cancel = true;
            break;
        }

        // Build the rectangle for this one
        const float width2 = (marker->width == 0.0f ? markerInfo.width : marker->width)/2.0f;
        const float height2 = (marker->height == 0.0f ? markerInfo.height : marker->height)/2.0f;
        
        const Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(marker->loc);
        const Vector3d norm = coordAdapter->normalForLocal(localPt);
        
        // Look for a texture sub mapping
        std::vector<SubTexture> subTexs;
        subTexs.reserve(marker->texIDs.size());
        for (unsigned long long texID : marker->texIDs)
        {
            subTexs.push_back(scene->getSubTexture(texID));
        }
        
        // Build one set of texture coordinates
        std::vector<TexCoord> texCoord =
        {
            { 0.0, 1.0 },
            { 1.0, 1.0 },
            { 1.0, 0.0 },
            { 0.0, 0.0 },
        };

        // Note: This assume they all have the same (or no) sub texture mapping
        if (!subTexs.empty())
        {
            subTexs[0].processTexCoords(texCoord);
        }

        if (markerInfo.screenObject)
        {
            const Point3f pts[4] =
            {
                { -width2 + marker->offset.x(), -height2 + marker->offset.y(), 0.0 },
                {  width2 + marker->offset.x(), -height2 + marker->offset.y(), 0.0 },
                {  width2 + marker->offset.x(),  height2 + marker->offset.y(), 0.0 },
                { -width2 + marker->offset.x(),  height2 + marker->offset.y(), 0.0 },
            };

            float layoutImport = markerInfo.layoutImportance;
            if (layoutManager && marker->layoutImportance < MAXFLOAT)
            {
                layoutImport = marker->layoutImportance;
            }

            std::shared_ptr<LayoutObject> layoutObj;
            std::shared_ptr<ScreenSpaceObject> shape;   // may or may not alias layoutObj
            if (layoutImport < MAXFLOAT)
            {
                markerRep->useLayout = true;
                layoutObj = std::make_shared<LayoutObject>();
                shape = layoutObj;
            }
            else
            {
                shape = std::make_shared<ScreenSpaceObject>();
            }

            if (!marker->uniqueID.empty() && layoutObj)
            {
                layoutObj->uniqueID = marker->uniqueID;
            }

            if (marker->orderBy >= 0)
            {
                shape->setOrderBy(marker->orderBy);
            }

            shape->setPeriod(marker->period);
            
            ScreenSpaceConvexGeometry smGeom;
            smGeom.texIDs.reserve(subTexs.size());
            for (auto &subTex : subTexs)
            {
                smGeom.texIDs.push_back(subTex.texId);
            }

            smGeom.progID = markerInfo.programID;
            smGeom.color = markerInfo.color;
            smGeom.vertexAttrs = marker->vertexAttrs;

            if (marker->colorSet)
            {
                smGeom.color = marker->color;
            }

            for (unsigned int jj=0; jj < 4; jj++)
            {
                smGeom.coords.emplace_back(pts[jj].x(), pts[jj].y());
                smGeom.texCoords.push_back(texCoord[jj]);
            }

            if (marker->isSelectable && marker->selectID != EmptyIdentity)
            {
                shape->setId(marker->selectID);
            }

            shape->setWorldLoc(coordAdapter->localToDisplay(localPt));
            if (marker->hasMotion)
            {
                const Point3d local = coordSys->geographicToLocal3d(marker->endLoc);
                const Point3d display = coordAdapter->localToDisplay(local);
                shape->setMovingLoc(display, marker->startTime, marker->endTime);
            }

            if (marker->lockRotation)
            {
                shape->setRotation(marker->rotation);
            }

            if (markerInfo.fadeIn > 0.0)
            {
                shape->setFade(curTime+markerInfo.fadeIn, curTime);
            }
            else if (markerInfo.fadeOut > 0.0 && markerInfo.fadeOutTime > 0.0)
            {
                // up<down=fade out
                shape->setFade(/*up=*/markerInfo.fadeOutTime, /*down=*/markerInfo.fadeOutTime+markerInfo.fadeOut);
            }

            shape->setVisibility((float)markerInfo.minVis, (float)markerInfo.maxVis);
            shape->setZoomInfo(markerInfo.zoomSlot, markerInfo.minZoomVis, markerInfo.maxZoomVis);
            shape->setOpacityExp(markerInfo.opacityExp);
            shape->setColorExp(markerInfo.colorExp);
            shape->setScaleExp(markerInfo.scaleExp);
            shape->setDrawOrder(markerInfo.drawOrder);
            shape->setDrawPriority(markerInfo.drawPriority);

            shape->setEnable(markerInfo.enable);
            if (markerInfo.startEnable != markerInfo.endEnable)
            {
                shape->setEnableTime(markerInfo.startEnable, markerInfo.endEnable);
            }

            shape->addGeometry(std::move(smGeom));
            markerRep->screenShapeIDs.insert(shape->getId());
            
            // Setup layout points if we have them
            if (!marker->layoutShape.empty() && layoutObj)
            {
                layoutObj->layoutShape = convertGeoPtsToModelSpace(marker->layoutShape);
                layoutObj->layoutRepeat = markerInfo.layoutRepeat;
                layoutObj->layoutOffset = markerInfo.layoutOffset;
                layoutObj->layoutSpacing = markerInfo.layoutSpacing;
                layoutObj->layoutWidth = 2.0f * height2;
            }
            
            // Handle the mask rendering if it's there
            if (marker->maskID != EmptyIdentity && marker->maskRenderTargetID != EmptyIdentity)
            {
                // Make a copy of the geometry, but target it to the mask render target
                const std::vector<ScreenSpaceConvexGeometry> *geom = shape->getGeometry();
                for (ScreenSpaceConvexGeometry entry: *geom)    // N.B.: make a copy
                {
                    entry.vertexAttrs.emplace(a_maskNameID, renderer->getSlotForNameID(a_maskNameID), (int)marker->maskID);
                    entry.renderTargetID = marker->maskRenderTargetID;
                    entry.progID = maskProgID;
                    shape->addGeometry(std::move(entry));
                }
            }
            
            // Set up for the layout layer
            if (layoutImport < MAXFLOAT)
            {
                const Point2d &off = marker->offset;

                layoutObj->selectPts.emplace_back(-width2+off.x(),-height2+off.y());
                layoutObj->selectPts.emplace_back( width2+off.x(),-height2+off.y());
                layoutObj->selectPts.emplace_back( width2+off.x(), height2+off.y());
                layoutObj->selectPts.emplace_back(-width2+off.x(), height2+off.y());

                if (marker->layoutWidth >= 0.0)
                {
                    const auto w2 = marker->layoutWidth / 2.0f;
                    const auto h2 = marker->layoutHeight / 2.0f;
                    layoutObj->layoutPts.emplace_back(-w2+off.x(),-h2+off.y());
                    layoutObj->layoutPts.emplace_back( w2+off.x(),-h2+off.y());
                    layoutObj->layoutPts.emplace_back( w2+off.x(), h2+off.y());
                    layoutObj->layoutPts.emplace_back(-w2+off.x(), h2+off.y());
                }
                else
                {
                    layoutObj->layoutPts = layoutObj->selectPts;
                }

                layoutObj->layoutDebug = markerInfo.layoutDebug;
                layoutObj->clusterGroup = markerInfo.clusterGroup;
                layoutObj->importance = layoutImport;

                // No moving it around
                layoutObj->acceptablePlacement = marker->layoutPlacement;

                // Potentially lay it out with something else (e.g., a label)
                layoutObj->mergeID = marker->mergeID;

                // Start out off, let the layout layer handle the rest
                shape->setEnable(markerInfo.enable);
                if (markerInfo.startEnable != markerInfo.endEnable)
                {
                    shape->setEnableTime(markerInfo.startEnable, markerInfo.endEnable);
                }
                shape->setOffset(Point2d(MAXFLOAT,MAXFLOAT));
            }
            
            if (layoutObj)
            {
                layoutObjects.push_back(std::move(layoutObj));
            }
            else
            {
                if (selectManager)
                {
                    // If the marker doesn't already have an ID, it needs one
                    if (!marker->selectID)
                    {
                        marker->selectID = Identifiable::genId();
                    }

                    markerRep->selectIDs.insert(marker->selectID);

                    Point2f pts2f[4];
                    for (unsigned int jj=0;jj<4;jj++)
                    {
                        pts2f[jj] = Point2f(pts[jj].x(), pts[jj].y());
                    }

                    if (marker->hasMotion)
                    {
                        selectManager->addSelectableMovingScreenRect(marker->selectID,
                                                                     shape->getWorldLoc(),
                                                                     shape->getEndWorldLoc(),
                                                                     shape->getStartTime(),
                                                                     shape->getEndTime(), pts2f,
                                                                     (float)markerInfo.minVis,
                                                                     (float)markerInfo.maxVis,
                                                                     markerInfo.enable);
                    }
                    else
                        {
                        selectManager->addSelectableScreenRect(marker->selectID,
                                                               shape->getWorldLoc(),
                                                               pts2f,
                                                               (float)markerInfo.minVis,
                                                               (float)markerInfo.maxVis,
                                                               markerInfo.enable);
                    }
                    if (!markerInfo.enable)
                    {
                        selectManager->enableSelectable(marker->selectID, false);
                    }
                }

                screenShapes.push_back(std::move(shape));
            }
        }
        else
        {
            const Point3d center = coordAdapter->localToDisplay(localPt);
            const Vector3d up(0,0,1);
            Point3d horiz,vert;
            if (coordAdapter->isFlat())
            {
                horiz = Point3d(1,0,0);
                vert = Point3d(0,1,0);
            }
            else
            {
                horiz = up.cross(norm).normalized();
                vert = norm.cross(horiz).normalized();
            }
            
            const Point3d ll = center - width2*horiz - height2*vert;
            const Point3f pts[4] =
            {
                Vector3dToVector3f(ll),
                Vector3dToVector3f(ll + 2 * width2 * horiz),
                Vector3dToVector3f(ll + 2 * width2 * horiz + 2 * height2 * vert),
                Vector3dToVector3f(ll + 2 * height2 * vert),
            };

            // We're sorting the static drawables by texture, so look for that
            SimpleIDSet texIDs;
            for (const auto &subTex : subTexs)
            {
                texIDs.insert(subTex.texId);
            }
            if (texIDs.empty())
            {
                texIDs.insert(EmptyIdentity);
            }

            BasicDrawableBuilderRef draw;
            const auto it = drawables.find(texIDs);
            if (it != drawables.end())
            {
                draw = it->second;
            }
            else
            {
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
                    std::vector<SimpleIdentity> texIDVec(texIDs.size());
                    std::copy(texIDs.begin(), texIDs.end(), texIDVec.begin());
                    auto tweak = std::make_shared<BasicDrawableTexTweaker>(texIDVec,curTime,marker->period);
                    draw->addTweaker(tweak);
                }
            }
            
            // Toss the geometry into the drawable
            const unsigned vOff = draw->getNumPoints();
            for (unsigned int jj=0; jj < 4; jj++)
            {
                const Point3f &pt = pts[jj];
                draw->addPoint(pt);
                draw->addNormal(norm);
                for (int kk=0; kk < texIDs.size(); kk++)
                {
                    draw->addTexCoord(kk, texCoord[jj]);
                }

                const Point3f localLoc = coordAdapter->getCoordSystem()->geographicToLocal(marker->loc);

                Mbr localMbr = draw->getLocalMbr();
                localMbr.addPoint(Point2f(localLoc.x(),localLoc.y()));
                draw->setLocalMbr(localMbr);
            }
            
            draw->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
            draw->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));

            if (selectManager)
            {
                selectManager->addSelectableRect(marker->selectID,pts,
                                                 (float)markerInfo.minVis,
                                                 (float)markerInfo.maxVis,
                                                 markerInfo.enable);
                markerRep->selectIDs.insert(marker->selectID);
            }
        }
    }

    // Flush out any drawables for the static geometry
    if (!cancel)
    {
        for (const auto &drawable : drawables)
        {
            if (markerInfo.fadeIn > 0.0)
            {
                drawable.second->setFade(curTime,curTime+markerInfo.fadeIn);
            }
            changes.push_back(new AddDrawableReq(drawable.second->getDrawable()));
        }
    }
    drawables.clear();
    
    // Add any simple 2D markers to the scene
    if (!screenShapes.empty())
    {
        if (!cancel && renderer)
        {
            ScreenSpaceBuilder ssBuild(renderer,coordAdapter,renderer->getScale());
            ssBuild.addScreenObjects(screenShapes);
            ssBuild.flushChanges(changes, markerRep->drawIDs);
        }
    }
    
    // And any layout constraints to the layout engine
    if (layoutManager && !layoutObjects.empty() && !cancel)
    {
        layoutManager->addLayoutObjects(std::move(layoutObjects));
    }

    if (!cancel && renderer)
    {
        const SimpleIdentity markerID = markerRep->getId();

        std::lock_guard<std::mutex> guardLock(lock);
        // transfer ownership
        markerReps.insert(markerRep.release());

        return markerID;
    }
    else
    {
        return EmptyIdentity;
    }
}

void MarkerManager::enableMarkers(SimpleIDSet &markerIDs,bool enable,ChangeSet &changes)
{
    const auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);

    std::lock_guard<std::mutex> guardLock(lock);

    MarkerSceneRep dummyRep;
    for (const SimpleIdentity markerID : markerIDs)
    {
        dummyRep.setId(markerID);

        const auto it = markerReps.find(&dummyRep);
        if (it != markerReps.end())
        {
            MarkerSceneRep *markerRep = *it;
            markerRep->enableContents(selectManager, layoutManager, enable, changes);
        }
    }
}

void MarkerManager::removeMarkers(SimpleIDSet &markerIDs,ChangeSet &changes)
{
    if (!scene)
        return;
    
    const auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);

    std::lock_guard<std::mutex> guardLock(lock);

    const TimeInterval curTime = scene->getCurrentTime();
    MarkerSceneRep dummyRep;
    for (const auto markerID : markerIDs)
    {
        dummyRep.setId(markerID);
        const auto it = markerReps.find(&dummyRep);
        if (it == markerReps.end())
        {
            continue;
        }

        MarkerSceneRep *markerRep = *it;

        TimeInterval removeTime = 0.0;
        if (markerRep->fadeOut > 0.0)
        {
            for (const auto did : markerRep->drawIDs)
            {
                changes.push_back(new FadeChangeRequest(did, curTime, curTime + markerRep->fadeOut));
            }
            removeTime = curTime + markerRep->fadeOut;
        }

        markerRep->clearContents(selectManager, layoutManager, changes, removeTime);

        markerReps.erase(it);
        delete markerRep;
    }
}

void MarkerManager::setScene(Scene *inScene)
{
    SceneManager::setScene(inScene);
}

}
