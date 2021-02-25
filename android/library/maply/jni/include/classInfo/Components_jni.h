/*
 *  Astronomy_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/19/19.
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
#import "WhirlyGlobe_Android.h"

typedef JavaClassInfo<WhirlyKit::BaseInfo> BaseInfoClassInfo;
typedef JavaClassInfo<WhirlyKit::SingleVertexAttribute> SingleVertexAttributeClassInfo;
typedef JavaClassInfo<WhirlyKit::ComponentObjectRef> ComponentObjectRefClassInfo;
typedef JavaClassInfo<WhirlyKit::ComponentManager_Android> ComponentManagerClassInfo;

// Build a Java-side object to wrap the given ComponentObject
JNIEXPORT jobject JNICALL MakeComponentObjectWrapper(JNIEnv *env,ComponentObjectRefClassInfo *classInfo,WhirlyKit::ComponentObjectRef compObj);
