/*
 *  ComponentManager_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/15/19.
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
 *
 */

#import <Foundation/Foundation.h>
#import "ComponentManager_iOS.h"
#import "WhirlyKitLog.h"
#import "NSString+Stuff.h"
#import "Dictionary_NSDictionary.h"

namespace WhirlyKit
{

ComponentObject_iOS::ComponentObject_iOS()
{
}

ComponentObject_iOS::ComponentObject_iOS(bool enable, bool isSelectable, const NSDictionary *_Nullable desc) :
    ComponentObject(enable, isSelectable, iosDictionary(desc))
{
}

// The scene wants a component manager early in the process
// This gives it an iOS specific one
ComponentManagerRef MakeComponentManager()
{
    return std::make_shared<ComponentManager_iOS>();
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
    
ComponentObjectRef ComponentManager_iOS::makeComponentObject(const Dictionary *desc)
{
    NSDictionary *nsDesc = desc ? [NSMutableDictionary fromDictionaryCPointer:desc] : nil;
    return std::make_shared<ComponentObject_iOS>(/*enabled=*/false, /*isSelectable=*/false, nsDesc);
}
    
void ComponentManager_iOS::removeSelectObjects(SimpleIDSet selIDs)
{
    if (selIDs.empty())
    {
        return;
    }

    std::lock_guard<std::mutex> guardLock(selectLock);

    for (auto selID : selIDs) {
        SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
        if (it != selectObjectSet.end())
            selectObjectSet.erase(it);
        else
            wkLogLevel(Warn,"Tried to delete non-existent selection ID");
    }
}
    
void ComponentManager_iOS::removeComponentObjects(PlatformThreadInfo *threadInfo,const SimpleIDSet &compIDs,ChangeSet &changes)
{
    // Lock around all component objects
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        SimpleIDSet selectIDs;
        
        for (auto compID: compIDs)
        {
            const auto it = compObjsById.find(compID);
            if (it != compObjsById.end() && it->second)
            {
                selectIDs.insert(it->second->selectIDs.begin(),it->second->selectIDs.end());
            }
        }
        
        removeSelectObjects(selectIDs);
    }

    ComponentManager::removeComponentObjects(threadInfo,compIDs, changes);
}

void ComponentManager_iOS::clear()
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    selectObjectSet.clear();
}
    
void ComponentManager_iOS::dumpStats()
{
    std::lock_guard<std::mutex> guardLock(selectLock);
    wkLogLevel(Debug, "Component Objects: %d",compObjsById.size());
    wkLogLevel(Debug, "Selectable Objects: %d",selectObjectSet.size());
    wkLogLevel(Debug, "Objects w/ UUID: %d",compObjsByUUID.size());
    wkLogLevel(Debug, "Representations: %d",representations.size());
}

}
