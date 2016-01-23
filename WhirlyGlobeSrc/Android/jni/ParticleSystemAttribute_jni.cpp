/*
 *  ParticleSystemAttribute_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 21/1/16.
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
#import "com_mousebird_maply_ParticleSystemAttribute.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemAttribute_nativeInit
(JNIEnv *env, jclass cls)
{
    ParticleSystemAttributeClassInfo::getClassInfo(env,cls);

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemAttribute_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        ParticleSystemAttributeClassInfo info = ParticleSystemAttributeClassInfo::getClassInfo();
        SingleVertexAttributeInfo *inst = new SingleVertexAttributeInfo();
        info->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemAttribute::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemAttribute_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ParticleSystemAttributeClassInfo *classInfo = ParticleSystemAttributeClassInfo::getClassInfo();
        SingleVertexAttributeInfo *parcSysAttr = classInfo->getObject(env,obj);
        if (!parcSysAttr)
            return;
        delete parcSysAttr;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemAttribute::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemAttribute_setName
(JNIEnv *env, jobject obj, jstring name)
{
    try
    {
        ParticleSystemAttributeClassInfo *classInfo = ParticleSystemAttributeClassInfo::getClassInfo();
        SingleVertexAttributeInfo *parcSysAttr = classInfo->getObject(env,obj);
        if (!parcSysAttr)
            return;
        parcSysAttr->name = name;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemAttribute::setName()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ParticleSystemAttribute_setType
(JNIEnv *env, jobject obj, jobject type)
{
    try{
        ParticleSystemAttributeClassInfo *classInfo = ParticleSystemAttributeClassInfo::getClassInfo();
        SingleVertexAttributeInfo *parcSysAttr = classInfo->getObject(env,obj);
        if (!parcSysAttr)
            return;
        jmethodID getValueMethod = (*env)->GetMethodID(env, (*env)->FindClass(env, "ParticleSystemAttribute"), "getValue", "I()");
        jint value = (*env)->CallIntMethod(env, type, getValueMethod);
        parcSysAttr->type = value;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ParticleSystemAttribute::setType()");
    }
}
