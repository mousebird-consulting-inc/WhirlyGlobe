/*
 *  SelectedObject_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/17/16.
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
#import "com_mousebird_maply_SelectedObject.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_nativeInit
(JNIEnv *env, jclass cls)
{
    SelectedObjectClassInfo::getClassInfo(env,cls);
}

JNIEXPORT jobject JNICALL MakeSelectedObject(JNIEnv *env,const WhirlyKit::SelectionManager::SelectedObject &selObj)
{
    SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo(env,"com/mousebird/maply/SelectedObject");
    SelectionManager::SelectedObject *newObj = new SelectionManager::SelectedObject();
    *newObj = selObj;
    return classInfo->makeWrapperObject(env,newObj);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectionManager::SelectedObject *selectedObj = new SelectionManager::SelectedObject();
        SelectedObjectClassInfo::getClassInfo()->setHandle(env,obj,selectedObj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SelectedObject_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
            if (!selectedObj)
                return;
            delete selectedObj;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_SelectedObject_getSelectID
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
        if (!selectedObj)
            return 0;
        
        return selectedObj->selectID;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::getSelectID()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SelectedObject_getDistIn3d
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
        if (!selectedObj)
            return 0.0;
        
        return selectedObj->distIn3D;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::getDistIn3d()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_SelectedObject_getScreenDist
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
        if (!selectedObj)
            return 0.0;
        
        return selectedObj->screenDist;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::getScreenDist()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_SelectedObject_isPartOfCluster
(JNIEnv *env, jobject obj)
{
    try
    {
        SelectedObjectClassInfo *classInfo = SelectedObjectClassInfo::getClassInfo();
        SelectionManager::SelectedObject *selectedObj = classInfo->getObject(env,obj);
        if (!selectedObj)
            return false;
        
        return selectedObj->isCluster;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SelectedObject::isPartOfCluster()");
    }
}
