/*
 *  GeoJSONSource_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 4/3/17.
 *  Copyright 2011-2017 mousebird consulting
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
#import "com_mousebird_maply_GeoJSONSource.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeoJSONSource_nativeInit
(JNIEnv *env, jclass cls)
{
    GeoJSONSourceClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeoJSONSource_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        GeoJSONSourceClassInfo *classInfo = GeoJSONSourceClassInfo::getClassInfo();
        GeoJSONSource *inst = new GeoJSONSource();
        classInfo->setHandle(env,obj,inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeoJSONSource::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_GeoJSONSource_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        GeoJSONSourceClassInfo *classInfo = GeoJSONSourceClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            GeoJSONSource *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeoJSONSource::dispose()");
    }
}


JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_GeoJSONSource_parseData
  (JNIEnv *env, jobject obj, jstring json)
{
    try
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1");
        GeoJSONSourceClassInfo *classInfo = GeoJSONSourceClassInfo::getClassInfo();
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1a");
        GeoJSONSource *inst = classInfo->getObject(env,obj);
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1b");
        if (!inst || !json)
            return NULL;
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1c");

        std::vector<VectorObject *> vecObjs;
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1d");
        const char *cStr = env->GetStringUTFChars(json, 0);
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 1e");
        bool parsed = inst->parseData(cStr, vecObjs);
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 2");

        if (!parsed || vecObjs.empty())
            return NULL;

        VectorObjectClassInfo *vecClassInfo = VectorObjectClassInfo::getClassInfo();
        if (!vecClassInfo)
            vecClassInfo = VectorObjectClassInfo::getClassInfo(env,"com/mousebird/maply/VectorObject");

        jobjectArray retArr = env->NewObjectArray(vecObjs.size(), vecClassInfo->getClass(), NULL);
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 3");

        int which = 0;
        for (VectorObject *vecObj : vecObjs)
        {
            jobject vecObjObj = MakeVectorObject(env,vecObj);
            env->SetObjectArrayElement( retArr, which, vecObjObj);
            env->DeleteLocalRef( vecObjObj);
            which++;
        }
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "_parseData flag 4");
        
        return retArr;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in GeoJSONSource::parseData()");
    }
    
    return NULL;
}




