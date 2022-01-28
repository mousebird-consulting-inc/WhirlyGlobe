/*  ShapeRectangle_jni.cpp
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
#import "com_mousebird_maply_ShapeRectangle.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> RectangleClassInfo *RectangleClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_nativeInit
  (JNIEnv *env, jclass cls)
{
    RectangleClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_initialise
  (JNIEnv *env, jobject obj)
{
    try
    {
        RectangleClassInfo::set(env, obj, new Rectangle());
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        RectangleClassInfo *classInfo = RectangleClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_setPoints
  (JNIEnv *env, jobject obj, jobject llObj, jobject urObj)
{
    try
    {
        if (Rectangle *inst = RectangleClassInfo::get(env, obj))
        {
            if (const Point3d *ll = Point3dClassInfo::get(env, llObj))
            {
                inst->setLL(*ll);
            }
            if (const Point3d *ur = Point3dClassInfo::get(env, urObj))
            {
                inst->setUR(*ur);
            }
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeRectangle_addTextureID
  (JNIEnv *env, jobject obj, jlong texID)
{
    try
    {
        if (Rectangle *inst = RectangleClassInfo::get(env, obj))
        {
            inst->texIDs.push_back(texID);
        }
    }
    MAPLY_STD_JNI_CATCH()
}
