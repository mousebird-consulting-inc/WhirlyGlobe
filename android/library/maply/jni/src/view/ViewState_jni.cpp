/*
 *  ViewState_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2016 mousebird consulting
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

#import "View_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ViewState.h"

using namespace WhirlyKit;

template<> ViewStateRefClassInfo *ViewStateRefClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ViewState_nativeInit
  (JNIEnv *env, jclass cls)
{
	ViewStateRefClassInfo::getClassInfo(env,cls);
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ViewState_dispose
		(JNIEnv *env, jobject obj)
{
	try
	{
		ViewStateRefClassInfo *classInfo = ViewStateRefClassInfo::getClassInfo();
		{
			std::lock_guard<std::mutex> lock(disposeMutex);
			ViewStateRef *viewState = classInfo->getObject(env,obj);
			if (!viewState)
				return;
			delete viewState;

			classInfo->clearHandle(env,obj);
		}
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ViewState::dispose()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ViewState_isEqual
  (JNIEnv *env, jobject obj, jobject otherObj)
{
	try
	{
		ViewStateRefClassInfo *classInfo = ViewStateRefClassInfo::getClassInfo();
		ViewStateRef *viewState = classInfo->getObject(env,obj);
		ViewStateRef *otherViewState = classInfo->getObject(env,otherObj);
		if (!viewState || !otherViewState)
			return false;

		return (*viewState)->isSameAs((*otherViewState).get());
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::isEqual()");
	}
    
    return false;
}
