/*
 *  SingleVertexAttribute_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 23/1/16.
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
#import "com_mousebird_maply_SingleVertexAttribute.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_nativeInit
(JNIEnv *env, jclass cls)
{
    SingleVertexAttributeClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_initialise
(JNIEnv *env, jobject obj, jstring name, jint type)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *attr = new SingleVertexAttribute();
        attr->type = (BDAttributeDataType)type;
        JavaString jstr(env, name);
        attr->name = jstr.cStr;
        classInfo->setHandle(env, obj, attr);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setVec4Values
(JNIEnv *env, jobject obj, jfloatArray vecArray)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        jfloat * data = env->GetFloatArrayElements(vecArray, 0);
        inst->data.vec4[0] = data[0];
        inst->data.vec4[1] = data[1];
        inst->data.vec4[2] = data[2];
        inst->data.vec4[3] = data[3];
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setVec4Values()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getVec4Values
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst) {
            jfloatArray result;
            result = env->NewFloatArray(0);
            return result;
        }
        jfloatArray result = env->NewFloatArray(4);
        env->SetFloatArrayRegion(result, 0, 3, inst->data.vec4);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getVec4Values()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setVec3Values
(JNIEnv *env, jobject obj, jfloatArray vecArray)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        jfloat * data = env->GetFloatArrayElements(vecArray, 0);
        inst->data.vec3[0] = data[0];
        inst->data.vec3[1] = data[1];
        inst->data.vec3[2] = data[2];
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setVec3Values()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getVec3Values
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst) {
            jfloatArray result;
            result = env->NewFloatArray(0);
            return result;
        }
        jfloatArray result = env->NewFloatArray(3);
        env->SetFloatArrayRegion(result, 0, 2, inst->data.vec3);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getVec3Values()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setVec2Values
(JNIEnv *env, jobject obj, jfloatArray vecArray)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        jfloat * data = env->GetFloatArrayElements(vecArray, 0);
        inst->data.vec2[0] = data[0];
        inst->data.vec2[1] = data[1];
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setVec2Values()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getVec2Values
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst) {
            jfloatArray result;
            result = env->NewFloatArray(0);
            return result;
        }
        jfloatArray result = env->NewFloatArray(2);
        env->SetFloatArrayRegion(result, 0, 1, inst->data.vec2);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getVec2Values()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setFloatVal
(JNIEnv *env, jobject obj, jfloat val)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->data.floatVal = val;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setFloatVal()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getFloatVal
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;

        return inst->data.floatVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getFloatVal()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setIntVal
(JNIEnv *env, jobject obj, jint val)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->data.intVal = val;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setIntVal()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getIntVal
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->data.intVal;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getIntVal()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_SingleVertexAttribute_setColor
(JNIEnv *env, jobject obj, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        inst->data.color[0] = r;
        inst->data.color[1] = g;
        inst->data.color[2] = b;
        inst->data.color[3] = a;
        
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::setColor()");
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_mousebird_maply_SingleVertexAttribute_getColor
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env, obj);
        if (!inst) {
            jfloatArray result;
            result = env->NewFloatArray(0);
            return result;
        }
        jcharArray result = env->NewCharArray(4);
        env->SetCharArrayRegion(result, 0, 3, inst->data.color);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SingleVertexAttribute::getColor()");
    }
}
