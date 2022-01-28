/*  ShapeGreatCircle_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "Shapes_jni.h"
#import "Geometry_jni.h"
#import "com_mousebird_maply_ShapeGreatCircle.h"

using namespace WhirlyKit;

template<> GreatCircleClassInfo *GreatCircleClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_nativeInit
  (JNIEnv *env, jclass cls)
{
    GreatCircleClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        GreatCircleClassInfo::set(env, obj, new GreatCircle_Android());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        GreatCircleClassInfo *classInfo = GreatCircleClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setPoints
  (JNIEnv *env, jobject obj, jobject startPtObj, jobject endPtObj)
{
    try
    {
        if (GreatCircle_Android *inst = GreatCircleClassInfo::get(env, obj))
        {
            if (const Point2d *startPt = Point2dClassInfo::get(env, startPtObj))
            {
                inst->startPt = *startPt;
            }
            if (const Point2d *endPt = Point2dClassInfo::get(env, endPtObj))
            {
                inst->endPt = *endPt;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        if (GreatCircle_Android *inst = GreatCircleClassInfo::get(env, obj))
        {
            inst->height = height;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setSamplingEpsilon
  (JNIEnv *env, jobject obj, jdouble eps)
{
    try
    {
        if (GreatCircle_Android *inst = GreatCircleClassInfo::get(env, obj))
        {
            inst->samplingEps = eps;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeGreatCircle_setSamplingStatic
  (JNIEnv *env, jobject obj, jint sample)
{
    try
    {
        if (GreatCircle_Android *inst = GreatCircleClassInfo::get(env, obj))
        {
            inst->sampleNum = sample;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ShapeGreatCircle_angleBetween
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (GreatCircle_Android *inst = GreatCircleClassInfo::get(env, obj))
        {
            const Point3d p0 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d(inst->startPt.x(), inst->startPt.y(), 0.0));
            const Point3d p1 = FakeGeocentricDisplayAdapter::LocalToDisplay(Point3d(inst->endPt.x(), inst->endPt.y(), 0.0));

            const double dot = p0.dot(p1);
            //    Point3f cross = p0.cross(p1);
            //    float mag = cross.norm();

            // Note: Atan2 is the correct way, but it's not working right here
            //return atan2f(dot, mag);
            const double ret = acos(dot);
            if (!std::isnan(ret))
            {
                return ret;
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0.0;
}