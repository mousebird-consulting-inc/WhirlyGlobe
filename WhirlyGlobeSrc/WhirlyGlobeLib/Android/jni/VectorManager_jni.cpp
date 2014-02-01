/*
 * VectorManager_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */




#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_VectorManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
		MapScene *scene = getHandle<MapScene>(env,sceneObj);
		setHandleNamed(env,obj,scene,SceneHandleName);
		VectorManager *vecManager = dynamic_cast<VectorManager *>(scene->getManager(kWKVectorManager));
		setHandle(env,obj,vecManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebirdconsulting_maply_VectorManager_addVectors
  (JNIEnv *env, jobject obj, jobjectArray vecObjArray, jobject vecInfoObj, jobject changeSetObj)
{
	try
	{
		VectorManager *vecManager = getHandle<VectorManager>(env,obj);
		VectorInfo *vecInfo = getHandle<VectorInfo>(env,vecInfoObj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);

		ShapeSet shapes;
		int vecObjCount = env->GetArrayLength(vecObjArray);
		for (int ii=0;ii<vecObjCount;ii++)
		{
			jobject javaVecObj = env->GetObjectArrayElement(vecObjArray, ii);
			VectorObject *vecObj = getHandle<VectorObject>(env,javaVecObj);
			shapes.insert(vecObj->shapes.begin(),vecObj->shapes.end());
			env->DeleteLocalRef(javaVecObj);
		}

		SimpleIdentity vecId = vecManager->addVectors(&shapes,*vecInfo,*changeSet);

		return vecId;
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::addVectors()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_removeVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
	try
	{
		VectorManager *vecManager = getHandle<VectorManager>(env,obj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		vecManager->removeVectors(idSet,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::removeVectors()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_VectorManager_enableVectors
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
	try
	{
		VectorManager *vecManager = getHandle<VectorManager>(env,obj);
		ChangeSet *changeSet = getHandle<ChangeSet>(env,changeSetObj);
		Scene *scene = getHandleNamed<Scene>(env,obj,SceneHandleName);

		long long *ids = env->GetLongArrayElements(idArrayObj, NULL);
		int idCount = env->GetArrayLength(idArrayObj);
		if (idCount == 0)
			return;

		SimpleIDSet idSet;
		for (int ii=0;ii<idCount;ii++)
			idSet.insert(ids[ii]);
		env->ReleaseLongArrayElements(idArrayObj,ids, 0);

		vecManager->enableVectors(idSet,enable,*changeSet);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorManager::enableVectors()");
	}
}

