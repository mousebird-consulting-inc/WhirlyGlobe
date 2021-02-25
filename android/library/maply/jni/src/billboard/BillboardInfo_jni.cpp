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
#import "Billboard_jni.h"
#import "com_mousebird_maply_BillboardInfo.h"
#import "WhirlyGlobe_Android.h"

using namespace WhirlyKit;

template<> BillboardInfoClassInfo *BillboardInfoClassInfo::classInfoObj = NULL;

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
        BillboardInfoRef *inst= new BillboardInfoRef(new BillboardInfo());
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            BillboardInfoRef *inst= classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::dispose()");
    }

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfoRef *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        (*inst)->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardInfo_setOrientNative
(JNIEnv *env, jobject obj, jint orient)
{
    try
    {
        BillboardInfoClassInfo *classInfo = BillboardInfoClassInfo::getClassInfo();
        BillboardInfoRef *inst= classInfo->getObject(env, obj);
        if (!inst)
            return;
        (*inst)->orient = (BillboardInfo::Orient)orient;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardInfo::setOrientNative()");
    }
}

