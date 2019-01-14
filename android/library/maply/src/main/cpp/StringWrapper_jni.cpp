/*
 *  StringWrapper_jni.cpp
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
#import <jni.h>
#import "Maply_jni.h"
#import "com_mousebird_maply_StringWrapper.h"
#import "WhirlyGlobe.h"
#import "Maply_utils_jni.h"

using namespace WhirlyKit;


JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_nativeInit
(JNIEnv *env, jclass cls)
{
    StringWrapperClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        StringWrapper *inst = new StringWrapper();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_initialise__IILcom_mousebird_maply_Matrix3d_2
(JNIEnv *env, jobject obj, jint height, jint width, jobject matrixObj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        Matrix3d *mat = Matrix3dClassInfo::getClassInfo()->getObject(env, matrixObj);
        if (!mat)
            return;
        StringWrapper *inst = new StringWrapper();
        inst->size = CGSize(height, width);
        inst->mat = *mat;
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::initialise()");
    }

}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            StringWrapper *inst = classInfo->getObject(env, obj);
            if (!inst)
                return;
            delete inst;
            classInfo->clearHandle(env, obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_setSize
(JNIEnv *env, jobject obj, jint height, jint width)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        StringWrapper *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        CGSize size(height, width);
        inst->size = size;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::setSize()");
    }
}

JNIEXPORT jintArray JNICALL Java_com_mousebird_maply_StringWrapper_getSize
(JNIEnv *env, jobject obj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        StringWrapper *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        
        int * size = new int[2];
        size[0] = inst->size.height;
        size[1] = inst->size.width;
        jintArray result;
        result = env->NewIntArray(2);
        env->SetIntArrayRegion(result, 0, 2, size);
        free(size);
        return result;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::getSize()");
    }
    
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StringWrapper_setMat
(JNIEnv *env, jobject obj, jobject matObj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        StringWrapper *inst = classInfo->getObject(env, obj);
        Matrix3d *mtx = Matrix3dClassInfo::getClassInfo()->getObject(env, matObj);
        if (!inst)
            return;
        inst->mat = *mtx;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::setMat()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_StringWrapper_getMat
(JNIEnv *env, jobject obj)
{
    try
    {
        StringWrapperClassInfo *classInfo = StringWrapperClassInfo::getClassInfo();
        StringWrapper *inst = classInfo->getObject(env, obj);
        if (!inst)
            return NULL;
        return MakeMatrix3d(env, inst->mat);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in StringWrapper::getMat()");
    }
    return NULL;
}
