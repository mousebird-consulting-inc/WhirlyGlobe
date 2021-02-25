/*
 *  ShapeCircle_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2019 mousebird consulting
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
#import "com_mousebird_maply_ShapeCircle.h"

using namespace WhirlyKit;

template<> CircleClassInfo *CircleClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_nativeInit
(JNIEnv *env, jclass cls)
{
    CircleClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        Circle *inst = new Circle();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Circle *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        Circle *inst = classInfo->getObject(env, obj);
        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *loc = ptClassInfo->getObject(env,ptObj);
        if (!inst || !loc)
            return;

        inst->loc = GeoCoord(loc->x(),loc->y());
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::setLoc()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        Circle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->height = height;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::setHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setRadius
  (JNIEnv *env, jobject obj, jdouble radius)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        Circle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->radius = radius;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::setRadius()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setSample
  (JNIEnv *env, jobject obj, jint sampleX)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        Circle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->sampleX = sampleX;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCircle::setSample()");
    }
}
