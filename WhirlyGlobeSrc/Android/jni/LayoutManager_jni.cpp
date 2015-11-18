/*
 *  LayoutManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2015 mousebird consulting
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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_LayoutManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<LayoutManager> LayoutManagerClassInfo;
template<> LayoutManagerClassInfo *LayoutManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	LayoutManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_initialise
  (JNIEnv *env, jobject obj,jobject sceneObj)
{
	try
	{
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		LayoutManager *layoutManager = dynamic_cast<LayoutManager *>(scene->getManager(kWKLayoutManager));
		LayoutManagerClassInfo::getClassInfo()->setHandle(env,obj,layoutManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LayoutManagerClassInfo::getClassInfo()->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setMaxDisplayObjects
  (JNIEnv *env, jobject obj, jint maxObjs)
{
	try
	{
		LayoutManagerClassInfo *classInfo = LayoutManagerClassInfo::getClassInfo();
		LayoutManager *layoutManager = classInfo->getObject(env,obj);

		if (!layoutManager)
			return;

		layoutManager->setMaxDisplayObjects(maxObjs);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::setMaxDisplayObjects()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_updateLayout
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeSetObj)
{
	try
	{
		LayoutManagerClassInfo *classInfo = LayoutManagerClassInfo::getClassInfo();
		LayoutManager *layoutManager = classInfo->getObject(env,obj);
		ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);

		if (!layoutManager || !viewState || !changeSet)
			return;

		layoutManager->updateLayout(viewState,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::updateLayout()");
	}
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_hasChanges
  (JNIEnv *env, jobject obj)
{
	try
	{
		LayoutManagerClassInfo *classInfo = LayoutManagerClassInfo::getClassInfo();
		LayoutManager *layoutManager = classInfo->getObject(env,obj);

		if (!layoutManager)
			return false;

		return layoutManager->hasChanges();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::hasChanges()");
	}
}
