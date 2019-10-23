/*
 *  ComponentManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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

#import "ComponentManager.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
ComponentObject::ComponentObject()
: vectorOffset(0.0,0.0), isSelectable(false),
enable(false), underConstruction(false)
{
}
    
ComponentObject::~ComponentObject()
{
}

void ComponentObject::clear()
{
    markerIDs.clear();
    labelIDs.clear();
    vectorIDs.clear();
    wideVectorIDs.clear();
    shapeIDs.clear();
    chunkIDs.clear();
    loftIDs.clear();
    billIDs.clear();
    geomIDs.clear();
    partSysIDs.clear();
    selectIDs.clear();
    drawStringIDs.clear();
}

ComponentManager::ComponentManager()
    : layoutManager(NULL), markerManager(NULL), labelManager(NULL), vectorManager(NULL), wideVectorManager(NULL),
    shapeManager(NULL), loftManager(NULL), billManager(NULL), geomManager(NULL),
    fontTexManager(NULL), partSysManager(NULL)
{
}
    
ComponentManager::~ComponentManager()
{
}
    
void ComponentManager::setScene(Scene *scene)
{
    layoutManager = (LayoutManager *)scene->getManagerNoLock(kWKLayoutManager);
    markerManager = (MarkerManager *)scene->getManagerNoLock(kWKMarkerManager);
    labelManager = (LabelManager *)scene->getManagerNoLock(kWKLabelManager);
    vectorManager = (VectorManager *)scene->getManagerNoLock(kWKVectorManager);
    wideVectorManager = (WideVectorManager *)scene->getManagerNoLock(kWKWideVectorManager);
    shapeManager = (ShapeManager *)scene->getManagerNoLock(kWKShapeManager);
    chunkManager = (SphericalChunkManager *)scene->getManagerNoLock(kWKSphericalChunkManager);
    loftManager = (LoftManager *)scene->getManagerNoLock(kWKLoftedPolyManager);
    billManager = (BillboardManager *)scene->getManagerNoLock(kWKBillboardManager);
    geomManager = (GeometryManager *)scene->getManagerNoLock(kWKGeometryManager);
    fontTexManager = scene->getFontTextureManager();
    partSysManager = (ParticleSystemManager *)scene->getManagerNoLock(kWKParticleSystemManager);
}

void ComponentManager::addComponentObject(ComponentObjectRef compObj)
{
    std::lock_guard<std::mutex> guardLock(lock);
    
    compObj->underConstruction = false;
    compObjs[compObj->getId()] = compObj;
}

bool ComponentManager::hasComponentObject(SimpleIdentity compID)
{
    std::lock_guard<std::mutex> guardLock(lock);

    auto it = compObjs.find(compID);
    return it != compObjs.end();
}
    
void ComponentManager::removeComponentObject(SimpleIdentity compID, ChangeSet &changes)
{
    SimpleIDSet compIDs;
    compIDs.insert(compID);
    
    removeComponentObjects(compIDs, changes);
}

void ComponentManager::removeComponentObjects(const std::vector<ComponentObjectRef> &compObjs,ChangeSet &changes)
{
    SimpleIDSet compIDs;
    
    for (auto compObj: compObjs) {
        compIDs.insert(compObj->getId());
    }
    
    removeComponentObjects(compIDs, changes);
}

void ComponentManager::removeComponentObjects(const SimpleIDSet &compIDs,ChangeSet &changes)
{
    std::vector<ComponentObjectRef> compRefs;
    
    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);

        for (SimpleIdentity compID : compIDs) {
            auto it = compObjs.find(compID);
            if (it == compObjs.end())
            {
                wkLogLevel(Warn,"Tried to delete object that doesn't exist: %d",compID);
                return;
            }
            ComponentObjectRef compObj = it->second;
            
            if (compObj->underConstruction) {
                wkLogLevel(Warn,"Deleting an object that's under construction");
            }
            
            compRefs.push_back(compObj);
            
            compObjs.erase(it);
        }
    }
    
    for (ComponentObjectRef compObj : compRefs) {
        // Get rid of the various layer objects
        if (!compObj->markerIDs.empty())
            markerManager->removeMarkers(compObj->markerIDs, changes);
        if (!compObj->labelIDs.empty())
            labelManager->removeLabels(compObj->labelIDs, changes);
        if (!compObj->vectorIDs.empty())
            vectorManager->removeVectors(compObj->vectorIDs, changes);
        if (!compObj->wideVectorIDs.empty())
            wideVectorManager->removeVectors(compObj->wideVectorIDs, changes);
        if (!compObj->shapeIDs.empty())
            shapeManager->removeShapes(compObj->shapeIDs, changes);
        if (!compObj->loftIDs.empty())
            loftManager->removeLoftedPolys(compObj->loftIDs, changes);
        if (!compObj->chunkIDs.empty())
            chunkManager->removeChunks(compObj->chunkIDs, changes);
        if (!compObj->billIDs.empty())
            billManager->removeBillboards(compObj->billIDs, changes);
        if (!compObj->geomIDs.empty())
            geomManager->removeGeometry(compObj->geomIDs, changes);
        if (!compObj->drawStringIDs.empty())
        {
            // Giving the fonts 2s to stick around
            //       This avoids problems with texture being paged out.
            //       Without this we lose the textures before we're done with them
            TimeInterval when = scene->getCurrentTime() + 2.0;
            for (SimpleIdentity dStrID : compObj->drawStringIDs)
                fontTexManager->removeString(dStrID, changes, when);
        }
        if (!compObj->partSysIDs.empty())
        {
            for (SimpleIdentity partSysID : compObj->partSysIDs)
                partSysManager->removeParticleSystem(partSysID, changes);
        }
    }
}
    
void ComponentManager::enableComponentObject(SimpleIdentity compID, bool enable, ChangeSet &changes)
{
    SimpleIDSet compIDs;
    compIDs.insert(compID);
    
    enableComponentObjects(compIDs, enable, changes);
}

void ComponentManager::enableComponentObjects(const SimpleIDSet &compIDs,bool enable,ChangeSet &changes)
{
    std::vector<ComponentObjectRef> compRefs;
    
    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (SimpleIdentity compID : compIDs)
        {
            auto it = compObjs.find(compID);
            if (it == compObjs.end())
            {
                wkLogLevel(Warn,"Tried to enable/disable object that doesn't exist");
                return;
            }
            ComponentObjectRef compObj = it->second;
            
            if (compObj->underConstruction) {
                wkLogLevel(Warn,"Disable/enabled an object that's under construction");
            }
            
            compRefs.push_back(compObj);
        }
    }
    
    for (ComponentObjectRef compObj: compRefs)
    {
        // Note: Should lock just around this component object
        //       But I'm not sure I want one std::mutex per object
        compObj->enable = enable;

        if (!compObj->vectorIDs.empty())
            vectorManager->enableVectors(compObj->vectorIDs, enable, changes);
        if (!compObj->wideVectorIDs.empty())
            wideVectorManager->enableVectors(compObj->wideVectorIDs, enable, changes);
        if (!compObj->markerIDs.empty())
            markerManager->enableMarkers(compObj->markerIDs, enable, changes);
        if (!compObj->labelIDs.empty())
            labelManager->enableLabels(compObj->labelIDs, enable, changes);
        if (!compObj->shapeIDs.empty())
            shapeManager->enableShapes(compObj->shapeIDs, enable, changes);
        if (!compObj->billIDs.empty())
            billManager->enableBillboards(compObj->billIDs, enable, changes);
        if (!compObj->loftIDs.empty())
            loftManager->enableLoftedPolys(compObj->loftIDs, enable, changes);
        if (geomManager && !compObj->geomIDs.empty())
            geomManager->enableGeometry(compObj->geomIDs, enable, changes);
        if (!compObj->chunkIDs.empty())
        {
            for (SimpleIDSet::iterator it = compObj->chunkIDs.begin();
                 it != compObj->chunkIDs.end(); ++it)
                chunkManager->enableChunk(*it, enable, changes);
        }
        if (partSysManager && !compObj->partSysIDs.empty()) {
            for (SimpleIDSet::iterator it = compObj->partSysIDs.begin();
                 it != compObj->partSysIDs.end(); ++it)
                partSysManager->enableParticleSystem(*it, enable, changes);
        }
    }
}
    
void ComponentManager::setUniformBlock(const SimpleIDSet &compIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes)
{
    std::vector<ComponentObjectRef> compRefs;
    
    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (SimpleIdentity compID : compIDs)
        {
            auto it = compObjs.find(compID);
            if (it == compObjs.end())
            {
                wkLogLevel(Warn,"Tried to set uniform block on object that doesn't exist");
                return;
            }
            ComponentObjectRef compObj = it->second;
            
            compRefs.push_back(compObj);
        }
    }
    
    for (auto compObj : compRefs) {
        if (shapeManager && !compObj->shapeIDs.empty()) {
            shapeManager->setUniformBlock(compObj->shapeIDs,uniBlock,bufferID,changes);
        }
        if (partSysManager && !compObj->partSysIDs.empty()) {
            partSysManager->setUniformBlock(compObj->partSysIDs,uniBlock,bufferID,changes);
        }
        // TODO: Fill this in for the other object types
    }
}
    
std::vector<std::pair<ComponentObjectRef,VectorObjectRef> > ComponentManager::findVectors(const Point2d &pt,double maxDist,ViewStateRef viewState,const Point2f &frameSize,bool multi)
{
    std::vector<ComponentObjectRef> compRefs;
    std::vector<std::pair<ComponentObjectRef,VectorObjectRef> > rets;

    // Copy out the vectors that might be candidates
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (auto it: compObjs) {
            auto compObj = it.second;
            if (compObj->enable && compObj->isSelectable && !compObj->vecObjs.empty())
                compRefs.push_back(compObj);
        }
    }
    
    // Work through the vector objects
    for (auto compObj: compRefs) {
        auto center = compObj->vectorOffset;
        Point2d coord;
        coord.x() = pt.x()-center.x();
        coord.y() = pt.y()-center.y();
        
        for (auto vecObj: compObj->vecObjs) {
            if (vecObj->pointInside(pt)) {
                
                rets.push_back(std::make_pair(compObj, vecObj));
                
                if (!multi)
                    break;
                continue;
            }
            if (vecObj->pointNearLinear(coord, maxDist, viewState, frameSize)) {
                rets.push_back(std::make_pair(compObj, vecObj));

                if (!multi)
                    break;
            }
        }
        
        if (!multi && !rets.empty())
            break;
    }
    
    return rets;
}

}
