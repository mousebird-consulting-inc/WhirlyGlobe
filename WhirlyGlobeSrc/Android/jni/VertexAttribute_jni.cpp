/*
 *  VertexAttribute_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/16.
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

#import <jni.h>
#import "Maply_jni.h"
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_VertexAttribute.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_nativeInit
(JNIEnv *env, jclass cls)
{
    SingleVertexAttributeClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        SingleVertexAttribute *vertAttr = new SingleVertexAttribute();
        
        SingleVertexAttributeClassInfo::getClassInfo()->setHandle(env,obj,vertAttr);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setName
(JNIEnv *env, jobject obj, jstring str)
{
    try
    {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        JavaString jStr(env,str);
        inst->name = jStr.cStr;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setName()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_dispose
(JNIEnv *env, jobject obj)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            SingleVertexAttribute *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env,obj);
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setFloat
(JNIEnv *env, jobject obj, jfloat val)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->type = BDFloatType;
        inst->data.floatVal = val;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setFloat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setInt
(JNIEnv *env, jobject obj, jint val)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->type = BDIntType;
        inst->data.intVal = val;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setInt()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setVec2
(JNIEnv *env, jobject obj, jfloat x, jfloat y)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->type = BDFloat2Type;
        inst->data.vec2[0] = x;
        inst->data.vec2[1] = y;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setVec2()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setVec3
(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat z)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->type = BDFloat3Type;
        inst->data.vec2[0] = x;
        inst->data.vec2[1] = y;
        inst->data.vec2[2] = z;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setVec3()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setColor
(JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
    try {
        SingleVertexAttributeClassInfo *classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        SingleVertexAttribute *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->type = BDChar4Type;
        inst->data.color[0] = r;
        inst->data.color[1] = g;
        inst->data.color[2] = b;
        inst->data.color[3] = a;
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setColor()");
    }
}
