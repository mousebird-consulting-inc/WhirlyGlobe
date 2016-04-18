/*
 *  Billboard_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "com_mousebird_maply_Billboard.h"
#import "WhirlyGlobe.h"
#import "Maply_utils_jni.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_nativeInit
(JNIEnv *env, jclass cls)
{
    BillboardClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = new Billboard();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_setCenter
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        Point3d *newCenter = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !newCenter)
            return;
        inst->center = *newCenter;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::setCenter()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Billboard_getCenter
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return MakePoint3d(env,inst->center);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::getCenter()");
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_addPoly
(JNIEnv *env, jobject obj, jobject polyObj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        SingleBillboardPoly *poly = SingleBillboardPolyClassInfo::getClassInfo()->getObject(env, polyObj);
        if (!inst || !poly)
            return;
        inst->polys.push_back(*poly);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::addPoly()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_setSize
(JNIEnv *env, jobject obj, jobject sizeObj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        Point2d *newSize = Point2dClassInfo::getClassInfo()->getObject(env, sizeObj);
        if (!inst || !newSize)
            return;
        inst->size = *newSize;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::setSize()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Billboard_getSize
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return MakePoint2d(env,inst->size);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::getSize()");
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Billboard_setSelectable
(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->isSelectable = selectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::setSelectable()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Billboard_getSelectable
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardClassInfo *classInfo = BillboardClassInfo::getClassInfo();
        Billboard *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->isSelectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Billboard::getSelectable()");
    }
}
