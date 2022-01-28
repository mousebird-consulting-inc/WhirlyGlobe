/*  Shape_jni.cpp
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
#import "com_mousebird_maply_Shape.h"

using namespace WhirlyKit;

template<> ShapeClassInfo *ShapeClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_nativeInit
  (JNIEnv *env, jclass cls)
{
    ShapeClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Shape_getSelectID
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (const Shape *inst = ShapeClassInfo::get(env, obj))
        {
            return inst->selectID;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return -1;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectID
  (JNIEnv *env, jobject obj, jlong selectID)
{
    try
    {
        if (Shape *inst = ShapeClassInfo::get(env, obj))
        {
            inst->selectID = selectID;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shape_isSelectable
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (Shape *inst = ShapeClassInfo::get(env, obj))
        {
            return inst->isSelectable;
        }
    }
    MAPLY_STD_JNI_CATCH()
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectable
  (JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        if (Shape *inst = ShapeClassInfo::get(env, obj))
        {
            inst->isSelectable = selectable;
        }
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setColorInt
  (JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
    try
    {
        if (Shape *inst = ShapeClassInfo::get(env, obj))
        {
            inst->color = RGBAColor(r,g,b,a);
        }
    }
    MAPLY_STD_JNI_CATCH()
}


extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setClipCoords
  (JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        if (Shape *inst = ShapeClassInfo::get(env, obj))
        {
            inst->clipCoords = newVal;
        }
    }
    MAPLY_STD_JNI_CATCH()
}
