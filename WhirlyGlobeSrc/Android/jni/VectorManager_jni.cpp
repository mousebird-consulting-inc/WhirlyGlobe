/*
 *  VectorManager_jni.cpp
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
#import "com_mousebird_maply_VectorManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

// Wrapper that tracks the scene as well
class VecManagerWrapper
{
public:
	VecManagerWrapper(VectorManager *vecManager,Scene *scene)
		: vecManager(vecManager), scene(scene)
	{

	}
	VectorManager *vecManager;
	Scene *scene;
};

typedef JavaClassInfo<VecManagerWrapper> VectorManagerWrapperClassInfo;
template<> VectorManagerWrapperClassInfo *VectorManagerWrapperClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	VectorManagerWrapperClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
		VectorManager *vecManager = dynamic_cast<VectorManager *>(scene->getManager(kWKVectorManager));
		VecManagerWrapper *wrap = new VecManagerWrapper(vecManager,scene);
		VectorManagerWrapperClassInfo::getClassInfo()->setHandle(env,obj,wrap);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		VectorManagerWrapperClassInfo *classInfo = VectorManagerWrapperClassInfo::getClassInfo();
		VecManagerWrapper *wrap = classInfo->getObject(env,obj);
		if (!wrap)
			return;
		classInfo->clearHandle(env,obj);
		delete wrap;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_VectorManager_addVectors
  (JNIEnv *env, jobject obj, jobject vecObjList, jobject vecInfoObj, jobject changeSetObj)
{
	try
	{
		VectorManagerWrapperClassInfo *classInfo = VectorManagerWrapperClassInfo::getClassInfo();
		VecManagerWrapper *wrap = classInfo->getObject(env,obj);
		VectorInfo *vecInfo = VectorInfoClassInfo::getClassInfo()->getObject(env,vecInfoObj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!wrap || !vecInfo || !changeSet)
			return EmptyIdentity;

        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "VectorInfo: (min,max) = (%f, %f), color = (%d,%d,%d,%d)",vecInfo->minVis,vecInfo->maxVis,(int)vecInfo->color.r,(int)vecInfo->color.g,(int)vecInfo->color.b,(int)vecInfo->color.a);

		// Get the iterator
		// Note: Look these up once
		jclass listClass = env->GetObjectClass(vecObjList);
		jclass iterClass = env->FindClass("java/util/Iterator");
		jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
		jobject liter = env->CallObjectMethod(vecObjList,literMethod);
		jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
		jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");

		ShapeSet shapes;
		VectorObjectClassInfo *vecObjClassInfo = VectorObjectClassInfo::getClassInfo();
		while (env->CallBooleanMethod(liter, hasNext))
		{
			jobject javaVecObj = env->CallObjectMethod(liter, next);
			VectorObject *vecObj = vecObjClassInfo->getObject(env,javaVecObj);
			shapes.insert(vecObj->shapes.begin(),vecObj->shapes.end());
			env->DeleteLocalRef(javaVecObj);
		}
		env->DeleteLocalRef(liter);

		SimpleIdentity vecId = wrap->vecManager->addVectors(&shapes,*vecInfo,*changeSet);

		return vecId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::addVectors()");
	}
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_removeVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		VectorManagerWrapperClassInfo *classInfo = VectorManagerWrapperClassInfo::getClassInfo();
		VecManagerWrapper *wrap = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!wrap || !changeSet)
			return;

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		wrap->vecManager->removeVectors(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::removeVectors()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorManager_enableVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		VectorManagerWrapperClassInfo *classInfo = VectorManagerWrapperClassInfo::getClassInfo();
		VecManagerWrapper *wrap = classInfo->getObject(env,obj);
		ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
		if (!wrap || !changeSet)
			return;

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		wrap->vecManager->enableVectors(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::enableVectors()");
	}
}

