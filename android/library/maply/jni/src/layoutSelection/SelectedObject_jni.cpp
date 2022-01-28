/*  SelectedObject_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/17/16.
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
#import "com_mousebird_maply_SelectedObject.h"

using namespace WhirlyKit;
using namespace Maply;

template<> SelectedObjectClassInfo *SelectedObjectClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_nativeInit
    (JNIEnv *env, jclass cls)
{
    SelectedObjectClassInfo::getClassInfo(env,cls);
}

static jobject JNICALL MakeSelectedObject(JNIEnv *env)
{
    auto *classInfo = SelectedObjectClassInfo::getClassInfo(env,"com/mousebird/maply/SelectedObject");
    return classInfo->makeWrapperObject(env,nullptr);
}
JNIEXPORT jobject JNICALL MakeSelectedObject(JNIEnv *env,const WhirlyKit::SelectionManager::SelectedObject &selObj)
{
    jobject newObj = MakeSelectedObject(env);
    if (const auto inst = SelectedObjectClassInfo::get(env,newObj))
    {
        *inst = selObj;
    }
    return newObj;
}
JNIEXPORT jobject JNICALL MakeSelectedObject(JNIEnv *env, WhirlyKit::SelectionManager::SelectedObject &&selObj)
{
    jobject newObj = MakeSelectedObject(env);
    if (const auto inst = SelectedObjectClassInfo::get(env,newObj))
    {
        *inst = std::move(selObj);
    }
    return newObj;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_initialise
    (JNIEnv *env, jobject obj)
{
    try
    {
        auto *selectedObj = new SelectionManager::SelectedObject();
        SelectedObjectClassInfo::getClassInfo()->setHandle(env,obj,selectedObj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_dispose
    (JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
        delete selectedObj;
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::dispose()");
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SelectedObject_getSelectID
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            if (!selectedObj->selectIDs.empty())
            {
                return selectedObj->selectIDs[0];
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getSelectID()");
    }
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SelectedObject_getDistIn3d
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return selectedObj->distIn3D;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getDistIn3d()");
    }
    return 0.0;
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SelectedObject_getScreenDist
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return selectedObj->screenDist;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getScreenDist()");
    }
    return 0.0;
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SelectedObject_isPartOfCluster
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return selectedObj->isCluster;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::isPartOfCluster()");
    }
    return false;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_SelectedObject_getClusterGroup
    (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return selectedObj->clusterGroup;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getClusterGroup()");
    }
    return EmptyIdentity;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SelectedObject_getClusterID
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return selectedObj->clusterId;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getClusterID()");
    }
    return EmptyIdentity;
}


extern "C"
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_SelectedObject_getClusterCenter
        (JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto selectedObj = SelectedObjectClassInfo::get(env,obj))
        {
            return MakePoint2d(env, Point2d(selectedObj->center.x(), selectedObj->center.y()));
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in SelectedObject::getClusterCenter()");
    }
    return nullptr;
}
