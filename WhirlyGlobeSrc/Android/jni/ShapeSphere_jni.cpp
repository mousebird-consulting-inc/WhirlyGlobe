/*
 *  ShapeSphere_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "WhirlyGlobe.h"
#import "com_mousebird_maply_ShapeSphere.h"
#import "Maply_utils_jni.h"



using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeSphereClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = new WhirlyKitSphere();
        classInfo->setHandle(env, obj, inst);
        
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
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
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        Point2d *loc = Point2dClassInfo::getClassInfo()->getObject(env, ptObj);

        if (!inst || !loc)
            return;
        
        WhirlyKit::GeoCoord newLoc(loc->x(), loc->y());
        inst->setLoc(newLoc);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setLoc()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_ShapeSphere_getLoc
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        
        if (!inst)
            return NULL;
        WhirlyKit::GeoCoord loc = inst->getLoc();
        Point2d newLoc(loc.x(), loc.y());
        return MakePoint2d(env,newLoc);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::getLoc()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_ShapeSphere_getHeight
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getHeight();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::getHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setHeight
(JNIEnv *env, jobject obj, jfloat height)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setHeight(height);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setHeight()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_ShapeSphere_getRadius
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getRadius();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::getRadius()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setRadius
(JNIEnv *env, jobject obj, jfloat radius)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setRadius(radius);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setRadius()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setSampleX
(JNIEnv *env, jobject obj, jint sampleX)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setSampleX(sampleX);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setSampleX()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ShapeSphere_getSampleX
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getSampleX();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::getSampleX()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setSampleY
(JNIEnv *env, jobject obj, jint sampleY)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setSampleY(sampleY);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::setSampleY()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_ShapeSphere_getSampleY
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeSphereClassInfo *classInfo = ShapeSphereClassInfo::getClassInfo();
        WhirlyKitSphere *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getSampleY();
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeSphere::getSampleY()");
    }
}
