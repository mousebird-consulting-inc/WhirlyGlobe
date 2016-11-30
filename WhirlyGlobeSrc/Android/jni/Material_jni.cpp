/*
 *  com_mousebird_maply_Material.cpp
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
#include "com_mousebird_maply_Material.h"
#import <jni.h>
#import "Maply_jni.h"
#import "Maply_utils_jni.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_nativeInit
(JNIEnv *env, jclass cls)
{
    MaterialClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst= new WhirlyKitMaterial();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            WhirlyKitMaterial *inst= classInfo->getObject(env, obj);
            if (!inst)
                return;

            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_setAmbient
(JNIEnv *env, jobject obj, jobject ambientObj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        Point4d *ambient = Point4dClassInfo::getClassInfo()->getObject(env, ambientObj);
        if (!inst || !ambient)
            return;

        inst->setAmbient(ambient->cast<float>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::setAmbient()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Material_getAmbient
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return MakePoint4d(env, inst->getAmbient().cast<double>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::getAmbient()");
    }
    
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_setDiffuse
(JNIEnv *env, jobject obj, jobject diffuseObj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        Point4d *diffuse = Point4dClassInfo::getClassInfo()->getObject(env, diffuseObj);
        if (!inst || !diffuse)
            return;

        inst->setDiffuse(diffuse->cast<float>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::setDiffuse()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Material_getDifusse
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return MakePoint4d(env, inst->getDiffuse().cast<double>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::getDiffuse()");
    }
    
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_setSpecular
(JNIEnv *env, jobject obj, jobject specularObj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        Point4d *specular = Point4dClassInfo::getClassInfo()->getObject(env, specularObj);
        if (!inst || !specular)
            return;

        inst->setSpecular(specular->cast<float>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::setSpecular()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Material_getSpecular
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        return MakePoint4d(env, inst->getSpecular().cast<double>());
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::getSpecular()");
    }
    
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Material_setSpecularExponent
(JNIEnv *env, jobject obj, jfloat specularExponent)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        inst->setSpecularExponent(specularExponent);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::setSpecularExponent()");
    }
}

JNIEXPORT jfloat JNICALL Java_com_mousebird_maply_Material_getSpecularExponent
(JNIEnv *env, jobject obj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        if (!inst)
            return 0.0;
        
        return inst->getSpecularExponent();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::getSpecularExponent()");
    }
    
    return 0.0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_Material_bindToProgram
(JNIEnv *env, jobject obj, jobject shaderObj)
{
    try
    {
        MaterialClassInfo *classInfo = MaterialClassInfo::getClassInfo();
        WhirlyKitMaterial *inst = classInfo->getObject(env, obj);
        OpenGLES2Program *program = OpenGLES2ProgramClassInfo::getClassInfo()->getObject(env, shaderObj);
        if (!inst || !program)
            return false;

        return inst->bindToProgram(program);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Material::bindToProgram()");
    }
    
    return false;
}
