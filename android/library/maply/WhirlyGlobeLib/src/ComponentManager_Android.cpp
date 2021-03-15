/*
 *  ComponentManager_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/4/19.
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

#import "Maply_jni.h"
#import "ComponentManager_Android.h"

namespace WhirlyKit
{
    
// The scene wants a component manager early in the process
// This gives it an Android specific one
ComponentManagerRef MakeComponentManager()
{
    return std::make_shared<ComponentManager_Android>();
}

ComponentManager_Android::ComponentManager_Android() :
    compManagerObj(nullptr),
    objectsRemovedMethod(nullptr)
{
}

void ComponentManager_Android::setupJNI(JNIEnv *env,jobject inCompManagerObj)
{
    compManagerObj = env->NewGlobalRef(inCompManagerObj);
    jclass compManagerClass =  env->GetObjectClass(compManagerObj);
    objectsRemovedMethod = env->GetMethodID(compManagerClass, "objectsRemoved", "([J)V");
}

void ComponentManager_Android::clearJNI(JNIEnv *env)
{
    if (compManagerObj) {
        env->DeleteGlobalRef(compManagerObj);
        compManagerObj = nullptr;
    }
    objectsRemovedMethod = nullptr;
}

ComponentManager_Android::~ComponentManager_Android()
{
    if (compManagerObj) {
        wkLogLevel(Warn, "ComponentManager_Android not cleaned up");
    }
}

void ComponentManager_Android::removeComponentObjects(PlatformThreadInfo *inThreadInfo,const SimpleIDSet &compIDs,ChangeSet &changes)
{
    if (compIDs.empty())
        return;

    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;

    ComponentManager::removeComponentObjects(threadInfo,compIDs,changes);

    std::vector<SimpleIdentity> idsVec;
    for (auto id: compIDs)
        idsVec.push_back(id);
    jlongArray idsArray = BuildLongArray(threadInfo->env,idsVec);

    // Tell the Java side about the IDs we just deleted
    threadInfo->env->CallVoidMethod(compManagerObj,objectsRemovedMethod,idsArray);
    threadInfo->env->DeleteLocalRef(idsArray);
}

ComponentObjectRef ComponentManager_Android::makeComponentObject(const Dictionary *desc)
{
    return std::make_shared<ComponentObject>();
}

}
