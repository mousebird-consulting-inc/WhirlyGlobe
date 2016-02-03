/*
 *  WhirlyKitGLSetupInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
 *  Copyright 2011-2015 mousebird consulting
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
#import "com_mousebird_maply_WhirlyKitGLSetupInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;



JNIEXPORT void JNICALL Java_com_mousebird_maply_WhirlyKitGLSetupInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    WhirlyKitGLSetupInfoClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WhirlyKitGLSetupInfo_initialise
(JNIEnv *env, jobject obj)
{
    try {
        WhirlyKitGLSetupInfoClassInfo *classInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo();
        WhirlyKitGLSetupInfo *inst = new WhirlyKitGLSetupInfo();
        classInfo->setHandle(env, obj, inst);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WhirlyKitGLSetupInfo::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WhirlyKitGLSetupInfo_dispose
(JNIEnv *env, jobject obj)
{
    try {
        WhirlyKitGLSetupInfoClassInfo *classInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo();
        WhirlyKitGLSetupInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WhirlyKitGLSetupInfo::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_WhirlyKitGLSetupInfo_setMinZres
(JNIEnv *env, jobject obj, jfloat minZres)
{
    try {
        WhirlyKitGLSetupInfoClassInfo *classInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo();
        WhirlyKitGLSetupInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->minZres = minZres;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WhirlyKitGLSetupInfo::setMinZres()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_WhirlyKitGLSetupInfo_getMinZres
(JNIEnv *env, jobject obj)
{
    try {
        WhirlyKitGLSetupInfoClassInfo *classInfo = WhirlyKitGLSetupInfoClassInfo::getClassInfo();
        WhirlyKitGLSetupInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return inst->minZres;
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in WhirlyKitGLSetupInfo::getMinZres()");
    }
}
