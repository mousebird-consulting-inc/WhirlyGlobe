/*  ShapeCylinder_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
 *  Copyright 2011-2021 mousebird consulting
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
#import "com_mousebird_maply_ShapeCylinder.h"

using namespace WhirlyKit;

template<> CylinderClassInfo *CylinderClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_nativeInit
  (JNIEnv *env, jclass cls)
{
    CylinderClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        CylinderClassInfo::set(env, obj, new Cylinder());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        CylinderClassInfo *classInfo = CylinderClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setBaseCenter
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        if (const Point2d *pt = Point2dClassInfo::get(env,ptObj))
        {
            inst->loc = GeoCoord(pt->x(),pt->y());
        }
    }
    MAPLY_STD_JNI_CATCH()
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setBaseCenter3d
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        if (const Point3d *pt = Point3dClassInfo::get(env,ptObj))
        {
            inst->loc = GeoCoord(pt->x(),pt->y());
            inst->height = pt->y();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setBaseHeight
  (JNIEnv *env, jobject obj, jdouble baseHeight)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        {
            inst->baseHeight = baseHeight;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        {
            inst->height = height;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setRadius
  (JNIEnv *env, jobject obj, jdouble radius)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        {
            inst->radius = radius;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeCylinder_setSample
  (JNIEnv *env, jobject obj, jint sample)
{
    try
    {
        if (Cylinder *inst = CylinderClassInfo::get(env, obj))
        {
            inst->sampleX = sample;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
