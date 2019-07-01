/*
 *  ShapeRectangle_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2017 mousebird consulting
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

#import "Shapes_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ShapeRectangle.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> RectangleClassInfo *RectangleClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_nativeInit
(JNIEnv *env, jclass cls)
{
    RectangleClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        RectangleClassInfo *classInfo = RectangleClassInfo::getClassInfo();
        Rectangle *inst = new Rectangle();
        classInfo->setHandle(env, obj, inst);
        
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeRectangle::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        RectangleClassInfo *classInfo = RectangleClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Rectangle *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeRectangle::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_setPoints
(JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        RectangleClassInfo *classInfo = RectangleClassInfo::getClassInfo();
        Rectangle *inst = classInfo->getObject(env, obj);
        Point3dClassInfo *ptClassInfo = Point3dClassInfo::getClassInfo();
        Point3d *ll = ptClassInfo->getObject(env,llObj);
        Point3d *ur = ptClassInfo->getObject(env,urObj);
        if (!inst || !ll || !ur)
            return;
     
        inst->setLL(*ll);
        inst->setUR(*ur);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeRectangle::setPoints()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_addTextureID
(JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        RectangleClassInfo *classInfo = RectangleClassInfo::getClassInfo();
        Rectangle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->texIDs.push_back(texID);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeRectangle::addTextureID()");
    }
}
