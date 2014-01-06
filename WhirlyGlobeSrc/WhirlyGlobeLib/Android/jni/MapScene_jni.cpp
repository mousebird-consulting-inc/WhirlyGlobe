/*
 * MapScene_jni.cpp
 *
 *  Created on: Dec 19, 2013
 *      Author: sjg
 */



#import <jni.h>
#import "handle.h"
#import "com_mousebirdconsulting_maply_MapScene.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapScene_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = getHandle<CoordSystemDisplayAdapter>(env,coordAdapterObj);
		Maply::MapScene *inst = new Maply::MapScene(coordAdapter);
		setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapScene_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		Maply::MapScene *inst = getHandle<Maply::MapScene>(env,obj);
		if (!inst)
			return;
		delete inst;
		inst = NULL;

		setHandle(env,obj,inst);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::dispose()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebirdconsulting_maply_MapScene_addChanges
  (JNIEnv *env, jobject obj, jobject changesObj)
{
	try
	{
		Maply::MapScene *scene = getHandle<Maply::MapScene>(env,obj);
		ChangeSet *changes = getHandle<ChangeSet>(env,changesObj);
		scene->addChangeRequests(*changes);
		changes->clear();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapScene::addChanges()");
	}
}
