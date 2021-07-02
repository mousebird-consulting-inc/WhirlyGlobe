/*  LabelManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
 */

#import "LabelsAndMarkers_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_LabelManager.h"

using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<LabelManagerRef> LabelManagerClassInfo;
template<> LabelManagerClassInfo *LabelManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_nativeInit(JNIEnv *env, jclass cls)
{
	LabelManagerClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_initialise(JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        if (auto scene = SceneClassInfo::get(env, sceneObj))
        {
            auto labelManager = scene->getManager<LabelManager>(kWKLabelManager);
            LabelManagerClassInfo::getClassInfo()->setHandle(env, obj,new LabelManagerRef(labelManager));
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LabelManager::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_dispose(JNIEnv *env, jobject obj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManagerRef *labelManager = classInfo->getObject(env,obj);
		delete labelManager;
		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LabelManager::dispose()");
	}
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_LabelManager_addLabels
  (JNIEnv *env, jobject obj, jobjectArray labelArray,
   jobject labelInfoObj, jobject changeSetObj)
{
	try
	{
		LabelClassInfo *labelClassInfo = LabelClassInfo::getClassInfo();
		LabelManagerRef *labelManager = LabelManagerClassInfo::get(env,obj);
        LabelInfoAndroidRef *labelInfo = LabelInfoClassInfo::get(env,labelInfoObj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::get(env,changeSetObj);
		if (!labelManager || !labelInfo || !changeSet)
		{
			__android_log_print(ANDROID_LOG_WARN, "Maply", "One of the inputs was null in LabelManager::addLabels()");
			return EmptyIdentity;
		}

		// We need this in the depths of the engine
        (*labelInfo)->labelInfoObj = labelInfoObj;

		// Collect the labels
		std::vector<SingleLabel *> labels;
		JavaObjectArrayHelper labelHelp(env,labelArray);
		labels.reserve(labelHelp.numObjects());
		bool isMoving = false;
		while (jobject labelObj = labelHelp.getNextObject())
		{
			SingleLabelAndroid *label = labelClassInfo->getObject(env,labelObj);
			if (label->startTime != label->endTime)
			{
				isMoving = true;
			}
			labels.push_back(label);
		}

		// Resolve a missing program
		if ((*labelInfo)->programID == EmptyIdentity)
        {
			if (auto prog = (ProgramGLES *)(*labelManager)->getScene()->findProgramByName(
			        isMoving ? MaplyScreenSpaceDefaultMotionShader : MaplyScreenSpaceDefaultShader))
            {
			    (*labelInfo)->programID = prog->getId();
            }
        }
		PlatformInfo_Android platformInfo(env);
		SimpleIdentity labelId = (*labelManager)->addLabels(&platformInfo,labels,**labelInfo,**changeSet);

        (*labelInfo)->labelInfoObj = nullptr;

		return labelId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LabelManager::addLabels()");
	}
    
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_removeLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManagerRef *labelManager = classInfo->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

		PlatformInfo_Android platformInfo(env);
		(*labelManager)->removeLabels(&platformInfo,idSet,**changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LabelManager::removeLabels()");
	}
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LabelManager_enableLabels
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		LabelManagerClassInfo *classInfo = LabelManagerClassInfo::getClassInfo();
		LabelManagerRef *labelManager = classInfo->getObject(env,obj);
		ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!labelManager || !changeSet)
			return;

        SimpleIDSet idSet;
        ConvertLongArrayToSet(env,idArrayObj,idSet);

		(*labelManager)->enableLabels(idSet,enable,**changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LabelManager::enableLabels()");
	}
}
