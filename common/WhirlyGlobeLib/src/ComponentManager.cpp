/*  ComponentManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import "ComponentManager.h"
#import "WhirlyKitLog.h"
#import "SharedAttributes.h"

//#define LOG_REPRESENTATIONS

namespace WhirlyKit
{

static constexpr size_t TypicalRepUUIDs = 100;
static constexpr size_t TypicalUUIDComps = 1000;

ComponentObject::ComponentObject(bool enable, bool selectable) :
    enable(enable),
    isSelectable(selectable)
{
}

ComponentObject::ComponentObject(bool enable, bool selectable, const Dictionary &desc) :
    ComponentObject(enable, selectable)
{
    if (!desc.empty())
    {
        this->enable = desc.getBool(MaplyEnable, enable);
        this->uuid = desc.getString(MaplyUUIDDesc);
        this->representation = desc.getString(MaplyRepresentationDesc);
    }
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

void ComponentManager::setScene(Scene *scene)
{
    SceneManager::setScene(scene);

    shapeManager      = scene ? scene->getManager<ShapeManager>(kWKShapeManager) : nullptr;
#if !MAPLY_MINIMAL
    layoutManager     = scene ? scene->getManager<LayoutManager>(kWKLayoutManager) : nullptr;
    markerManager     = scene ? scene->getManager<MarkerManager>(kWKMarkerManager) : nullptr;
    labelManager      = scene ? scene->getManager<LabelManager>(kWKLabelManager) : nullptr;
    vectorManager     = scene ? scene->getManager<VectorManager>(kWKVectorManager) : nullptr;
    wideVectorManager = scene ? scene->getManager<WideVectorManager>(kWKWideVectorManager) : nullptr;
    chunkManager      = scene ? scene->getManager<SphericalChunkManager>(kWKSphericalChunkManager) : nullptr;
    loftManager       = scene ? scene->getManager<LoftManager>(kWKLoftedPolyManager) : nullptr;
    billManager       = scene ? scene->getManager<BillboardManager>(kWKBillboardManager) : nullptr;
    geomManager       = scene ? scene->getManager<GeometryManager>(kWKGeometryManager) : nullptr;
    partSysManager    = scene ? scene->getManager<ParticleSystemManager>(kWKParticleSystemManager) : nullptr;
#endif //!MAPLY_MINIMAL
}

void ComponentManager::addComponentObject(const ComponentObjectRef &compObj, ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(lock);

    compObj->underConstruction = false;
    compObjsById[compObj->getId()] = compObj;

    // Does the new object have a UUID?
    if (!compObj->uuid.empty())
    {
        if (compObjsByUUID.empty())
        {
            compObjsByUUID.reserve(TypicalUUIDComps);
        }
        
        // track it by that UUID
        compObjsByUUID.insert(std::make_pair(compObj->uuid, compObj));

        // Is the current representation for that UUID already set?
        const auto hit = representations.find(compObj->uuid);

        // If a representation is set, show this item if it matches that representation.
        // If no representation is set, show this item if its representation is blank.
        const bool enable = (hit != representations.end()) ? (compObj->representation == hit->second) : compObj->representation.empty();

#ifdef LOG_REPRESENTATIONS
        if (enable)
        wkLogLevel(Verbose, "CO id=%lld uuid=%s rep=%s active=%s result=%s",
                   compObj->getId(), compObj->uuid.c_str(), compObj->representation.c_str(),
                   (hit != representations.end())?hit->second.c_str():"(none)",
                   enable ? "enabled" : "disabled");
#endif

        enableComponentObject(compObj, enable, changes);
    }
}

bool ComponentManager::hasComponentObject(SimpleIdentity compID)
{
    std::lock_guard<std::mutex> guardLock(lock);

    const auto it = compObjsById.find(compID);
    return it != compObjsById.end();
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::removeComponentObject(PlatformThreadInfo *threadInfo,
                                             SimpleIdentity compID,
                                             ChangeSet &changes,
                                             bool disposeAfterRemoval)
{
    SimpleIDSet compIDs { compID };
    removeComponentObjects(threadInfo,compIDs, changes, disposeAfterRemoval);
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::removeComponentObjects(PlatformThreadInfo *threadInfo,
                                              const std::vector<ComponentObjectRef> &compObjs,
                                              ChangeSet &changes,
                                              bool disposeAfterRemoval)
{
    SimpleIDSet compIDs;

    for (const auto &compObj: compObjs)
    {
        compIDs.insert(compObj->getId());
    }

    removeComponentObjects(threadInfo,compIDs, changes, disposeAfterRemoval);
}

void ComponentManager::removeComponentObjects_NoLock(PlatformThreadInfo *,
                                                     const SimpleIDSet &compIDs,
                                                     std::vector<ComponentObjectRef> &objs)
{
    objs.reserve(compIDs.size());
    for (SimpleIdentity compID : compIDs)
    {
        const auto it = compObjsById.find(compID);
        if (it == compObjsById.end())
        {
            wkLogLevel(Warn,"Tried to delete object that doesn't exist: %d",compID);
            return;
        }

        const ComponentObjectRef &compObj = it->second;

        if (compObj->underConstruction)
        {
            wkLogLevel(Warn,"Deleting an object that's under construction");
        }

        if (!compObj->uuid.empty())
        {
            const auto range = compObjsByUUID.equal_range(compObj->uuid);
            for (auto i = range.first; i != range.second; )
            {
                if (i->second->getId() == compID)
                {
                    // "References and iterators to the erased elements are invalidated. Other references and iterators are not affected."
                    i = compObjsByUUID.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }

        objs.push_back(compObj);

        compObjsById.erase(it);
    }
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::removeComponentObjects(PlatformThreadInfo *threadInfo,
                                              const SimpleIDSet &compIDs,
                                              ChangeSet &changes,
                                              __unused bool disposeAfterRemoval)    // used by platform override
{
    if (compIDs.empty() || !scene)
        return;
    
    std::vector<ComponentObjectRef> compRefs;

    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
        removeComponentObjects_NoLock(threadInfo, compIDs, compRefs);
    }

    SimpleIDSet maskIDs;
    for (const ComponentObjectRef &compObj : compRefs)
    {
        // Get rid of the various layer objects
#if !MAPLY_MINIMAL
        if (!compObj->markerIDs.empty())
            markerManager->removeMarkers(compObj->markerIDs, changes);
        if (!compObj->labelIDs.empty())
            labelManager->removeLabels(threadInfo,compObj->labelIDs, changes);
        if (!compObj->vectorIDs.empty())
            vectorManager->removeVectors(compObj->vectorIDs, changes);
        if (!compObj->wideVectorIDs.empty())
            wideVectorManager->removeVectors(compObj->wideVectorIDs, changes);
#endif //!MAPLY_MINIMAL
        if (!compObj->shapeIDs.empty())
            shapeManager->removeShapes(compObj->shapeIDs, changes);
#if !MAPLY_MINIMAL
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
            if (const auto ftm = scene ? scene->getFontTextureManager() : nullptr)
            {
                // Giving the fonts 2s to stick around
                //       This avoids problems with texture being paged out.
                //       Without this we lose the textures before we're done with them
                const TimeInterval when = scene->getCurrentTime() + 2.0;
                for (SimpleIdentity dStrID : compObj->drawStringIDs)
                {
                    ftm->removeString(threadInfo, dStrID, changes, when);
                }
            }
        }
        for (const auto partSysID : compObj->partSysIDs)
        {
            partSysManager->removeParticleSystem(partSysID, changes);
        }
        if (!compObj->maskIDs.empty())
            maskIDs.insert(compObj->maskIDs.begin(),compObj->maskIDs.end());
#endif //!MAPLY_MINIMAL
    }

    releaseMaskIDs(maskIDs);
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::enableComponentObject(SimpleIdentity compID, bool enable, ChangeSet &changes, bool resolveReps)
{
    ComponentObjectRef compRef;

    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
    
        const auto it = compObjsById.find(compID);
        if (it == compObjsById.end())
        {
            wkLogLevel(Warn,"Tried to enable/disable object that doesn't exist");
            return;
        }

        compRef = it->second;

        if (compRef->underConstruction)
        {
            wkLogLevel(Warn,"Disable/enabled an object that's under construction");
        }
    }

    enableComponentObject(compRef, enable, changes, resolveReps);
}


// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::enableComponentObjects(const SimpleIDSet &compIDs,bool enable,ChangeSet &changes, bool resolveReps)
{
    std::vector<ComponentObjectRef> compRefs;
    compRefs.reserve(compIDs.size());

    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
    
        for (SimpleIdentity compID : compIDs)
        {
            const auto it = compObjsById.find(compID);
            if (it == compObjsById.end())
            {
                wkLogLevel(Warn,"Tried to enable/disable object that doesn't exist");
                return;
            }

            const ComponentObjectRef &compObj = it->second;

            if (compObj->underConstruction)
            {
                wkLogLevel(Warn,"Disable/enabled an object that's under construction");
            }

            compRefs.push_back(compObj);
        }
    }

    enableComponentObjects(compRefs, enable, changes, resolveReps);
}

// Determine the new state for "that" given a change to "this."
static bool ResolveRepresentationState(const ComponentObjectRef &thisObj, const ComponentObjectRef &thatObj)
{
    assert(thisObj->uuid == thatObj->uuid && !thisObj->uuid.empty());

    bool const enable = thisObj->enable;
    if (thisObj.get() == thatObj.get() || thatObj->representation == thisObj->representation)
    {
        // The references refer to the same object, or instances of the same representation.
        // For example, there may be two versions while transitioning between
        // levels, or the same object may appear in multiple tiles of a dataset.
        // Apply the same state to it.
        return enable;
    }
    else if (thisObj->representation.empty())
    {
        // If the default representation is being enabled, disable others
        // If it's being disabled, don't mess with the others.
        return !enable || thatObj->enable;
    }
    // If a non-default state is being enabled, disable others
    else if (enable)
    {
        return false;
    }
    // If a non-default state is being disabled, enable the default state
    else if (thatObj->representation.empty())
    {
        return true;
    }

    // No change
    return thatObj->enable;
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::enableComponentObject(const ComponentObjectRef &compObj, bool enable, ChangeSet &changes, bool resolveReps)
{
    // Note: Should lock just around this component object
    //       But I'm not sure I want one std::mutex per object
    compObj->enable = enable;

#if !MAPLY_MINIMAL
    if (!compObj->vectorIDs.empty())
        vectorManager->enableVectors(compObj->vectorIDs, enable, changes);
    if (!compObj->wideVectorIDs.empty())
        wideVectorManager->enableVectors(compObj->wideVectorIDs, enable, changes);
    if (!compObj->markerIDs.empty())
        markerManager->enableMarkers(compObj->markerIDs, enable, changes);
    if (!compObj->labelIDs.empty())
        labelManager->enableLabels(compObj->labelIDs, enable, changes);
#endif //!MAPLY_MINIMAL
    if (!compObj->shapeIDs.empty())
        shapeManager->enableShapes(compObj->shapeIDs, enable, changes);
#if !MAPLY_MINIMAL
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
#endif //!MAPLY_MINIMAL

    // Handle the other representations of the same thing?
    if (resolveReps && !compObj->uuid.empty())
    {
        // Consider all the objects we know about with this same uuid
        for (auto i = compObjsByUUID.equal_range(compObj->uuid); i.first != i.second; ++i.first)
        {
            // If the desired state is not the state it's in, toggle it
            const auto &otherObj = i.first->second;
            if (ResolveRepresentationState(compObj, otherObj) != otherObj->enable)
            {
                enableComponentObject(otherObj, !otherObj->enable, changes, false);
            }
        }
    }
}

static inline bool HasUUID(const ComponentObjectRef &obj)
{
    return !obj->uuid.empty();
}
static inline bool ByUUIDThenRep(const ComponentObjectRef &l, const ComponentObjectRef &r)
{
    return std::less<>()(l->uuid, r->uuid) ||       // less
           (!std::less<>()(r->uuid, l->uuid) &&        // equal
             std::less<>()(l->representation, r->representation));
}

// NOLINTNEXTLINE(google-default-arguments)
void ComponentManager::enableComponentObjects(const std::vector<ComponentObjectRef> &compRefs, bool enable,
                                              ChangeSet &changes, bool resolveReps)
{
    // It's probably worth an array scan here to avoid unnecessary heap allocations
    if (resolveReps && compRefs.size() > 1 &&
        std::any_of(compRefs.begin(), compRefs.end(), HasUUID))
    {
        // make a copy, sorted by uuid then by representation
        std::vector<ComponentObjectRef> sorted(compRefs.size());
        std::partial_sort_copy(compRefs.begin(), compRefs.end(), sorted.begin(), sorted.end(), ByUUIDThenRep);

        // Iterate the sorted copy by groups of uuids
        for (auto i = sorted.begin(); i != sorted.end(); )
        {
            const auto &obj = *i;
            const auto groupEnd = std::find_if(i, sorted.end(), [&](const auto &j) { return j->uuid != obj->uuid; });
            const auto next = obj->uuid.empty() ? groupEnd : std::next(i);

            // For items with no uuid, just call each of them normally.
            // For items with a uuid, enable or disable the first item in each group, which
            // will be the default representation (blank) if it's present.  The logic in the
            // single-item version of enableComponentObject will take care of the rest.
            for (; i != next; ++i)
            {
                enableComponentObject(*i, enable, changes, true);
            }

            i = groupEnd;
        }
        return;
    }

    // Don't resolve individual items unless we skipped the above because there's only one item.
    resolveReps = resolveReps && (compRefs.size() == 1);

    for (const auto &compObj : compRefs)
    {
        enableComponentObject(compObj, enable, changes, resolveReps);
    }
}

template <typename TIter>
void ComponentManager::setRepresentation(const std::string &repName,
                                         const std::string &fallback,
                                         TIter beg, TIter end,
                                         ChangeSet &changes)
{
    std::vector<ComponentObjectRef> enableObjs, disableObjs;

#ifdef LOG_REPRESENTATIONS
    std::stringstream idsStr, enStr, disStr;
    for (auto i = beg; i != end; ++i) idsStr << ((i==beg)?"":",") << i->c_str();
#endif

    std::unique_lock<std::mutex> guardLock(lock);

    for (; beg != end; ++beg)
    {
        const std::string &uuid = *beg;

        if (repName.empty())
        {
            // Don't store blank entries, remove them to return to default (un-set) state
            representations.erase(uuid);
        }
        else
        {
            if (representations.empty())
            {
                // Now that we know they're using the representation feature,
                // bypass the first few allocation cycles when adding items.
                representations.reserve(TypicalRepUUIDs);
            }

            const auto insertResult = representations.insert(std::make_pair(uuid, repName));
            if (!insertResult.second)
            {
                // key exists, update the value
                insertResult.first->second = repName;
            }
        }

        // Find all component items with matching UUIDs
        const auto range = compObjsByUUID.equal_range(uuid);

        // Put each of them in the enable or disable lists
        disableObjs.reserve(std::distance(range.first, range.second));
        for (auto i = range.first; i != range.second; ++i)
        {
            const auto &obj = i->second;
            ((obj->representation == repName) ? enableObjs : disableObjs).push_back(obj);
        }

        // If there are no matches, enable the fallback (usually blank) representation instead.
        if (enableObjs.empty() && !disableObjs.empty())
        {
            for (size_t i = 0; i < disableObjs.size(); ++i)
            {
                const auto &obj = disableObjs[i];
                if (obj->representation == fallback)
                {
                    // Move the item from the disable to enable list
                    enableObjs.push_back(obj);
                    disableObjs.erase(std::next(disableObjs.begin(), (int)i));
                }
            }
        }
    }

    guardLock.unlock();

#ifdef LOG_REPRESENTATIONS
    for (const auto &x : enableObjs) enStr << x->getId() << ",";
    for (const auto &x : disableObjs) disStr << x->getId() << ",";
    wkLogLevel(Verbose, "Set representation %s for %s : enable %s disable %s",
               repName.c_str(), idsStr.str().c_str(), enStr.str().c_str(), disStr.str().c_str());
#endif

    if (!enableObjs.empty()) enableComponentObjects(enableObjs, true, changes);
    if (!disableObjs.empty()) enableComponentObjects(disableObjs, false, changes);
}

void ComponentManager::setRepresentation(const std::string &repName, const std::string &fallback,
                                         const std::vector<std::string> &uuids, ChangeSet &changes)
{
    setRepresentation(repName, fallback, uuids.begin(), uuids.end(), changes);
}

void ComponentManager::setRepresentation(const std::string &repName, const std::string &fallback,
                                         const std::set<std::string> &uuids, ChangeSet &changes)
{
    setRepresentation(repName, fallback, uuids.begin(), uuids.end(), changes);
}

void ComponentManager::setRepresentation(const std::string &repName, const std::string &fallback,
                                         const std::unordered_set<std::string> &uuids, ChangeSet &changes)
{
    setRepresentation(repName, fallback, uuids.begin(), uuids.end(), changes);
}

void ComponentManager::setUniformBlock(const SimpleIDSet &compIDs,const RawDataRef &uniBlock,int bufferID,ChangeSet &changes)
{
    std::vector<ComponentObjectRef> compRefs;
    
    {
        // Lock around all component objects
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (SimpleIdentity compID : compIDs)
        {
            auto it = compObjsById.find(compID);
            if (it == compObjsById.end())
            {
                wkLogLevel(Warn,"Tried to set uniform block on object that doesn't exist");
                return;
            }
            compRefs.push_back(it->second);
        }
    }
    
    for (const auto &compObj : compRefs)
    {
        if (shapeManager && !compObj->shapeIDs.empty()) {
            shapeManager->setUniformBlock(compObj->shapeIDs,uniBlock,bufferID,changes);
        }
#if !MAPLY_MINIMAL
        if (partSysManager && !compObj->partSysIDs.empty()) {
            partSysManager->setUniformBlock(compObj->partSysIDs,uniBlock,bufferID,changes);
        }
        if (geomManager && !compObj->geomIDs.empty()) {
            geomManager->setUniformBlock(compObj->geomIDs,uniBlock,bufferID,changes);
        }
        // TODO: Fill this in for the other object types
#endif //!MAPLY_MINIMAL
    }
}

SimpleIdentity ComponentManager::retainMaskByName(const std::string &maskName)
{
    std::lock_guard<std::mutex> guardLock(maskLock);

    // Add an entry or reuse one
    MaskEntryRef entry;
    auto it = maskEntriesByName.find(maskName);
    if (it == maskEntriesByName.end()) {
        entry = std::make_shared<MaskEntry>();
        entry->name = maskName;
        entry->maskID = ++lastMaskID;
        entry->refCount = 0;
        maskEntriesByName[maskName] = entry;
        maskEntriesByID[entry->maskID] = entry;
    } else {
        entry = it->second;
    }
    entry->refCount++;
    
    return entry->maskID;
}

void ComponentManager::releaseMaskIDs(const SimpleIDSet &maskIDs)
{
    std::lock_guard<std::mutex> guardLock(maskLock);

    for (auto maskID: maskIDs) {
        // Reduce reference count
        auto it = maskEntriesByID.find(maskID);
        if (it != maskEntriesByID.end()) {
            auto entry = it->second;
            entry->refCount--;

            // Erase it when we're done
            if (entry->refCount <= 0) {
                maskEntriesByID.erase(it);
                auto it2 = maskEntriesByName.find(entry->name);
                if (it2 != maskEntriesByName.end())
                    maskEntriesByName.erase(it2);
            }
        }
    }
}

#if !MAPLY_MINIMAL

std::vector<std::pair<ComponentObjectRef,VectorObjectRef>> ComponentManager::findVectors(
        const Point2d &pt,double maxDist,const ViewStateRef &viewState,
        const Point2f &frameSize,int resultLimit)
{
    // not locked, we don't care if the size is off, we just want
    // to typically do the allocations outside the locked region.
    std::vector<ComponentObjectRef> compRefs;
    compRefs.reserve(compObjsById.size());

    // Copy out the vectors that might be candidates
    {
        std::lock_guard<std::mutex> guardLock(lock);
        for (const auto &kvp: compObjsById)
        {
            const auto &compObj = kvp.second;
            if (compObj->enable && compObj->isSelectable && !compObj->vecObjs.empty())
            {
                compRefs.push_back(compObj);
            }
        }
    }

    std::vector<std::pair<ComponentObjectRef,VectorObjectRef> > rets;
    rets.reserve((resultLimit > 0) ? resultLimit : compRefs.size());

    // Work through the vector objects
    for (const auto &compObj: compRefs)
    {
        const Point2d coord = pt - compObj->vectorOffset;

        for (const auto &vecObj: compObj->vecObjs)
        {
            if (vecObj->pointInside(pt) ||
                vecObj->pointNearLinear(coord, (float)maxDist, viewState, frameSize))
            {
                rets.emplace_back(compObj, vecObj);
            }
        }
        
        if (resultLimit > 0 && rets.size() >= resultLimit)
        {
            break;
        }
    }
    
    return rets;
}

#endif //!MAPLY_MINIMAL

}
