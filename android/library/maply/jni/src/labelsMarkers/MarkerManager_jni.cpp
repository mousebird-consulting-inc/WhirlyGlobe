/*
 *  MarkerManager_jni.cpp
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

#import "LabelsAndMarkers_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_MarkerManager.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

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

static std::mutex disposeMutex;

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
  (JNIEnv *env, jobject obj, jobjectArray markerArray, jobject markerInfoObj, jobject changeSetObj)
{
	try
	{
		MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
		MarkerManager *markerManager = classInfo->getObject(env,obj);
		MarkerInfoRef *markerInfo = MarkerInfoClassInfo::getClassInfo()->getObject(env,markerInfoObj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !markerInfo || !changeSet)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in MarkerManager::addMarkers()");
			return EmptyIdentity;
		}

		std::vector<Marker *> markers;
		JavaObjectArrayHelper markerHelp(env,markerArray);
		bool hasMultiTex = false;
		MarkerClassInfo *markerClassInfo = MarkerClassInfo::getClassInfo();
		while (jobject markerObj = markerHelp.getNextObject()) {
			Marker *marker = markerClassInfo->getObject(env,markerObj);
			if (marker->texIDs.size() > 1)
				hasMultiTex = true;

			markers.push_back(marker);
		}

		(*markerInfo)->screenObject = false;
		// Resolve the program ID
		if ((*markerInfo)->programID == EmptyIdentity)
        {
            Program *prog = NULL;
            if (hasMultiTex)
                prog = markerManager->getScene()->findProgramByName(MaplyDefaultMarkerShader);
            else
                prog = markerManager->getScene()->findProgramByName(MaplyDefaultTriangleShader);
            if (prog)
				(*markerInfo)->programID = prog->getId();
        }

		SimpleIdentity markerId = markerManager->addMarkers(markers,*(*markerInfo),*(changeSet->get()));

		return markerId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::addMarkers()");
	}
    
    return EmptyIdentity;
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_MarkerManager_addScreenMarkers
(JNIEnv *env, jobject obj, jobjectArray markerArray, jobject markerInfoObj, jobject changeSetObj)
{
    try
    {
        MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
        MarkerManager *markerManager = classInfo->getObject(env,obj);
		MarkerInfoRef *markerInfo = MarkerInfoClassInfo::getClassInfo()->getObject(env,markerInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!markerManager || !markerInfo || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in MarkerManager::addScreenMarkers()");
            return EmptyIdentity;
        }
        
        std::vector<Marker *> markers;
        JavaObjectArrayHelper markerHelper(env,markerArray);
		bool isMoving = false;
		MarkerClassInfo *markerClassInfo = MarkerClassInfo::getClassInfo();
        while (jobject markerObj = markerHelper.getNextObject()) {
			Marker *marker = markerClassInfo->getObject(env,markerObj);
			if (marker->startTime != marker->endTime)
				isMoving = true;

			markers.push_back(marker);
        }

		(*markerInfo)->screenObject = true;
        // Resolve the program ID
        if ((*markerInfo)->programID == EmptyIdentity)
        {
            Program *prog = NULL;
            if (isMoving)
                prog = markerManager->getScene()->findProgramByName(MaplyScreenSpaceDefaultMotionShader);
            else
                prog = markerManager->getScene()->findProgramByName(MaplyScreenSpaceDefaultShader);
            if (prog)
				(*markerInfo)->programID = prog->getId();
        }

        SimpleIdentity markerId = markerManager->addMarkers(markers,*(*markerInfo),*(changeSet->get()));
        
        return markerId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::addMarkers()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MarkerManager_removeMarkers
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		MarkerManagerClassInfo *classInfo = MarkerManagerClassInfo::getClassInfo();
		MarkerManager *markerManager = classInfo->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

		markerManager->removeMarkers(idSet,*(changeSet->get()));
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
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!markerManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

//		if (enable)
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Enabling marker: %d",(int)ids[0]);
//		else
//			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Disabling marker: %d",(int)ids[0]);

		markerManager->enableMarkers(idSet,enable,*(changeSet->get()));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MarkerManager::enableMarkers()");
	}
}
