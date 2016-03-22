/*
 *  MapScene_jni.cpp
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
