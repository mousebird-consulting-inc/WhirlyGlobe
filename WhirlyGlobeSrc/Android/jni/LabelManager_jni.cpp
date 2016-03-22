/*
 *  LabelManager_jni.cpp
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

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_LabelManager.h"
#import "WhirlyGlobe.h"
#import "SingleLabelAndroid.h"
#import "LabelInfoAndroid.h"
#import "FontTextureManagerAndroid.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

typedef JavaClassInfo<LabelManager> LabelManagerClassInfo;
template<> LabelManagerClassInfo *LabelManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	LabelManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		LabelManager *labelManager = dynamic_cast<LabelManager *>(scene->getManager(kWKLabelManager));
		LabelManagerClassInfo::getClassInfo()->setHandle(env,obj,labelManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		LabelManagerClassInfo::getClassInfo()->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_LabelManager_addLabels
  (JNIEnv *env, jobject obj, jobject labelObjList, jobject labelInfoObj, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		LabelInfoAndroid *labelInfo = (LabelInfoAndroid *)LabelInfoClassInfo::getClassInfo()->getObject(env,labelInfoObj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !labelInfo || !changeSet)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in LabelManager::addLabels()");
			return EmptyIdentity;
		}
        
        // Need to tell the font texture manager what the current environment is
        //  so it can delete things if it needs to
        FontTextureManagerAndroid *fontTexManager = (FontTextureManagerAndroid *)labelManager->getScene()->getFontTextureManager();
        fontTexManager->setEnv(env);

		std::vector<SingleLabel *> labels;
		JavaListInfo *listClassInfo = JavaListInfo::getClassInfo(env);
		jobject iterObj = listClassInfo->getIter(env,labelObjList);

		LabelClassInfo *labelClassInfo = LabelClassInfo::getClassInfo();
		ShapeSet shapes;
		// We need this in the depths of the engine
		labelInfo->env = env;
		labelInfo->labelInfoObj = labelInfoObj;
		while (listClassInfo->hasNext(env,labelObjList,iterObj))
		{
			jobject javaLabelObj = listClassInfo->getNext(env,labelObjList,iterObj);
			SingleLabelAndroid *label = labelClassInfo->getObject(env,javaLabelObj);
			labels.push_back(label);
			env->DeleteLocalRef(javaLabelObj);
		}
		env->DeleteLocalRef(iterObj);

		// Resolve a missing program
		if (labelInfo->programID == EmptyIdentity)
	        {
                  // Note: Doesn't handle motion
                  labelInfo->programID = labelManager->getScene()->getProgramIDBySceneName(kToolkitDefaultScreenSpaceProgram);
                }
		SimpleIdentity labelId = labelManager->addLabels(labels,*labelInfo,*changeSet);

		labelInfo->env = NULL;
		labelInfo->labelInfoObj = NULL;

		return labelId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::addLabels()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_removeLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;
        
        // Need to tell the font texture manager what the current environment is
        //  so it can delete things if it needs to
        FontTextureManagerAndroid *fontTexManager = (FontTextureManagerAndroid *)labelManager->getScene()->getFontTextureManager();
        fontTexManager->setEnv(env);

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		labelManager->removeLabels(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::removeLabels()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_enableLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManager *labelManager = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;

		int idCount = env->GetArrayLength(idArrayObj);
		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);

		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		labelManager->enableLabels(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LabelManager::enableLabels()");
	}

}
