/*
 *  Moon_jni.cpp
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
#import "com_mousebird_maply_Moon.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Moon_nativeInit
(JNIEnv *env, jclass cls)
{
    MoonClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Moon_initialise
(JNIEnv *env, jobject obj, jint year, jint month, jint day, jint hour, jint minutes, jint second)
{
    try
    {
        MoonClassInfo *classInfo = MoonClassInfo::getClassInfo();
        Moon *inst = new Moon(year, month, day, hour, minutes, second);
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Moon::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Moon_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        MoonClassInfo *classInfo = MoonClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Moon *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Moon::dispose()");
    }
}

JNIEXPORT jdoubleArray JNICALL Java_com_mousebird_maply_Moon_getIlluminatedFractionAndPhaseNative
(JNIEnv *env, jobject obj)
{
    try
    {
        MoonClassInfo *classInfo = MoonClassInfo::getClassInfo();
        Moon *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        double *position = new double[2];
        position[0] = inst->illuminatedFraction;
        position[1] = inst->phase;
        jdoubleArray result;
        result = env->NewDoubleArray(2);
        env->SetDoubleArrayRegion(result, 0, 2, position);
        delete[] position;
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Moon::getIlluminatedFractionAndPhaseNative()");
    }
    
    return NULL;
}

JNIEXPORT jdoubleArray JNICALL Java_com_mousebird_maply_Moon_getPositionOfMoon
(JNIEnv *env, jobject obj)
{
    try
    {
        MoonClassInfo *classInfo = MoonClassInfo::getClassInfo();
        Moon *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        double * position = new double[2];
        position[0] = inst->moonLon;
        position[1] = inst->moonLat;
        jdoubleArray result;
        result = env->NewDoubleArray(2);
        env->SetDoubleArrayRegion(result, 0, 2, position);
        free(position);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Moon::getPositionOfMoon()");
    }
    
    return NULL;
}
