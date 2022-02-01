/*  ShapeInfo_jni.cpp
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
#import "com_mousebird_maply_ShapeInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ShapeInfoClassInfo *ShapeInfoClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_nativeInit
  (JNIEnv *env, jclass cls)
{
    ShapeInfoClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo::set(env, obj, new ShapeInfoRef(std::make_shared<ShapeInfo>()));
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setColorInt
  (JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
    try
    {
        if (ShapeInfoRef *inst = ShapeInfoClassInfo::get(env, obj))
        {
            (*inst)->color = RGBAColor(r,g,b,a);
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setLineWidth
  (JNIEnv *env, jobject obj, jfloat lineWidth)
{
    try
    {
        if (ShapeInfoRef *inst = ShapeInfoClassInfo::get(env, obj))
        {
            (*inst)->lineWidth = lineWidth;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setInsideOut
  (JNIEnv *env, jobject obj, jboolean insideOut)
{
    try
    {
        if (ShapeInfoRef *inst = ShapeInfoClassInfo::get(env, obj))
        {
            (*inst)->insideOut = insideOut;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setCenter
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        if (ShapeInfoRef *inst = ShapeInfoClassInfo::get(env, obj))
        if (const Point3d *center = Point3dClassInfo::get(env, ptObj))
        {
            (*inst)->hasCenter = true;
            (*inst)->center = *center;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
