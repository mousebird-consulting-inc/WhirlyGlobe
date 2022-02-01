/*  ShapeCircle_jni.cpp
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
#import "com_mousebird_maply_ShapeCircle.h"

using namespace WhirlyKit;

template<> CircleClassInfo *CircleClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_nativeInit
  (JNIEnv *env, jclass cls)
{
    CircleClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        CircleClassInfo::set(env, obj, new Circle());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Circle *inst = CircleClassInfo::get(env, obj))
        if (Point2d *loc = Point2dClassInfo::get(env,ptObj))
        {
            inst->loc = GeoCoord(loc->x(),loc->y());
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setLoc3d
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Circle *inst = CircleClassInfo::get(env, obj))
        if (Point3d *loc = Point3dClassInfo::get(env,ptObj))
        {
            inst->loc = GeoCoord(loc->x(),loc->y());
            inst->height = loc->z();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        if (Circle *inst = classInfo->getObject(env, obj))
        {
            inst->height = height;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setRadius
  (JNIEnv *env, jobject obj, jdouble radius)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        if (Circle *inst = classInfo->getObject(env, obj))
        {
            inst->radius = radius;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCircle_setSample
  (JNIEnv *env, jobject obj, jint sampleX)
{
    try
    {
        CircleClassInfo *classInfo = CircleClassInfo::getClassInfo();
        if (Circle *inst = classInfo->getObject(env, obj))
        {
            inst->sampleX = sampleX;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
