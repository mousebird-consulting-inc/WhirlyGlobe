/*
 *  View_jni.cpp
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_View.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_View_nativeInit
  (JNIEnv *env, jclass cls)
{
	ViewClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_View_runViewUpdates
  (JNIEnv *env, jobject obj)
{
	try
	{
		MapViewClassInfo *classInfo = MapViewClassInfo::getClassInfo();
		Maply::MapView *view = classInfo->getObject(env,obj);
		if (!view)
			return;
		view->runViewUpdates();
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::runViewUpdates()");
	}
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_View_calcModelViewMatrix
  (JNIEnv *env, jobject obj)
{
	try
	{
		ViewClassInfo *classInfo = ViewClassInfo::getClassInfo();
		WhirlyKit::View *view = classInfo->getObject(env,obj);
		if (!view)
			return NULL;

		Matrix4d mat = view->calcModelMatrix();
		return MakeMatrix4d(env,mat);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapView::calcModelViewMatrix()");
	}
}
