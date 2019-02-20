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
    
ComponentObject::ComponentObject(SimpleIdentity theID)
: Identifiable(theID),
vectorOffset(0.0,0.0), isSelectable(false),
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
    layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    markerManager = (MarkerManager *)scene->getManager(kWKMarkerManager);
    labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    vectorManager = (VectorManager *)scene->getManager(kWKVectorManager);
    wideVectorManager = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    shapeManager = (ShapeManager *)scene->getManager(kWKShapeManager);
    // Note: Turned off chunk manager
    //    SphericalChunkManager *chunkManager = (SphericalChunkManager *)scene->getManager(kWKSphericalChunkManager);
    loftManager = (LoftManager *)scene->getManager(kWKLoftedPolyManager);
    billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    geomManager = (GeometryManager *)scene->getManager(kWKGeometryManager);
    fontTexManager = scene->getFontTextureManager();
    partSysManager = (ParticleSystemManager *)scene->getManager(kWKParticleSystemManager);
}

void ComponentManager::addComponentObject(ComponentObjectRef compObj)
{
    std::lock_guard<std::mutex> guardLock(lock);
    
    compObj->underConstruction = false;
    compObjs.insert(compObj);
}

bool ComponentManager::hasComponentObject(SimpleIdentity compID)
{
    std::lock_guard<std::mutex> guardLock(lock);

    auto it = compObjs.find(ComponentObjectRef(new ComponentObject(compID)));
    return it != compObjs.end();
}

void ComponentManager::removeComponentObject(SimpleIdentity compID,ChangeSet &changes)
{
    ComponentObjectRef compObj;
    
    // Lock around all component objects
    {
        std::lock_guard<std::mutex> guardLock(lock);

        auto it = compObjs.find(ComponentObjectRef(new ComponentObject(compID)));
        if (it == compObjs.end())
        {
            wkLogLevel(Warn,"Tried to delete object that doesn't exist");
            return;
        }
        compObj = *it;
        
        if (compObj->underConstruction) {
            wkLogLevel(Warn,"Deleting an object that's under construction");
        }
        compObj = *it;
        compObjs.erase(it);
    }
    
    // Get rid of the various layer objects
    if (compObj->markerIDs.empty())
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
    // Note: Turned off chunk manager
    //                if (chunkManager && !userObj.chunkIDs.empty())
    //                    chunkManager->removeChunks(userObj.chunkIDs, changes);
    if (!compObj->billIDs.empty())
        billManager->removeBillboards(compObj->billIDs, changes);
    if (!compObj->geomIDs.empty())
        geomManager->removeGeometry(compObj->geomIDs, changes);
    if (!compObj->drawStringIDs.empty())
    {
        // Giving the fonts 2s to stick around
        //       This avoids problems with texture being paged out.
        //       Without this we lose the textures before we're done with them
        TimeInterval when = TimeGetCurrent() + 2.0;
        for (SimpleIdentity dStrID : compObj->drawStringIDs)
            fontTexManager->removeString(dStrID, changes, when);
    }
    if (!compObj->partSysIDs.empty())
    {
        for (SimpleIdentity partSysID : compObj->partSysIDs)
            partSysManager->removeParticleSystem(partSysID, changes);
    }        
}

void ComponentManager::enableComponentObject(SimpleIdentity compID,bool enable,ChangeSet &changes)
{
    ComponentObjectRef compObj;
    
    // Lock around all component objects
    std::lock_guard<std::mutex> guardLock(lock);
        
    auto it = compObjs.find(ComponentObjectRef(new ComponentObject(compID)));
    if (it == compObjs.end())
    {
        wkLogLevel(Warn,"Tried to enable/disable object that doesn't exist");
        return;
    }
    compObj = *it;
    
    if (compObj->underConstruction) {
        wkLogLevel(Warn,"Disable/enabled an object that's under construction");
    }
    compObj = *it;
    
    compObj->enable = enable;

    // Note: Should lock just around this component object
    //       But I'm not sure I want one std::mutex per object
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
// Note: Disabled chunk manager
//            if (chunkManager && !compObj->contents->chunkIDs.empty())
//            {
//                for (SimpleIDSet::iterator it = compObj->contents->chunkIDs.begin();
//                     it != compObj->contents->chunkIDs.end(); ++it)
//                    chunkManager->enableChunk(*it, enable, changes);
//            }
}

}
