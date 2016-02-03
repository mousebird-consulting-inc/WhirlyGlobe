/*
 *  OpenGLESAttribute_jni.cpp
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
#import "com_mousebird_maply_OpenGLESAttribute.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_nativeInit
(JNIEnv *env, jclass cls) {
    
    OpenGLESAttributeClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *attr = new OpenGLESAttribute();
        classInfo->setHandle(env, obj, attr);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
        classInfo->clearHandle(env, obj);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::dispose()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLESAttribute_isArray
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->isArray();
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::isArray()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_OpenGLESAttribute_isType
(JNIEnv *env, jobject obj, jint inType)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return false;
        
        return inst->isType(inType);
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::isType()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_setName
(JNIEnv *env, jobject obj, jstring name)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        JavaString jstr(env, name);
        inst->name = jstr.cStr;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::isArray()");
    }
}

JNIEXPORT jstring JNICALL Java_com_mousebird_maply_OpenGLESAttribute_getName
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        std::string empty = "";
        if (!inst)
            return env->NewStringUTF(empty.c_str());
        
        return env->NewStringUTF(inst->name.c_str());
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::getName()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_setIndex
(JNIEnv *env, jobject obj, jint index)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->index = index;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::setIndex()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLESAttribute_getIndex
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->index;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::getIndex()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_setSize
(JNIEnv *env, jobject obj, jint size)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->size = size;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::setSize()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLESAttribute_getSize
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
        
        return inst->size;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::getSize()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_OpenGLESAttribute_setType
(JNIEnv *env, jobject obj, jint type)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->type = type;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::setType()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_OpenGLESAttribute_getType
(JNIEnv *env, jobject obj)
{
    try
    {
        OpenGLESAttributeClassInfo *classInfo = OpenGLESAttributeClassInfo::getClassInfo();
        OpenGLESAttribute *inst = classInfo->getObject(env, obj);
        if (!inst)
            return -1;
     
        return inst->type;
    }
    catch(...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in OpenGLESAttribute::getType()");
    }
}
