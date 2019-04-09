/*
 *  ShapeGreatCircle_jni.cpp
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
#import "com_mousebird_maply_ShapeGreatCircle.h"

using namespace WhirlyKit;

template<> GreatCircleClassInfo *GreatCircleClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_nativeInit
(JNIEnv *env, jclass cls)
{
    GreatCircleClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = new GreatCircle_Android();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            GreatCircle_Android *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setPoints
  (JNIEnv *env, jobject obj, jobject startPtObj, jobject endPtObj)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = classInfo->getObject(env, obj);
        Point2dClassInfo *pt2dClassInfo = Point2dClassInfo::getClassInfo();
		Point2d *startPt = pt2dClassInfo->getObject(env,startPtObj);
		Point2d *endPt = pt2dClassInfo->getObject(env,endPtObj);
        if (!inst || !startPt || !endPt)
            return;

        inst->startPt = *startPt;
        inst->endPt = *endPt;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::setPoints()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->height = height;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::setHeight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setSamplingEpsilon
  (JNIEnv *env, jobject obj, jdouble eps)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->samplingEps = eps;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::setSamplingEpsilon()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setSamplingStatic
  (JNIEnv *env, jobject obj, jint sample)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->sampleNum = sample;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::setSamplingStatic()");
    }
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ShapeGreatCircle_angleBetween
        (JNIEnv *env, jobject obj)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        GreatCircle_Android *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;

        Point3d p0 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d(inst->startPt.x(),inst->startPt.y(),0.0));
        Point3d p1 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d(inst->endPt.x(),inst->endPt.y(),0.0));

        double dot = p0.dot(p1);
//    Point3f cross = p0.cross(p1);
//    float mag = cross.norm();

        // Note: Atan2 is the correct way, but it's not working right here
//    return atan2f(dot, mag);
        double ret = acos(dot);
        if (std::isnan(ret))
            ret = 0.0;

        return ret;
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeGreatCircle::angleBetween()");
    }

    return 0.0;
}