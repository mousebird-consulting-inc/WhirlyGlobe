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

#import "View_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_GlobeViewState.h"

using namespace WhirlyKit;
using namespace Maply;
using namespace WhirlyGlobe;

template<> GlobeViewStateRefClassInfo *GlobeViewStateRefClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_nativeInit
  (JNIEnv *env, jclass cls)
{
	GlobeViewStateRefClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_initialise
  (JNIEnv *env, jobject obj, jobject viewObj, jobject rendererObj)
{
	try
	{
		GlobeView *globeView = GlobeViewClassInfo::getClassInfo()->getObject(env,viewObj);
		SceneRendererES2_Android *renderer = (SceneRendererES2_Android *)SceneRendererInfo::getClassInfo()->getObject(env,rendererObj);
		if (!globeView || !renderer)
			return;

		GlobeViewStateRef *globeViewState = new GlobeViewStateRef(new WhirlyGlobe::GlobeViewState(globeView,renderer));

		if (globeViewState)
			GlobeViewStateRefClassInfo::getClassInfo()->setHandle(env,obj,globeViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VectorInfo::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GlobeViewState_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		GlobeViewStateRefClassInfo *classInfo = GlobeViewStateRefClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            GlobeViewStateRef *globeViewState = classInfo->getObject(env,obj);
            if (!globeViewState)
                return;
            delete globeViewState;

            classInfo->clearHandle(env,obj);
        }
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GlobeViewState::dispose()");
	}
}
