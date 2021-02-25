/*
 *  ShapeInfo_jni.cpp
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

#import "Shapes_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ShapeInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ShapeInfoClassInfo *ShapeInfoClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeInfoClassInfo::getClassInfo(env, cls);

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        ShapeInfoRef *inst = new ShapeInfoRef(new ShapeInfo());
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            ShapeInfoRef *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        ShapeInfoRef *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        (*inst)->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setLineWidth
(JNIEnv *env, jobject obj, jfloat lineWidth)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        ShapeInfoRef *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        (*inst)->lineWidth = lineWidth;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setLineWidth()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setInsideOut
(JNIEnv *env, jobject obj, jboolean insideOut)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        ShapeInfoRef *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        (*inst)->insideOut = insideOut;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setInsideOut()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setCenter
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        ShapeInfoRef *inst = classInfo->getObject(env, obj);
        Point3d *center = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !center)
            return;
        (*inst)->hasCenter = true;
        (*inst)->center = *center;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setCenter()");
    }
}
