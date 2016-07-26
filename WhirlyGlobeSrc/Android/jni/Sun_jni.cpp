/*
 *  Sun_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/16.
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
#import <AA+.h>
#import "Maply_jni.h"
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_Sun.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_nativeInit
(JNIEnv *env, jclass cls)
{
    SunClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_initialise
(JNIEnv *env, jobject obj)
{
    try {
        Sun *sun = new Sun();
        SunClassInfo::getClassInfo()->setHandle(env,obj,sun);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Sun *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Sun_getDirection
(JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;
        
        Point3d pt = inst->getDirection();
                
        return MakePoint3d(env, pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_setTime
(JNIEnv *env, jobject obj, jdouble theTime, jdouble year, jdouble month, jdouble day, jdouble hour, jdouble minute, jdouble second)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->time = theTime;

        inst->setTime(year,month,day,hour,minute,second);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_Sun_asPosition
(JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;

        float position[2] = {(float) inst->sunLon, (float) inst->sunLat};
        jfloatArray result;
        result = env->NewFloatArray(2);
        env->SetFloatArrayRegion(result, 0, 2, position);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::asPosition()");
    }
}
