/*
 *  QuadTrackerPointReturn_jni.cpp
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
#import "com_mousebird_maply_QuadTrackerPointReturn.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_nativeInit
(JNIEnv *env, jclass cls)
{
    QuadTrackerPointReturnClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = new QuadTrackerPointReturn();
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setScreenU
(JNIEnv *env, jobject obj, jdouble screenU)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setScreenU(screenU);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setScreenU()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getScreenU
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getScreenU();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getScreenU()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setScreenV
(JNIEnv *env, jobject obj, jdouble screenV)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setScreenV(screenV);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setScreenV()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getScreenV
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getScreenV();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getScreenV()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setMaplyTileID
(JNIEnv *env, jobject obj, jint x, jint y, jint level)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        MaplyTileID tileID;
        tileID.x  = x;
        tileID.y  = y;
        tileID.level = level;
        inst->setMaplyTileID(tileID);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setMaplyTileID()");
    }
}

JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getMaplyTileID
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        MaplyTileID tileID = inst->getMaplyTileID();
        int data[3];
        data[0] = tileID.x;
        data[1] = tileID.y;
        data[2] = tileID.level;
        jintArray newArray = env->NewIntArray(3);
        env->SetIntArrayRegion(newArray, 0, 3, data);
        return newArray;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getMaplyTileID()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setPadding
(JNIEnv *env, jobject obj, jint padding)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setPadding(padding);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setPadding()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getPadding
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getPadding();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getPadding()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setLocX
(JNIEnv *env, jobject obj, jdouble locX)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setLocX(locX);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setLocX()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getLocX
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getLocX();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getLocX()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setLocY
(JNIEnv *env, jobject obj, jdouble locY)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setLocY(locY);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setLocY()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getLocY
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getLocY();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getLocY()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setTileU
(JNIEnv *env, jobject obj, jdouble tileU)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setTileU(tileU);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setTileU()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getTileU
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getTileU();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getTileU()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_setTileV
(JNIEnv *env, jobject obj, jdouble tileV)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setTileV(tileV);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::setTileV()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadTrackerPointReturn_getTileV
(JNIEnv *env, jobject obj)
{
    try
    {
        QuadTrackerPointReturnClassInfo *classInfo = QuadTrackerPointReturnClassInfo::getClassInfo();
        QuadTrackerPointReturn *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getTileV();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadTrackerPointReturn::getTileV()");
    }
}

