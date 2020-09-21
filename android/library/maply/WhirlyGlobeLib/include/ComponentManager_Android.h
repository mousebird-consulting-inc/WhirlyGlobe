/*
 *  ComponentManager_Android.h
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

#import "ComponentManager.h"

namespace WhirlyKit
{

/**  Android version of the Component Manager.
  **/
class ComponentManager_Android : public ComponentManager
{
public:
    /// We'll keep a global reference to the Java-side object
    ComponentManager_Android();
    virtual ~ComponentManager_Android();

    // Called by the JNI initialize to set up our link back to the Java side
    void setupJNI(JNIEnv *env,jobject compManagerObj);
    void clearJNI(JNIEnv *env);

    /// This version of remove calls back to notify the Java side
    virtual void removeComponentObjects(PlatformThreadInfo *threadInfo,const SimpleIDSet &compIDs,ChangeSet &changes) override;

protected:
    virtual ComponentObjectRef makeComponentObject() override;
    jobject compManagerObj;
    jmethodID objectsRemovedMethod;
};

}
