/*
 *  ShapeInfo_jni.cpp
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
#import "com_mousebird_maply_ShapeInfo.h"
#import "ShapeDrawableBuilder.h"
#import "Maply_utils_jni.h"


using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeInfoClassInfo::getClassInfo(env, cls);

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = new WhirlyKitShapeInfo();
        classInfo->setHandle(env, obj, inst);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        RGBAColor color(r*255.0,g*255.0,b*255.0,a*255.0);
        inst->setColor(color);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setColor()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_ShapeInfo_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
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
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getColor()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setLineWidth
(JNIEnv *env, jobject obj, jfloat lineWidth)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setLineWidth(lineWidth);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setLineWidth()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_ShapeInfo_getLineWidth
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        return inst->getLineWidth();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getLineWidth()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setInsideOut
(JNIEnv *env, jobject obj, jboolean insideOut)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setInsideOut(insideOut);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setInsideOut()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ShapeInfo_getInsideOut
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->getInsideOut();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getInsideOut()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setZBufferRead
(JNIEnv *env, jobject obj, jboolean zBufferRead)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setZBufferRead(zBufferRead);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setZBufferRead()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ShapeInfo_getZBufferRead
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->getZBufferRead();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getZBufferRead()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setZBufferWrite
(JNIEnv *env, jobject obj, jboolean zBufferWrite)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->setZBufferWrite(zBufferWrite);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setZBufferWrite()");
    }

}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_ShapeInfo_getZBufferWrite
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        return inst->getZBufferWrite();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getZBufferWrite()");
    }

}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_ShapeInfo_getCenter
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        if (!inst->getHasCenter())
            return NULL;
        Point3d pt = inst->getCenter();
        return MakePoint3d(env,pt);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::getCenter()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeInfo_setCenter
(JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        ShapeInfoClassInfo *classInfo = ShapeInfoClassInfo::getClassInfo();
        WhirlyKitShapeInfo *inst = classInfo->getObject(env, obj);
        Point3d *center = Point3dClassInfo::getClassInfo()->getObject(env, ptObj);
        if (!inst || !center)
            return;
        inst->setHasCenter(true);
        inst->setCenter(*center);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeInfo::setCenter()");
    }
}
