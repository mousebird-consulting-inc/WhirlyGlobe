/*  BillboardManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2021 mousebird consulting
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

#import "BillboardManager.h"
#import "WhirlyKitLog.h"
#import "SharedAttributes.h"

using namespace Eigen;

namespace WhirlyKit {

SingleBillboardPoly::SingleBillboardPoly() :
    color(255,255,255,255),
    texId(EmptyIdentity)
{
}

BillboardInfo::BillboardInfo() :
    color(RGBAColor::white()),
    orient(Eye)
{
}

BillboardInfo::BillboardInfo(const Dictionary &dict) :
    BaseInfo(dict),
    color(RGBAColor::white())
{
    // We have different defaults than the base
    zBufferRead = dict.getBool(MaplyZBufferRead,true);
    zBufferWrite = dict.getBool(MaplyZBufferWrite, false);

    color = dict.getColor(MaplyColor,color);

    const auto orientStr = dict.getString(MaplyBillboardOrient, std::string());
    orient = (orientStr == MaplyBillboardOrientEye) ? Eye : Ground;
}

Billboard::Billboard() :
    center(Point3d(0,0,0)),
    size(Point2d(0,0)),
    isSelectable(false),
    selectID(EmptyIdentity)
{
}
    
BillboardSceneRep::BillboardSceneRep() :
    Identifiable(),
    fade(0.0f)
{
}

BillboardSceneRep::BillboardSceneRep(SimpleIdentity inId) :
    Identifiable(inId),
    fade(0.0f)
{
}

void BillboardSceneRep::clearContents(SelectionManagerRef &selectManager,ChangeSet &changes,TimeInterval when)
{
    for (const auto it: drawIDs){
        changes.push_back(new RemDrawableReq(it,when));
    }
    if (selectManager && !selectIDs.empty()){
        selectManager->removeSelectables(selectIDs);
    }
}

BillboardBuilder::BillboardBuilder(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changes,BillboardSceneRep *sceneRep,const BillboardInfo &billInfo,SimpleIdentity billboardProgram,SimpleIdentity texId) :
    scene(scene),
    sceneRender(sceneRender),
    changes(changes),
    sceneRep(sceneRep),
    billInfo(billInfo),
    drawable(nullptr),
    billboardProgram(billboardProgram),
    texId(texId)
{

}

BillboardBuilder::~BillboardBuilder()
{
    flush();
}

void BillboardBuilder::addBillboard(const Point3d &center, const Point2dVector &pts,
                                    const std::vector<WhirlyKit::TexCoord> &texCoords,
                                    const WhirlyKit::RGBAColor *inColor,
                                    const SingleVertexAttributeSet &vertAttrs)
{
    if (pts.size() != 4)
    {
      //  NSLog(@"Only expecting 4 point polygons in BillboardDrawableBuilder");
        return;
    }
        
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
    // Get the drawable ready
    if (!drawable || !drawable->compareVertexAttributes(vertAttrs) ||
        (drawable->getNumPoints()+4 > MaxDrawablePoints) ||
        (drawable->getNumTris()+2 > MaxDrawableTriangles))
    {
        if (drawable)
            flush();
            
        drawable = sceneRender->makeBillboardDrawableBuilder("Billboard");
        drawable->Init();
        drawable->setType(Triangles);
        billInfo.setupBasicDrawable(drawable);
        drawable->setGroundMode(billInfo.orient == BillboardInfo::Ground);
        drawable->setProgram(billboardProgram);
        drawable->setTexId(0,texId);
        if (!vertAttrs.empty())
        {
            SingleVertexAttributeInfoSet vertInfoSet;
            VertexAttributeSetConvert(vertAttrs,vertInfoSet);
            drawable->setVertexAttributes(vertInfoSet);
        }
        //drawable->setForceZBufferOn(true);    // ?
    }
        
    const RGBAColor color = (inColor ? *inColor : billInfo.color);

    const double len = center.norm();
    const Point3d centerOnSphere = (len != 0.0) ? (center / len) : center;

    // Normal is straight up
    const Point3d localPt = coordAdapter->displayToLocal(centerOnSphere);
    const Point3d axisY = coordAdapter->normalForLocal(localPt);
        
    const auto startPoint = drawable->getNumPoints();
    for (unsigned int ii=0;ii<4;ii++)
    {
        drawable->addPoint(center);
        drawable->addOffset(Point3d(pts[ii].x(),pts[ii].y(),0.0));
        drawable->addTexCoord(0,texCoords[ii]);
        drawable->addNormal(axisY);
        drawable->addColor(color);
        if (!vertAttrs.empty())
            drawable->addVertexAttributes(vertAttrs);
    }
    drawable->addTriangle(BasicDrawable::Triangle(startPoint+0,startPoint+1,startPoint+2));
    drawable->addTriangle(BasicDrawable::Triangle(startPoint+0,startPoint+2,startPoint+3));
}
    
void BillboardBuilder::flush()
{
    if (drawable)
    {
        if (drawable->getNumPoints() > 0)
        {
            //drawable->setLocalMbr(drawMbr);
            sceneRep->drawIDs.insert(drawable->getDrawableID());
            
// TODO Port (fade isn't supported yet)
//           if (billInfo.fade > 0.0)
//           {
//                TimeInterval curTime = TimeGetCurrent();
//                drawable->setFade(curTime, curTime+billInfo.fade);
//            }
            changes.push_back(new AddDrawableReq(drawable->getDrawable()));
        }
        drawable = nullptr;
    }
}


BillboardManager::~BillboardManager()
{
    std::lock_guard<std::mutex> guardLock(lock);

    for (auto sceneRep : sceneReps)
        delete sceneRep;
    sceneReps.clear();
}

typedef std::map<SimpleIdentity,BillboardBuilderRef> BuilderMap;

/// Add billboards for display
SimpleIdentity BillboardManager::addBillboards(const std::vector<Billboard*> &billboards,const BillboardInfo &billboardInfo,ChangeSet &changes)
{
    const auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);

    auto sceneRep = new BillboardSceneRep();
    sceneRep->fade = (float)billboardInfo.fade;

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    // One builder per texture
    BuilderMap drawBuilders;
        
    // Work through the billboards, constructing as we go
    for (auto *billboard : billboards)
    {
        // Work through the individual polygons
        for (const auto &billPoly : billboard->polys)
        {
            const auto it = drawBuilders.insert(std::make_pair(billPoly.texId, BillboardBuilderRef()));
            if (it.second)
            {
                // new item inserted, fill in the value with a new builder
                it.first->second = std::make_shared<BillboardBuilder>(scene,renderer,changes,sceneRep,billboardInfo,
                                                                      billboardInfo.programID,billPoly.texId);
            }
            const auto &drawBuilder = it.first->second;
            drawBuilder->addBillboard(billboard->center, billPoly.pts, billPoly.texCoords,
                                      &billPoly.color, billPoly.vertexAttrs);
        }
            
        // While we're at it, let's add this to the selection layer
        if (selectManager && billboard->isSelectable)
        {
            // If the marker doesn't already have an ID, it needs one
            if (!billboard->selectID)
                billboard->selectID = Identifiable::genId();
                
            sceneRep->selectIDs.insert(billboard->selectID);
                
                // Normal is straight up
            const Point3d localPt = coordAdapter->displayToLocal(billboard->center);
            const Point3d axisY = coordAdapter->normalForLocal(localPt);
                
            selectManager->addSelectableBillboard(billboard->selectID, billboard->center, axisY, billboard->size,
                                                  (float)billboardInfo.minVis, (float)billboardInfo.maxVis,
                                                  billboardInfo.enable);
        }
    }
        
    // Flush out the changes and tear down the builders
    for (const auto &it : drawBuilders)
    {
        const auto &drawBuilder = it.second;
        drawBuilder->flush();
    }
    drawBuilders.clear();
        
    const SimpleIdentity billID = sceneRep->getId();

    std::lock_guard<std::mutex> guardLock(lock);
    sceneReps.insert(sceneRep);
    
    return billID;
}

void BillboardManager::enableBillboards(SimpleIDSet &billIDs,bool enable,ChangeSet &changes)
{
    const auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    std::lock_guard<std::mutex> guardLock(lock);

    for (auto billID : billIDs)
    {
        BillboardSceneRep dummyRep(billID);
        auto it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            const auto *billRep = *it;
            for (auto drawID : billRep->drawIDs)
            {
                changes.push_back(new OnOffChangeRequest(drawID, enable));
            }
                
            if (selectManager && !billRep->selectIDs.empty())
            {
                selectManager->enableSelectables(billRep->selectIDs, enable);
            }
        }
    }
}

/// Remove a group of billboards named by the given ID
void BillboardManager::removeBillboards(SimpleIDSet &billIDs,ChangeSet &changes)
{
    auto selectManager = scene->getManager<SelectionManager>(kWKSelectionManager);
    std::lock_guard<std::mutex> guardLock(lock);

    TimeInterval curTime = scene->getCurrentTime();
    for (auto billID : billIDs)
    {
        BillboardSceneRep dummyRep(billID);
        auto it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            auto *sceneRep = *it;

            TimeInterval removeTime = 0.0;
            if (sceneRep->fade > 0.0)
            {
                for (auto id : sceneRep->drawIDs)
                {
                    changes.push_back(new FadeChangeRequest(id, curTime, curTime+sceneRep->fade));
                }
                
                removeTime = curTime + sceneRep->fade;
            }
            
            sceneRep->clearContents(selectManager,changes,removeTime);
            sceneReps.erase(it);
            delete sceneRep;
        }
    }
}

}
