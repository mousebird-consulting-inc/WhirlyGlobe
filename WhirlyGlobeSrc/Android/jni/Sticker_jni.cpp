/*
 *  Sticker_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/16/15.
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
#import "com_mousebird_maply_Sticker.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_nativeInit
(JNIEnv *env, jclass cls)
{
    SphericalChunkClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        Dictionary dict;
        SphericalChunk *chunk = new SphericalChunk();
        SphericalChunkClassInfo::getClassInfo()->setHandle(env,obj,chunk);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        delete chunk;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::dispose()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setLowerLeft
(JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        chunk->mbr.ll().x() = x;
        chunk->mbr.ll().y() = y;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setLowerLeft()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setUpperRight
(JNIEnv *env, jobject obj, jdouble x, jdouble y)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        chunk->mbr.ur().x() = x;
        chunk->mbr.ur().y() = y;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setUpperRight()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setRotation
(JNIEnv *env, jobject obj, jdouble rot)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        chunk->rotation = rot;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setRotation()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setCoordSys
(JNIEnv *env, jobject obj, jobject coordSysObj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        
        CoordSystem *coordSys = CoordSystemClassInfo::getClassInfo()->getObject(env,coordSysObj);
        if (coordSys)
            chunk->coordSys = coordSys;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setCoordSys()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setImageFormatNative
(JNIEnv *env, jobject obj, jint imageFormatEnum)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        
        chunk->imageFormat = (TileImageType)imageFormatEnum;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setImageFormat()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setImagesNative
(JNIEnv *env, jobject obj, jlongArray texArrayObj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;

        chunk->texIDs.clear();
        
        JavaLongArray texArray(env,texArrayObj);
        for (int ii=0;ii<texArray.len;ii++)
            chunk->texIDs.push_back(texArray.rawLong[ii]);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setImages()");
    }
}
