/*  ShapeSphere_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "com_mousebird_maply_ShapeSphere.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SphereClassInfo *SphereClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_nativeInit
  (JNIEnv *env, jclass cls)
{
    SphereClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        SphereClassInfo::set(env, obj, new Sphere());
        
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        SphereClassInfo *classInfo = SphereClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Sphere *inst = SphereClassInfo::get(env, obj))
        if (const Point2d *loc = Point2dClassInfo::get(env, ptObj))
        {
            inst->loc = GeoCoord(loc->x(), loc->y());
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setLoc3d
        (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Sphere *inst = SphereClassInfo::get(env, obj))
        if (const Point3d *loc = Point3dClassInfo::get(env, ptObj))
        {
            inst->loc = GeoCoord(loc->x(), loc->y());
            inst->height = (float)loc->z();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setHeight
  (JNIEnv *env, jobject obj, jfloat height)
{
    try
    {
        if (Sphere *inst = SphereClassInfo::get(env, obj))
        {
            inst->height = height;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setRadius
  (JNIEnv *env, jobject obj, jfloat radius)
{
    try
    {
        if (Sphere *inst = SphereClassInfo::get(env, obj))
        {
            inst->radius = radius;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeSphere_setSamples
  (JNIEnv *env, jobject obj, jint sampleX, jint sampleY)
{
    try
    {
        if (Sphere *inst = SphereClassInfo::get(env, obj))
        {
            inst->sampleX = sampleX;
            inst->sampleY = sampleY;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
