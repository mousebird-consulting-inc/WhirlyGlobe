/*  SelectionManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "LayoutSelection_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "Components_jni.h"
#import "Selection_jni.h"
#import "com_mousebird_maply_SelectionManager.h"

using namespace WhirlyKit;
using namespace Maply;

//static const char *SceneHandleName = "nativeSceneHandle";

template<> SelectionManagerClassInfo *SelectionManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_nativeInit
  (JNIEnv *env, jclass cls)
{
	SelectionManagerClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
	try
	{
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
		const auto selectionManager = scene->getManager<SelectionManager>(kWKSelectionManager);
		SelectionManagerClassInfo::getClassInfo()->setHandle(env,obj,new SelectionManagerRef(selectionManager));
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectionManager::initialise()");
	}
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectionManager_dispose
  (JNIEnv *env, jobject obj)
{
	try
	{
        SelectionManagerClassInfo *classInfo = SelectionManagerClassInfo::getClassInfo();
        SelectionManagerRef *selectionManager = classInfo->getObject(env,obj);
        delete selectionManager;
        classInfo->clearHandle(env,obj);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectionManager::dispose()");
	}
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SelectionManager_pickObject
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject pointObj)
{
	try
	{
		SelectionManagerClassInfo *classInfo = SelectionManagerClassInfo::getClassInfo();
		SelectionManagerRef *selectionManager = classInfo->getObject(env,obj);
		ViewStateRefClassInfo *viewStateRefClassInfo = ViewStateRefClassInfo::getClassInfo();
		ViewStateRef *mapViewState = viewStateRefClassInfo->getObject(env,viewStateObj);
		Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
		Point2d *point = point2DclassInfo->getObject(env,pointObj);
		if (!selectionManager || !mapViewState || !point)
			return EmptyIdentity;

		return (jlong)(*selectionManager)->pickObject(Point2f(point->x(),point->y()),10.0,*mapViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectionManager::pickObject()");
	}
    
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_SelectionManager_pickObjects
(JNIEnv *env, jobject selManageObj, jobject compManageObj, jobject viewStateObj, jobject pointObj)
{
    try
    {
        SelectionManagerRef *selectionManager = SelectionManagerClassInfo::getClassInfo()->getObject(env,selManageObj);
        ComponentManager_AndroidRef *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,compManageObj);
		ViewStateRefClassInfo *viewStateRefClassInfo = ViewStateRefClassInfo::getClassInfo();
		ViewStateRef *mapViewState = viewStateRefClassInfo->getObject(env,viewStateObj);
        Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
        Point2d *point = point2DclassInfo->getObject(env,pointObj);
        if (!selectionManager || !compManager || !mapViewState || !point)
            return nullptr;
        
        const Point2f frameBufferSizeScaled = (*selectionManager)->getSceneRenderer()->getFramebufferSizeScaled();
        const Point2f frameBufferSize = (*selectionManager)->getSceneRenderer()->getFramebufferSize();

        // This takes care of labels, markers, billboards, 3D objects and such.
        const Point2f pt2f(point->x(),point->y());
        std::vector<SelectionManager::SelectedObject> selObjs;
        selObjs.reserve(10);
        (*selectionManager)->pickObjects(pt2f,10.0,*mapViewState,selObjs);

        // Need the point in geographic
        auto *globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(mapViewState->get());
        auto *maplyViewState = dynamic_cast<Maply::MapViewState *>(mapViewState->get());
        Point3d dispPt;
        if (globeViewState) {
            globeViewState->pointOnSphereFromScreen(Point2f(point->x(),point->y()),globeViewState->fullMatrices[0],frameBufferSize,dispPt);
        } else if (mapViewState) {
            maplyViewState->pointOnPlaneFromScreen(Point2f(point->x(),point->y()),maplyViewState->fullMatrices[0],frameBufferSize,dispPt,false);
        } else {
            return nullptr;
        }

        const Point3d locPoint = (*mapViewState)->coordAdapter->displayToLocal(dispPt);

        // This one does vector features
        auto vecObjs = (*compManager)->findVectors(Point2d(locPoint.x(),locPoint.y()),20.0,*mapViewState,frameBufferSizeScaled,true);

        selObjs.reserve(selObjs.size() + vecObjs.size());
        for (const auto &vecObj : vecObjs)
        {
            selObjs.emplace_back(vecObj.second->getId(), 0.0, 0.0);
            selObjs.back().vecObj = vecObj.second;
        }

        if (selObjs.empty())
            return nullptr;

        const jclass jc = SelectedObjectClassInfo::getClassInfo(env,"com/mousebird/maply/SelectedObject")->getClass();
        const jobjectArray retArray = env->NewObjectArray(selObjs.size(), jc, nullptr);
        int which = 0;
        for (auto &selObj : selObjs)
        {
            jobject newObj = MakeSelectedObject(env,std::move(selObj));
            env->SetObjectArrayElement(retArray,which,newObj);
            env->DeleteLocalRef(newObj);
            which++;
        }

        return retArray;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectionManager::pickObjects()");
    }
    
    return nullptr;
}
