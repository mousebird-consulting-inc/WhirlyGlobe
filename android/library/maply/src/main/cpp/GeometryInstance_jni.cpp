/*
 *  GeometryRawPoints_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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
#import "com_mousebird_maply_GeometryInstance.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_nativeInit
(JNIEnv *env, jclass cls)
{
    GeometryInstanceClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryInstance *geomInst = new GeometryInstance();
        GeometryInstanceClassInfo::getClassInfo()->setHandle(env,obj,geomInst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setCenter
(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        
        inst->center = Point3d(x,y,z);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setCenter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setEndCenter
(JNIEnv *env, jobject obj, jdouble x, jdouble y, jdouble z)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        
        inst->endCenter = Point3d(x,y,z);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setEndCenter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setDuration
(JNIEnv *env, jobject obj, jdouble duration)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        
        inst->duration = duration;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setDuration()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setMatrix
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        Eigen::Matrix4d *mat = Matrix4dClassInfo::getClassInfo()->getObject(env,matObj);
        if (!inst || !mat)
            return;

        inst->mat = *mat;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setMatrix()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        
        inst->color = RGBAColor(r,g,b,a);
        inst->colorOverride = true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeometryInstance_setSelectable
(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        GeometryInstanceClassInfo *classInfo = GeometryInstanceClassInfo::getClassInfo();
        GeometryInstance *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        
        inst->selectable = selectable;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeometryInstance::setSelectable()");
    }
}
