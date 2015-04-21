/*
 *  GlobeScene_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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
#import "com_mousebird_maply_GlobeScene.h"
#import "WhirlyGlobe.h"
#import "FontTextureManagerAndroid.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeScene_nativeInit
  (JNIEnv *env, jclass cls)
{
	GlobeSceneClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeScene_initialise
  (JNIEnv *env, jobject obj, jobject coordAdapterObj, jobject charRendererObj, jint cullTreeDepth)
{
	try
	{
		CoordSystemDisplayAdapter *coordAdapter = CoordSystemDisplayAdapterInfo::getClassInfo()->getObject(env,coordAdapterObj);
		WhirlyGlobe::GlobeScene *scene = new WhirlyGlobe::GlobeScene(coordAdapter,cullTreeDepth);
		scene->setFontTextureManager(new FontTextureManagerAndroid(env,scene,charRendererObj));
		GlobeSceneClassInfo::getClassInfo()->setHandle(env,obj,scene);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeScene::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeScene_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeSceneClassInfo *classInfo = GlobeSceneClassInfo::getClassInfo();
		WhirlyGlobe::GlobeScene *inst = classInfo->getObject(env,obj);
		if (!inst)
			return;
		delete inst;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeScene::dispose()");
	}
}
