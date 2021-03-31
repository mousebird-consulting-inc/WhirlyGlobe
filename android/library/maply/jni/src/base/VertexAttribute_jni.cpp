/*  VertexAttribute_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/17/16.
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

#import "Base_jni.h"
#import "com_mousebird_maply_VertexAttribute.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SingleVertexAttributeClassInfo *SingleVertexAttributeClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_nativeInit(JNIEnv *env, jclass cls)
{
    SingleVertexAttributeClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_initialise(JNIEnv *env, jobject obj)
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

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setName(JNIEnv *env, jobject obj, jstring str)
{
    try {
        auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (SingleVertexAttribute *inst = classInfo->getObject(env,obj)) {
            JavaString jStr(env, str);
            inst->nameID = StringIndexer::getStringID(jStr.getCString());
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setName()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_dispose(JNIEnv *env, jobject obj)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        std::lock_guard<std::mutex> lock(disposeMutex);
        if (auto inst = classInfo->getObject(env,obj)) {
            delete inst;
            classInfo->clearHandle(env,obj);
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setFloat(JNIEnv *env, jobject obj, jfloat val)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env,obj)) {
            inst->type = BDFloatType;
            inst->data.floatVal = val;
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setFloat()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setInt(JNIEnv *env, jobject obj, jint val)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env,obj)) {
            inst->type = BDIntType;
            inst->data.intVal = val;
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setInt()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setVec2(JNIEnv *env, jobject obj, jfloat x, jfloat y)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env,obj)) {
            inst->type = BDFloat2Type;
            inst->data.vec2[0] = x;
            inst->data.vec2[1] = y;
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setVec2()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setVec3(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat z)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env,obj)) {
            inst->type = BDFloat3Type;
            inst->data.vec3[0] = x;
            inst->data.vec3[1] = y;
            inst->data.vec3[2] = z;
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setVec3()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_VertexAttribute_setColor(JNIEnv *env, jobject obj, jint r, jint g, jint b, jint a)
{
    try {
        const auto classInfo = SingleVertexAttributeClassInfo::getClassInfo();
        if (auto inst = classInfo->getObject(env,obj)) {
            inst->type = BDChar4Type;
            inst->data.color[0] = r;
            inst->data.color[1] = g;
            inst->data.color[2] = b;
            inst->data.color[3] = a;
        }
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in VertexAttribute::setColor()");
    }
}
