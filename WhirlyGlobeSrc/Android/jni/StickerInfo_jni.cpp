/*
 *  StickerInfo_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/16/15.
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
#import "com_mousebird_maply_StickerInfo.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerInfo_nativeInit
(JNIEnv *env, jclass cls)
{
    SphericalChunkInfoClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerInfo_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Dictionary dict;
        SphericalChunkInfo *stickerInfo = new SphericalChunkInfo(dict);
        SphericalChunkInfoClassInfo::getClassInfo()->setHandle(env,obj,stickerInfo);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkInfo::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerInfo_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SphericalChunkInfoClassInfo *classInfo = SphericalChunkInfoClassInfo::getClassInfo();
        SphericalChunkInfo *stickerInfo = classInfo->getObject(env,obj);
        if (!stickerInfo)
            return;
        delete stickerInfo;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkInfo::dispose()");
    }
}
