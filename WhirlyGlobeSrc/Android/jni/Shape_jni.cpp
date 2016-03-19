/*
 *  Shape_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2015 mousebird consulting
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
#import <jni.h>
#import "Maply_jni.h"
#import "WhirlyGlobe.h"
#import "com_mousebird_maply_Shape.h"


using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeClassInfo::getClassInfo(env, cls);
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shape_isSelectable
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
    
        return inst->getSelectable();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::isSelectable()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectable
(JNIEnv *env, jobject obj, jboolean selectable)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setSelectable(selectable);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setSelectable()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setSelectID
(JNIEnv *env, jobject obj, jlong selectID)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setSelectID(selectID);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setSelectID()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_Shape_getSelectID
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->getSelectID();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::getSelectID()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Shape_getUseColor
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->getUseColor();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::getUseColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setUseColor
(JNIEnv *env, jobject obj, jboolean useColor)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setUseColor(useColor);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setUseColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Shape_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        RGBAColor color(r*255.0,g*255.0,b*255.0,a*255.0);
        inst->setColor(color);
    
    
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::setColor()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_Shape_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeClassInfo *classInfo = ShapeClassInfo::getClassInfo();
        WhirlyKitShape *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        RGBAColor color = inst->getColor();
        float * primaryColors = new float[4];
        color.asUnitFloats(primaryColors);
        jfloatArray result;
        result = env->NewFloatArray(4);
        env->SetFloatArrayRegion(result, 0, 4, primaryColors);
        free(primaryColors);
        return result;
        
        
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Shape::getColor()");
    }
}

