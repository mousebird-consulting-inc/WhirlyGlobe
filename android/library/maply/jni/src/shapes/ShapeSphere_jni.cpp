/*
 *  ShapeSphere_jni.cpp
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
#import "com_mousebird_maply_ShapeSphere.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SphereClassInfo *SphereClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_nativeInit
(JNIEnv *env, jclass cls)
{
    SphereClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        Sphere *inst = new Sphere();
        classInfo->setHandle(env, obj, inst);
        
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            Sphere *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setLoc
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        Sphere *inst = classInfo->getObject(env, obj);
        Point2d *loc = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);

        if (!inst || !loc)
            return;
        
        WhirlyKit::GeoCoord newLoc(loc->x(), loc->y());
        inst->loc = newLoc;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setLoc()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setHeight
(JNIEnv *env, jobject obj, jfloat height)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        Sphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->height = height;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setRadius
(JNIEnv *env, jobject obj, jfloat radius)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        Sphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->radius = radius;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setRadius()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setSamples
(JNIEnv *env, jobject obj, jint sampleX, jint sampleY)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        Sphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->sampleX = sampleX;
        inst->sampleY = sampleY;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setSamples()");
    }
}
