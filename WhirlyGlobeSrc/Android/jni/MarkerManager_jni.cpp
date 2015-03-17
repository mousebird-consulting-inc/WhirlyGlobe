/*
 *  MarkerManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import "com_mousebird_maply_MarkerManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

typedef JavaClassInfo<MarkerManager> MarkerManagerClassInfo;
template<> MarkerManagerClassInfo *MarkerManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	MarkerManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		MarkerManager *markerManager = dynamic_cast<MarkerManager *>(scene->getManager(kWKMarkerManager));
		MarkerManagerClassInfo::getClassInfo()->setHandle(env,obj,markerManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MarkerManagerClassInfo::getClassInfo()->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_MarkerManager_addMarkers
  (JNIEnv *env, jobject obj, jobject markerObjList, jobject markerInfoObj, jobject changeSetObj)
{
	try
	{
		MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
		MarkerManager *markerManager = classInfo->getObject(env,obj);
		MarkerInfo *markerInfo = MarkerInfoClassInfo::getClassInfo()->getObject(env,markerInfoObj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !markerInfo || !changeSet)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in MarkerManager::addMarkers()");
			return EmptyIdentity;
		}

		std::vector<Marker *> markers;
		JavaListInfo *listClassInfo = JavaListInfo::getClassInfo(env);
		jobject iterObj = listClassInfo->getIter(env,markerObjList);

		MarkerClassInfo *markerClassInfo = MarkerClassInfo::getClassInfo();
		while (listClassInfo->hasNext(env,markerObjList,iterObj))
		{
			jobject javaMarkerObj = listClassInfo->getNext(env,markerObjList,iterObj);
			Marker *marker = markerClassInfo->getObject(env,javaMarkerObj);

			// Note: Porting
			marker->offset.x() = 0.0;  marker->offset.y() = 0.0;

			markers.push_back(marker);
			env->DeleteLocalRef(javaMarkerObj);
		}
		env->DeleteLocalRef(iterObj);

		// Note: Porting
		// Note: Shouldn't have to set this
    	markerInfo->markerId = Identifiable::genId();

		SimpleIdentity markerId = markerManager->addMarkers(markers,*markerInfo,*changeSet);

		return markerId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::addMarkers()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_removeMarkers
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
		MarkerManager *markerManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !changeSet)
			return;

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		markerManager->removeMarkers(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::removeMarkers()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_enableMarkers
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
		MarkerManager *markerManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !changeSet)
			return;

		int idCount = env->GetArrayLength(idArrayObj);
		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);

//		if (enable)
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Enabling marker: %d",(int)ids[0]);
//		else
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Disabling marker: %d",(int)ids[0]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		markerManager->enableMarkers(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::enableMarkers()");
	}
}
