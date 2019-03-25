/*
 *  ObjectLoaderReturn_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/20/19.
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

#import "QuadLoading_jni.h"
#import "Components_jni.h"
#import "com_mousebird_maply_SamplingParams.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ObjectLoaderReturn_addComponentObjects
  (JNIEnv *env, jobject obj, jobjectArray compObjs)
{
	try
	{
		QuadLoaderReturn *loadReturn = LoaderReturnClassInfo::getClassInfo()->getObject(env,obj);
		if (!loadReturn)
		    return;

        // Work through the component object array
		ComponentObjectRefClassInfo *compObjClassInfo = ComponentObjectRefClassInfo::getClassInfo();
		int count = env->GetArrayLength(compObjs);
		if (count == 0)
			return;
		for (int ii=0;ii<count;ii++)
		{
			jobject compObjObj = env->GetObjectArrayElement(compObjs,ii);
            ComponentObjectRef *compObj = compObjClassInfo->getObject(env,compObjObj);
            loadReturn->compObjs.push_back(*compObj);
            env->DeleteLocalRef(compObjObj);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ObjectLoaderReturn::addComponentObjects()");
	}
}
