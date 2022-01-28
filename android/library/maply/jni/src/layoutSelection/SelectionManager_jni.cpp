/*  SelectionManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
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
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_SelectionManager_pickObjects
  (JNIEnv *env, jobject selManageObj, jobject compManageObj,
   jobject viewStateObj, jobject pointObj, jdouble maxDist)
{
    try
    {
        const auto selectionManager = SelectionManagerClassInfo::get(env,selManageObj);
        const auto compManager = ComponentManagerClassInfo::get(env,compManageObj);
		const auto viewState = ViewStateRefClassInfo::get(env, viewStateObj);
        const auto point = Point2dClassInfo::get(env,pointObj);
        if (!selectionManager || !compManager || !viewState || !point)
        {
            return nullptr;
        }

        const auto renderer = (*selectionManager)->getSceneRenderer();
        const auto coordAdapter = (*viewState)->coordAdapter;
        const auto coordSystem = coordAdapter ? coordAdapter->getCoordSystem() : nullptr;
        if (!renderer || !coordSystem)
        {
            return nullptr;
        }

        // This takes care of labels, markers, billboards, 3D objects and such.
        std::vector<SelectionManager::SelectedObject> selObjs;
        selObjs.reserve(10);
        const Point2f pt2f = point->cast<float>();
        (*selectionManager)->pickObjects(pt2f, maxDist, *viewState, selObjs);

        const Point2f frameBufSize = renderer->getFramebufferSize();
        const Point2f frameBufSizeScaled = renderer->getFramebufferSizeScaled();

        // Need the point in geographic
        Point3d dispPt;
        if (auto *globeViewState = dynamic_cast<WhirlyGlobe::GlobeViewState *>(viewState->get()))
        {
            const auto &matrix = globeViewState->fullMatrices[0];
            if (!globeViewState->pointOnSphereFromScreen(pt2f, matrix, frameBufSize,dispPt))
            {
                return nullptr;
            }
        }
        else if (auto *mapViewState = dynamic_cast<Maply::MapViewState *>(viewState->get()))
        {
            const auto &matrix = mapViewState->fullMatrices[0];
            if (!mapViewState->pointOnPlaneFromScreen(pt2f, matrix, frameBufSize, dispPt, false))
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }

        const Point3d locPoint = coordAdapter->displayToLocal(dispPt);
        const Point2f geoPoint = coordSystem->localToGeographic(locPoint);
        const Point2d geoPoint2d = geoPoint.cast<double>();

        // This one does vector features
        auto vecObjs = (*compManager)->findVectors(geoPoint2d, maxDist, *viewState, frameBufSizeScaled);

        selObjs.reserve(selObjs.size() + vecObjs.size());
        for (const auto &vecObj : vecObjs)
        {
            selObjs.emplace_back(vecObj.second->getId(), 0.0, 0.0);
            selObjs.back().vecObj = vecObj.second;
        }

        if (selObjs.empty())
        {
            return nullptr;
        }

        jclass jc = SelectedObjectClassInfo::getClassInfo(env,"com/mousebird/maply/SelectedObject")->getClass();
        jobjectArray retArray = env->NewObjectArray(selObjs.size(), jc, nullptr);
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
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}
