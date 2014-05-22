/*
 * MapScene_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */

#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_MapScene.h"
#import "WhirlyGlobe.h"
#import "FontTextureManagerAndroid.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapScene_nativeInit
  (JNIEnv *env, jclass cls)
{
	MapSceneClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapScene_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj, jobject charRendererObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
		Maply::MapScene *scene = new Maply::MapScene(coordAdapter);
		scene->setFontTextureManager(new FontTextureManagerAndroid(env,scene,charRendererObj));
		MapSceneClassInfo::getClassInfo()->setHandle(env,obj,scene);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapScene_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapSceneClassInfo *classInfo = MapSceneClassInfo::getClassInfo();
		Maply::MapScene *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_MapScene_addChanges
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		MapSceneClassInfo *classInfo = MapSceneClassInfo::getClassInfo();
		Maply::MapScene *scene = classInfo->getObject(env,obj);
		ChangeSet *changes = ChangeSetClassInfo::getClassInfo()->getObject(env,changesObj);
		scene->addChangeRequests(*changes);
		changes->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::addChanges()");
	}
}
