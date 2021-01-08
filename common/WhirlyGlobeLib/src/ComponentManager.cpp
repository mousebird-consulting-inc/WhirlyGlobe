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
    : vectorOffset(0.0,0.0)
    , isSelectable(false)
    , enable(false)
    , underConstruction(false)
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
{
}

ComponentManager::~ComponentManager()
{
    //std::lock_guard<std::mutex> guardLock(lock);
}

void ComponentManager::setScene(Scene *scene)
{
    layoutManager = scene->getManagerNoLock<LayoutManager>(kWKLayoutManager);
    markerManager = scene->getManagerNoLock<MarkerManager>(kWKMarkerManager);
    labelManager = scene->getManagerNoLock<LabelManager>(kWKLabelManager);
    vectorManager = scene->getManagerNoLock<VectorManager>(kWKVectorManager);
    wideVectorManager = scene->getManagerNoLock<WideVectorManager>(kWKWideVectorManager);
    shapeManager = scene->getManagerNoLock<ShapeManager>(kWKShapeManager);
    chunkManager = scene->getManagerNoLock<SphericalChunkManager>(kWKSphericalChunkManager);
    loftManager = scene->getManagerNoLock<LoftManager>(kWKLoftedPolyManager);
    billManager = scene->getManagerNoLock<BillboardManager>(kWKBillboardManager);
    geomManager = scene->getManagerNoLock<GeometryManager>(kWKGeometryManager);
    fontTexManager = scene->getFontTextureManager();
    partSysManager = scene->getManagerNoLock<ParticleSystemManager>(kWKParticleSystemManager);
}

void ComponentManager::addComponentObject(const ComponentObjectRef &compObj, ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    compObj->underConstruction = false;
    compObjs[compObj->getId()] = compObj;

    // Does the new object have a UUID?
    if (!compObj->uuid.empty())
    {
        // Is the current representation for that UUID already set?
        const auto hit = representations.find(compObj->uuid);
        // Enable if it matches, disable otherwise.
        // Representation must be none/empty if no current representation is set.
        const bool enable = (hit != representations.end()) ? (compObj->representation == hit->second) : compObj->representation.empty();

        if (enable != compObj->enable)
        {
            enableComponentObject(compObj, enable, changes);
        }
    }
}

bool ComponentManager::hasComponentObject(SimpleIdentity compID)
{
    std::lock_guard<std::mutex> guardLock(lock);

    auto it = compObjs.find(compID);
    return it != compObjs.end();
}
    
void ComponentManager::removeComponentObject(PlatformThreadInfo *threadInfo,SimpleIdentity compID, ChangeSet &changes)
{
    SimpleIDSet compIDs;
    compIDs.insert(compID);
    
    removeComponentObjects(threadInfo,compIDs, changes);
}

void ComponentManager::removeComponentObjects(PlatformThreadInfo *threadInfo,const std::vector<ComponentObjectRef> &compObjs,ChangeSet &changes)
{
    SimpleIDSet compIDs;
    
    for (auto compObj: compObjs) {
        compIDs.insert(compObj->getId());
    }
    
    removeComponentObjects(threadInfo,compIDs, changes);
}

void ComponentManager::removeComponentObjects(PlatformThreadInfo *threadInfo,const SimpleIDSet &compIDs,ChangeSet &changes)
{
    if (compIDs.empty())
        return;

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
            labelManager->removeLabels(threadInfo,compObj->labelIDs, changes);
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

    enableComponentObjects(compRefs, enable, changes);
}

void ComponentManager::enableComponentObject(const ComponentObjectRef &compObj, bool enable, ChangeSet &changes)
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
        for (auto const & it : compObj->chunkIDs)
        {
            chunkManager->enableChunk(it, enable, changes);
        }
    }
    if (partSysManager && !compObj->partSysIDs.empty())
    {
        for (auto const it : compObj->partSysIDs)
        {
            partSysManager->enableParticleSystem(it, enable, changes);
        }
    }
}

void ComponentManager::enableComponentObjects(const std::vector<ComponentObjectRef> &compRefs, bool enable, ChangeSet &changes)
{
    for (const auto &compObj : compRefs)
    {
        enableComponentObject(compObj, enable, changes);
    }
}

template <typename TIter>
void ComponentManager::setRepresentation(const std::string &repName,
                                         TIter beg, TIter end,
                                         ChangeSet &changes)
{
    std::vector<ComponentObjectRef> enableObjs, disableObjs;

    {
        std::lock_guard<std::mutex> guardLock(lock);

        for (; beg != end; ++beg)
        {
            const std::string &uuid = *beg;
            
            if (repName.empty())
            {
                // Remove entries, return to default (un-set) state
                representations.erase(uuid);
            }
            else
            {
                const auto insertResult = representations.insert(std::make_pair(uuid, repName));
                if (!insertResult.second)
                {
                    insertResult.first->second = repName;
                }
            }

            for (const auto &kvp : compObjs)
            {
                const ComponentObjectRef &obj = kvp.second;
                if (obj->uuid == uuid)
                {
                    // Enable matches, disable non-matches
                    ((obj->representation == repName) ? enableObjs : disableObjs).push_back(obj);
                }
            }
        }
    }

    if (!enableObjs.empty()) enableComponentObjects(enableObjs, true, changes);
    if (!disableObjs.empty()) enableComponentObjects(disableObjs, false, changes);
}

void ComponentManager::setRepresentation(const std::string &repName,
                                         const std::set<std::string> &uuids,
                                         ChangeSet &changes)
{
    setRepresentation(repName, uuids.begin(), uuids.end(), changes);
}

void ComponentManager::setRepresentation(const std::string &repName,
                                         const std::unordered_set<std::string> &uuids,
                                         ChangeSet &changes)
{
    setRepresentation(repName, uuids.begin(), uuids.end(), changes);
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
        if (geomManager && !compObj->geomIDs.empty()) {
            geomManager->setUniformBlock(compObj->geomIDs,uniBlock,bufferID,changes);
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
