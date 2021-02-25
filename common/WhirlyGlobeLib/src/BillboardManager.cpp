/*
 *  BillboardManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2019 mousebird consulting
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
    color(RGBAColor())
{
}

BillboardInfo::BillboardInfo(const Dictionary &dict)
: BaseInfo(dict)
{
    // We have different defaults than the base
    zBufferRead = dict.getBool(MaplyZBufferRead,true);
    zBufferWrite = dict.getBool(MaplyZBufferWrite, false);

    color = dict.getColor(MaplyColor,RGBAColor(255,255,255,255));
    std::string orientStr = dict.getString(MaplyBillboardOrient, "");
    if (orientStr == MaplyBillboardOrientEye)
        orient = Eye;
    else
        orient = Ground;
}

Billboard::Billboard() :
    center(Point3d(0,0,0)),
    size(Point2d(0,0)),
    isSelectable(false),
    selectID(EmptyIdentity)
{
}
    
BillboardSceneRep::BillboardSceneRep()
{
}

BillboardSceneRep::BillboardSceneRep(SimpleIdentity inId) :
    Identifiable(inId)
{
}

BillboardSceneRep::~BillboardSceneRep()
{
}

void BillboardSceneRep::clearContents(SelectionManager *selectManager,ChangeSet &changes,TimeInterval when)
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
    drawable(NULL),
    billboardProgram(billboardProgram),
    texId(texId)
{

}

BillboardBuilder::~BillboardBuilder()
{
    flush();
}

void BillboardBuilder::addBillboard(Point3d center, const Point2dVector &pts, const std::vector<WhirlyKit::TexCoord> &texCoords, const WhirlyKit::RGBAColor *inColor, const SingleVertexAttributeSet &vertAttrs)
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
        //        drawMbr.reset();
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
        //        drawable->setForceZBufferOn(true);
    }
        
    RGBAColor color = (inColor ? *inColor : billInfo.color);
    
    Point3d centerOnSphere = center;
    double len = sqrt(centerOnSphere.x()*centerOnSphere.x() + centerOnSphere.y()*centerOnSphere.y() + centerOnSphere.z()*centerOnSphere.z());
    if (len != 0.0)
        centerOnSphere /= len;
        
    // Normal is straight up
    Point3d localPt = coordAdapter->displayToLocal(centerOnSphere);
    Point3d axisY = coordAdapter->normalForLocal(localPt);
        
    int startPoint = drawable->getNumPoints();
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
            //            drawable->setLocalMbr(drawMbr);
            sceneRep->drawIDs.insert(drawable->getDrawableID());
            
// TODO Port (fade isn't supported yet)
//           if (billInfo.fade > 0.0)
//           {
//                TimeInterval curTime = TimeGetCurrent();
//                drawable->setFade(curTime, curTime+billInfo.fade);
//            }
            changes.push_back(new AddDrawableReq(drawable->getDrawable()));
        }
        drawable = NULL;
    }
}


BillboardManager::BillboardManager()
{
}

BillboardManager::~BillboardManager()
{
    for (BillboardSceneRepSet::iterator it = sceneReps.begin(); it != sceneReps.end(); ++it)
        delete *it;
    sceneReps.clear();
}

typedef std::map<SimpleIdentity,BillboardBuilderRef> BuilderMap;

/// Add billboards for display
SimpleIdentity BillboardManager::addBillboards(std::vector<Billboard*> billboards,const BillboardInfo &billboardInfo,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);


    BillboardSceneRep *sceneRep = new BillboardSceneRep();
    sceneRep->fade = billboardInfo.fade;
        
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        
    // One builder per texture
    BuilderMap drawBuilders;
        
    // Work through the billboards, constructing as we go
    for (Billboard *billboard : billboards)
    {
        // Work through the individual polygons
        for (const SingleBillboardPoly &billPoly : billboard->polys)
        {
            BuilderMap::iterator it = drawBuilders.find(billPoly.texId);
            BillboardBuilderRef drawBuilder;
            // Need a new one
            if (it == drawBuilders.end())
            {
                drawBuilder = BillboardBuilderRef(new BillboardBuilder(scene,renderer,changes,sceneRep,billboardInfo,billboardInfo.programID,billPoly.texId));
                drawBuilders[billPoly.texId] = drawBuilder;
            } else
                drawBuilder = it->second;
            
            drawBuilder->addBillboard(billboard->center, billPoly.pts, billPoly.texCoords, &billPoly.color, billPoly.vertexAttrs);
        }
            
        // While we're at it, let's add this to the selection layer
        if (selectManager && billboard->isSelectable)
        {
            // If the marker doesn't already have an ID, it needs one
            if (!billboard->selectID)
                billboard->selectID = Identifiable::genId();
                
            sceneRep->selectIDs.insert(billboard->selectID);
                
                // Normal is straight up
            Point3d localPt = coordAdapter->displayToLocal(billboard->center);
            Point3d axisY = coordAdapter->normalForLocal(localPt);
                
            selectManager->addSelectableBillboard(billboard->selectID, billboard->center, axisY, billboard->size, billboardInfo.minVis, billboardInfo.maxVis, billboardInfo.enable);
        }
    }
        
    // Flush out the changes and tear down the builders
    for (BuilderMap::iterator it = drawBuilders.begin(); it != drawBuilders.end(); ++it)
    {
        BillboardBuilderRef drawBuilder = it->second;
        drawBuilder->flush();
    }
    drawBuilders.clear();
        
    SimpleIdentity billID = sceneRep->getId();
        
    std::lock_guard<std::mutex> guardLock(billLock);
    sceneReps.insert(sceneRep);
    
    return billID;
}

void BillboardManager::enableBillboards(SimpleIDSet &billIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
        
    std::lock_guard<std::mutex> guardLock(billLock);

    for (SimpleIDSet::iterator bit = billIDs.begin();bit != billIDs.end();++bit)
    {
        BillboardSceneRep dummyRep(*bit);
        BillboardSceneRepSet::iterator it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            BillboardSceneRep *billRep = *it;
            for (SimpleIDSet::iterator dit = billRep->drawIDs.begin(); dit != billRep->drawIDs.end(); ++dit)
                changes.push_back(new OnOffChangeRequest((*dit), enable));
                
            if (selectManager && !billRep->selectIDs.empty())
                selectManager->enableSelectables(billRep->selectIDs, enable);
        }
    }
}

/// Remove a group of billboards named by the given ID
void BillboardManager::removeBillboards(SimpleIDSet &billIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
        
    std::lock_guard<std::mutex> guardLock(billLock);

    TimeInterval curTime = scene->getCurrentTime();
    for (SimpleIDSet::iterator bit = billIDs.begin();bit != billIDs.end();++bit)
    {
        BillboardSceneRep dummyRep(*bit);
        BillboardSceneRepSet::iterator it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            BillboardSceneRep *sceneRep = *it;
            
            TimeInterval removeTime = 0.0;
            if (sceneRep->fade > 0.0)
            {
                for (SimpleIDSet::iterator it = sceneRep->drawIDs.begin(); it != sceneRep->drawIDs.end(); ++it)
                    changes.push_back(new FadeChangeRequest(*it, curTime, curTime+sceneRep->fade));
                
                removeTime = curTime + sceneRep->fade;
            }
            
            sceneRep->clearContents(selectManager,changes,removeTime);
            sceneReps.erase(it);
            delete sceneRep;
        }
    }
}

}
