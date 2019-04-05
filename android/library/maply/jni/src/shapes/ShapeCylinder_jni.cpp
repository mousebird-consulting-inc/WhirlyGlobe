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
#import "com_mousebird_maply_ShapeCylinder.h"

using namespace WhirlyKit;

template<> CylinderClassInfo *CylinderClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_nativeInit
(JNIEnv *env, jclass cls)
{
    CylinderClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = new Cylinder();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Cylinder *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setBaseCenter
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = classInfo->getObject(env, obj);
        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *pt = ptClassInfo->getObject(env,ptObj);
        if (!inst || !pt)
            return;

        inst->loc = GeoCoord(pt->x(),pt->y());
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::setBaseCenter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setBaseHeight
  (JNIEnv *env, jobject obj, jdouble baseHeight)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->baseHeight = baseHeight;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::setBaseHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->height = height;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::setHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setRadius
  (JNIEnv *env, jobject obj, jdouble radius)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->radius = radius;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::setRadius()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setSample
  (JNIEnv *env, jobject obj, jint sample)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        Cylinder *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->sampleX = sample;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeCylinder::setSample()");
    }
}
