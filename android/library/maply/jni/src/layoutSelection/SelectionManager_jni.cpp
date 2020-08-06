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

#import "LayoutSelection_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "Components_jni.h"
#import "com_mousebird_maply_SelectionManager.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

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
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject pointObj)
{
	try
	{
		SelectionManagerClassInfo *classInfo = SelectionManagerClassInfo::getClassInfo();
		SelectionManager *selectionManager = classInfo->getObject(env,obj);
		ViewStateRefClassInfo *viewStateRefClassInfo = ViewStateRefClassInfo::getClassInfo();
		ViewStateRef *mapViewState = viewStateRefClassInfo->getObject(env,viewStateObj);
		Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
		Point2d *point = point2DclassInfo->getObject(env,pointObj);
		if (!selectionManager || !mapViewState || !point)
			return EmptyIdentity;

		return (jlong)selectionManager->pickObject(Point2f(point->x(),point->y()),10.0,*mapViewState);
	}
	catch (...)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectionManager::pickObject()");
	}
    
    return EmptyIdentity;
}

JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_SelectionManager_pickObjects
(JNIEnv *env, jobject selManageObj, jobject compManageObj, jobject viewStateObj, jobject pointObj)
{
    try
    {
        SelectionManager *selectionManager = SelectionManagerClassInfo::getClassInfo()->getObject(env,selManageObj);
        ComponentManager *compManager = ComponentManagerClassInfo::getClassInfo()->getObject(env,compManageObj);
		ViewStateRefClassInfo *viewStateRefClassInfo = ViewStateRefClassInfo::getClassInfo();
		ViewStateRef *mapViewState = viewStateRefClassInfo->getObject(env,viewStateObj);
        Point2dClassInfo *point2DclassInfo = Point2dClassInfo::getClassInfo();
        Point2d *point = point2DclassInfo->getObject(env,pointObj);
        if (!selectionManager || !compManager || !mapViewState || !point)
            return NULL;
        
        std::vector<SelectionManager::SelectedObject> selObjs;

        Point2f frameBufferSizeScaled = selectionManager->getSceneRenderer()->getFramebufferSizeScaled();
        Point2f frameBufferSize = selectionManager->getSceneRenderer()->getFramebufferSize();

        // This takes care of labels, markers, billboards, 3D objects and such.
        Point2f pt2f(point->x(),point->y());
        selectionManager->pickObjects(pt2f,10.0,*mapViewState,selObjs);

        // Need the point in geographic
        WhirlyGlobe::GlobeViewState *globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>((*mapViewState).get());
        Maply::MapViewState *maplyViewState = dynamic_cast<Maply::MapViewState *>((*mapViewState).get());
        Point3d dispPt;
        if (globeViewState) {
            globeViewState->pointOnSphereFromScreen(Point2f(point->x(),point->y()),globeViewState->fullMatrices[0],frameBufferSize,dispPt);
        } else {
            maplyViewState->pointOnPlaneFromScreen(Point2f(point->x(),point->y()),maplyViewState->fullMatrices[0],frameBufferSize,dispPt,false);
        }
        Point3d locPoint = (*mapViewState)->coordAdapter->displayToLocal(dispPt);

        // This one does vector features
        auto vecObjs = compManager->findVectors(Point2d(locPoint.x(),locPoint.y()),20.0,*mapViewState,frameBufferSizeScaled,true);
        for (auto vecObj : vecObjs) {
            SelectionManager::SelectedObject selObj;
            selObj.distIn3D = 0.0;
            selObj.isCluster = false;
            selObj.screenDist = 0.0;
            selObj.vecObj = vecObj.second;
            selObj.selectIDs.push_back(vecObj.second->getId());
            selObjs.push_back(selObj);
        }

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
    
    return NULL;
}
