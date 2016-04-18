/*
 *  BillboardInfo_jni.cpp
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
#import "com_mousebird_maply_BillboardInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    BillboardInfoClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= new BillboardInfo();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::dispose()");
    }

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setBillboardID
(JNIEnv *env, jobject obj, jlong billboardID)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->billboardId = billboardID;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setBillboardID()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_BillboardInfo_getBillboardID
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->billboardId;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::getBillboardID()");
    }
    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_addBillboard
(JNIEnv *env, jobject obj, jobject billboardObj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        Billboard *newBillboard = BillboardClassInfo::getClassInfo()->getObject(env, billboardObj);
        if (!inst || !newBillboard)
            return;
        inst->billboards.push_back(*newBillboard);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::addBillboard()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setColor()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_BillboardInfo_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        float * primaryColors = new float[4];
        inst->color.asUnitFloats(primaryColors);
        jfloatArray result;
        result = env->NewFloatArray(4);
        env->SetFloatArrayRegion(result, 0, 4, primaryColors);
        free(primaryColors);
        return result;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::getColor()");
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setZBufferRead
(JNIEnv *env, jobject obj, jboolean zBufferRead)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->zBufferRead = zBufferRead;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setZBufferRead()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_BillboardInfo_getZBufferRead
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->zBufferRead;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::getZBufferRead()");
    }
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setZBufferWrite
(JNIEnv *env, jobject obj, jboolean zBufferWrite)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->zBufferWrite = zBufferWrite;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setZBufferWrite()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_BillboardInfo_getZBufferWrite
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfo *inst= classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->zBufferWrite;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::getZBufferWrite()");
    }
    return false;
}
