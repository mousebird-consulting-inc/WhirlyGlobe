/*
 *  ShapeExtruded_jni.cpp
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
#import "com_mousebird_maply_ShapeExtruded.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ExtrudedClassInfo *ExtrudedClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_nativeInit
(JNIEnv *env, jclass cls)
{
    ExtrudedClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = new Extruded();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Extruded *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        Point2dClassInfo *ptClassInfo = Point2dClassInfo::getClassInfo();
        Point2d *pt = ptClassInfo->getObject(env,ptObj);
        if (!inst || !pt)
            return;

        inst->loc.x() = pt->x();
        inst->loc.y() = pt->y();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setLoc()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->loc.z() = height;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setOutline
  (JNIEnv *env, jobject obj, jobjectArray coordsObj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        JavaObjectArrayHelper coordsHelp(env,coordsObj);
        while (jobject ptObj = coordsHelp.getNextObject()) {
            Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
            inst->pts.push_back(*pt);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setOutline()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setScale
  (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->scale = scale;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setScale()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ShapeExtruded_getScale
  (JNIEnv *env, jobject obj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 1.0;

        return inst->scale;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::getScale()");
    }

    return 1.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setThickness
  (JNIEnv *env, jobject obj, jdouble thick)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->thickness = thick;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setThickness()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setTransform
  (JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        Extruded *inst = classInfo->getObject(env, obj);
        Matrix4dClassInfo *mat4dInfo = Matrix4dClassInfo::getClassInfo();
        Matrix4d *mat = mat4dInfo->getObject(env, matObj);
        if (!inst || !matObj)
            return;

        inst->transform = *mat;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeExtruded::setTransform()");
    }
}