/*
 *  Shape_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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
#import "com_mousebird_maply_Shape.h"

using namespace WhirlyKit;

template<> ShapeClassInfo *ShapeClassInfo::classInfoObj = NULL;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeClassInfo::getClassInfo(env, cls);
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Shape_getSelectID
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;

        return inst->selectID;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::getSelectID()");
    }

    return -1;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectID
(JNIEnv *env, jobject obj, jlong selectID)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->selectID = selectID;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setSelectID()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shape_isSelectable
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
    
        return inst->isSelectable;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::isSelectable()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectable
(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->isSelectable = selectable;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setSelectable()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->color = RGBAColor(r*255.0,g*255.0,b*255.0,a*255.0);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setColor()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setClipCoords
(JNIEnv *env, jobject obj, jboolean newVal)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        Shape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

        inst->clipCoords = newVal;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setClipCoords()");
    }
}
