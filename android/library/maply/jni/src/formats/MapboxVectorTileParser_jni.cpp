/*  MapboxVectorTileParser_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

#import <Formats_jni.h>
#import <Geometry_jni.h>
#import <Vectors_jni.h>
#import <Components_jni.h>
#import <QuadLoading_jni.h>
#import "WhirlyGlobe_Android.h"
#import "com_mousebird_maply_MapboxVectorTileParser.h"

using namespace WhirlyKit;

template<> MapboxVectorTileParserClassInfo *MapboxVectorTileParserClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_nativeInit(JNIEnv *env, jclass cls)
{
    MapboxVectorTileParserClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_initialise
    (JNIEnv *env, jobject obj, jobject vecStyleObj, jboolean isMapboxStyle)
{
    try
    {
        PlatformInfo_Android platformInfo(env);

        if (isMapboxStyle) {
            MapboxVectorStyleSetImpl_AndroidRef *style = MapboxVectorStyleSetClassInfo::getClassInfo()->getObject(env,vecStyleObj);
            if (!style)
                return;

            MapboxVectorTileParser *inst = new MapboxVectorTileParser(&platformInfo, *style);
            MapboxVectorTileParserClassInfo::getClassInfo()->setHandle(env,obj,inst);
        } else {
            VectorStyleSetWrapper_AndroidRef *style = VectorStyleSetWrapperClassInfo::getClassInfo()->getObject(env,vecStyleObj);
            if (!style)
                return;

            MapboxVectorTileParser *inst = new MapboxVectorTileParser(&platformInfo,*style);
            MapboxVectorTileParserClassInfo::getClassInfo()->setHandle(env,obj,inst);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        MapboxVectorTileParserClassInfo *classInfo = MapboxVectorTileParserClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            MapboxVectorTileParser *inst = classInfo->getObject(env,obj);
            if (!inst)
                return;
            delete inst;
        }

        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_setLocalCoords
    (JNIEnv *env, jobject obj, jboolean localCoords)
{
    try {
        MapboxVectorTileParser *inst = MapboxVectorTileParserClassInfo::getClassInfo()->getObject(
                env, obj);
        if (!obj)
            return;
        inst->setLocalCoords(localCoords);
    }
    catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply",
                            "Crash in MapboxVectorTileParser::setLocalCoords()");
    }
}

static bool noCancel(PlatformThreadInfo*) { return false; }

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_MapboxVectorTileParser_parseData
    (JNIEnv *env, jobject obj, jbyteArray data, jobject vecTileDataObj, jobject loadRetObj)
{
    try
    {
        const auto inst = MapboxVectorTileParserClassInfo::get(env,obj);
        const auto tileDataPtr = VectorTileDataClassInfo::get(env,vecTileDataObj);
        const auto tileData = tileDataPtr ? *tileDataPtr : nullptr;
        if (!inst || !tileData)
        {
            return false;
        }

        const auto loadRetPtr = LoaderReturnClassInfo::get(env,loadRetObj);
        const auto loadRet = (loadRetPtr && *loadRetPtr) ? loadRetPtr->get() : nullptr;

        using CancelFunction = MapboxVectorTileParser::CancelFunction;
        const CancelFunction loadRetCancel = [=](auto){return loadRet->cancel;};
        const auto cancelFn = loadRet ? loadRetCancel : noCancel;

        // Copy data into a temporary buffer (must we?)
        const int len = env->GetArrayLength(data);
        bool ret = false;
        if (jbyte *rawData = env->GetByteArrayElements(data,nullptr))
        {
            try
            {
                // Need a pointer to this JNIEnv for low level parsing callbacks
                PlatformInfo_Android platformInfo(env);

                RawDataWrapper rawDataWrap(rawData, len, false);
                ret = inst->parse(&platformInfo,&rawDataWrap,tileData.get(),cancelFn);
            }
            catch (...)
            {
                // since we can't `finally{}`, handle and re-throw.  todo: RAII wrapper
                env->ReleaseByteArrayElements(data, rawData, JNI_ABORT);
                throw;
            }
            env->ReleaseByteArrayElements(data, rawData, JNI_ABORT);
        }

        return ret;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in MapboxVectorTileParser::parseDataNative()");
    }

    return false;
}
