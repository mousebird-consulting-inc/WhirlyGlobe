/*  ShapeExtruded_jni.cpp
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
#import "com_mousebird_maply_ShapeExtruded.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ExtrudedClassInfo *ExtrudedClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_nativeInit
  (JNIEnv *env, jclass cls)
{
    ExtrudedClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        ExtrudedClassInfo::set(env, obj, new Extruded());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        ExtrudedClassInfo *classInfo = ExtrudedClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setLoc
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        if (const Point2d *pt = Point2dClassInfo::get(env,ptObj))
        {
            inst->loc.x() = pt->x();
            inst->loc.y() = pt->y();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setLoc3d
        (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        if (const Point3d *pt = Point3dClassInfo::get(env,ptObj))
        {
            inst->loc.x() = pt->x();
            inst->loc.y() = pt->y();
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setHeight
  (JNIEnv *env, jobject obj, jdouble height)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        {
            inst->loc.z() = height;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setOutline
  (JNIEnv *env, jobject obj, jobjectArray coordsObj)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        {
            const auto info = Point2dClassInfo::getClassInfo();
            JavaObjectArrayHelper coordsHelp(env, coordsObj);
            inst->pts.reserve(coordsHelp.numObjects());
            while (jobject ptObj = coordsHelp.getNextObject())
            {
                inst->pts.push_back(*info->getObject(env, ptObj));
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setScale
  (JNIEnv *env, jobject obj, jdouble scale)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        {
            inst->scale = scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_ShapeExtruded_getScale
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const Extruded *inst = ExtrudedClassInfo::get(env, obj))
        {
            return inst->scale;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 1.0;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setThickness
  (JNIEnv *env, jobject obj, jdouble thick)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        {
            inst->thickness = thick;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeExtruded_setTransform
  (JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        if (Extruded *inst = ExtrudedClassInfo::get(env, obj))
        if (const Matrix4d *mat = Matrix4dClassInfo::get(env, matObj))
        {
            inst->transform = *mat;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
