/*
 *  GlobeViewState_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/15.
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
#import "com_mousebird_maply_GlobeViewState.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;
using namespace WhirlyGlobe;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_nativeInit
  (JNIEnv *env, jclass cls)
{
	GlobeViewStateClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_initialise
  (JNIEnv *env, jobject obj, jobject viewObj, jobject rendererObj)
{
	try
	{
		GlobeView *globeView = GlobeViewClassInfo::getClassInfo()->getObject(env,viewObj);
		SceneRendererES *renderer = (SceneRendererES *)MaplySceneRendererInfo::getClassInfo()->getObject(env,rendererObj);
		if (!globeView || !renderer)
			return;

//		if (dynamic_cast<WhirlyGlobe::GlobeView *>(mapView))
//		{
//			viewState = new WhirlyGlobe::GlobeViewState((WhirlyGlobe::GlobeView *)view,renderer);
//		} else if (dynamic_cast<Maply::MapView *>(mapView))
		GlobeViewState *globeViewState = new WhirlyGlobe::GlobeViewState(globeView,renderer);

		if (globeViewState)
			GlobeViewStateClassInfo::getClassInfo()->setHandle(env,obj,globeViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewStateClassInfo *classInfo = GlobeViewStateClassInfo::getClassInfo();
		GlobeViewState *globeViewState = classInfo->getObject(env,obj);
		if (!globeViewState)
			return;
		delete globeViewState;

		classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeViewState::dispose()");
	}
}
