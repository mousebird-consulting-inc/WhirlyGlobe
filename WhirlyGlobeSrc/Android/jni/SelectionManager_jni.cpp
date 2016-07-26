/*
 *  SelectionManager_jni.cpp
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
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_SelectionManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

typedef JavaClassInfo<SelectionManager> SelectionManagerClassInfo;
template<> SelectionManagerClassInfo *SelectionManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	SelectionManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
		SelectionManager *selectionManager = dynamic_cast<SelectionManager *>(scene->getManager(kWKSelectionManager));
		SelectionManagerClassInfo::getClassInfo()->setHandle(env,obj,selectionManager);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectionManager::initialise()");
	}
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
		SelectionManagerClassInfo::getClassInfo()->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectionManager::dispose()");
	}
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SelectionManager_pickObject
  (JNIEnv *env, jobject obj, jobject viewObj, jobject pointObj)
{
	try
	{
		SelectionManagerClassInfo *classInfo = SelectionManagerClassInfo::getClassInfo();
		SelectionManager *selectionManager = classInfo->getObject(env,obj);
		ViewClassInfo *viewClassInfo = ViewClassInfo::getClassInfo();
		View *mapView = viewClassInfo->getObject(env,viewObj);
		Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
		Point2d *point = point2DclassInfo->getObject(env,pointObj);
		if (!selectionManager || !mapView || !point)
			return EmptyIdentity;

		return (jlong)selectionManager->pickObject(Point2f(point->x(),point->y()),10.0,mapView);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectionManager::pickObject()");
	}
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_SelectionManager_pickObjects
(JNIEnv *env, jobject obj, jobject viewObj, jobject pointObj)
{
    try
    {
        SelectionManagerClassInfo *classInfo = SelectionManagerClassInfo::getClassInfo();
        SelectionManager *selectionManager = classInfo->getObject(env,obj);
        ViewClassInfo *viewClassInfo = ViewClassInfo::getClassInfo();
        View *mapView = viewClassInfo->getObject(env,viewObj);
        Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
        Point2d *point = point2DclassInfo->getObject(env,pointObj);
        if (!selectionManager || !mapView || !point)
            return NULL;
        
        std::vector<SelectionManager::SelectedObject> selObjs;
        selectionManager->pickObjects(Point2f(point->x(),point->y()),10.0,mapView,selObjs);

        if (selObjs.empty())
            return NULL;

        jobjectArray retArray = env->NewObjectArray(selObjs.size(), SelectedObjectClassInfo::getClassInfo(env,"com/mousebird/maply/SelectedObject")->getClass(), NULL);
        int which = 0;
        for (auto &selObj : selObjs)
        {
            jobject newObj = MakeSelectedObject(env,selObj);
            env->SetObjectArrayElement(retArray,which,newObj);
            env->DeleteLocalRef( newObj);
            which++;
        }
        
        return retArray;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectionManager::pickObjects()");
    }
}
