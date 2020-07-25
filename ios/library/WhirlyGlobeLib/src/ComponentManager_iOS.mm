/*
 *  ComponentManager_iOS.mm
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

#import <Foundation/Foundation.h>
#import "ComponentManager_iOS.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
ComponentObject_iOS::ComponentObject_iOS()
{
}

// The scene wants a component manager early in the process
// This gives it an iOS specific one
ComponentManager *MakeComponentManager()
{
    return new ComponentManager_iOS();
}
    
ComponentManager_iOS::ComponentManager_iOS()
{
}

ComponentManager_iOS::~ComponentManager_iOS()
{
}
    
void ComponentManager_iOS::addSelectObject(SimpleIdentity selectID,NSObject *obj)
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    selectObjectSet.insert(SelectObject(selectID,obj));
}

NSObject *ComponentManager_iOS::getSelectObject(SimpleIdentity selID)
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
    if (it != selectObjectSet.end())
        return it->obj;
    return nil;
}
    
ComponentObjectRef ComponentManager_iOS::makeComponentObject()
{
    return ComponentObject_iOSRef(new ComponentObject_iOS());
}
    
void ComponentManager_iOS::removeSelectObjects(SimpleIDSet selIDs)
{
    std::lock_guard<std::mutex> guardLock(selectLock);

    for (auto selID : selIDs) {
        SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
        if (it != selectObjectSet.end())
            selectObjectSet.erase(it);
        else
            wkLogLevel(Warn,"Tried to delete non-existent selection ID");
    }
}
    
void ComponentManager_iOS::removeComponentObject(PlatformThreadInfo *threadInfo,SimpleIdentity compID,ChangeSet &changes)
{
    ComponentObjectRef compObj;
    
    // Lock around all component objects
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        auto it = compObjs.find(compID);
        compObj = it->second;
    }

    // Now clean up the selection stuff
    if (!compObj->selectIDs.empty())
        removeSelectObjects(compObj->selectIDs);
    
    ComponentManager::removeComponentObject(threadInfo,compID, changes);
}
    
void ComponentManager_iOS::clear()
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    selectObjectSet.clear();
}
    
void ComponentManager_iOS::dumpStats()
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    wkLogLevel(Debug, "Component Objects: %d",selectObjectSet.size());
}
    
}
