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

#import "Stickers_jni.h"
#import "Geometry_jni.h"
#import "CoordSystem_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_Sticker.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> SphericalChunkClassInfo *SphericalChunkClassInfo::classInfoObj = NULL;

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
        SphericalChunk *chunk = new SphericalChunk();
        SphericalChunkClassInfo::getClassInfo()->setHandle(env,obj,chunk);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            SphericalChunk *chunk = classInfo->getObject(env,obj);
            if (!chunk)
                return;
            delete chunk;
            
            classInfo->clearHandle(env,obj);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::dispose()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setLowerLeft
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
        if (!chunk || !pt)
            return;
        chunk->mbr.ll().x() = pt->x();
        chunk->mbr.ll().y() = pt->y();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setLowerLeft()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setUpperRight
  (JNIEnv *env, jobject obj, jobject ptObj)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        Point2d *pt = Point2dClassInfo::getClassInfo()->getObject(env,ptObj);
        if (!chunk || !pt)
            return;
        chunk->mbr.ur().x() = pt->x();
        chunk->mbr.ur().y() = pt->y();
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

        CoordSystemRef *coordSys = CoordSystemRefClassInfo::getClassInfo()->getObject(env,coordSysObj);
        if (coordSys)
            chunk->coordSys = *coordSys;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setCoordSys()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setSampling
(JNIEnv *env, jobject obj, jint sampleX, jint sampleY)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;

        chunk->sampleX = sampleX;
        chunk->sampleY = sampleY;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setSampling()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setEpsilon
(JNIEnv *env, jobject obj, jdouble eps, jint minSampleX, jint minSampleY, jint maxSampleX, jint maxSampleY)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;
        
        chunk->eps = eps;
        chunk->minSampleX = minSampleX;  chunk->minSampleY = minSampleY;
        chunk->sampleX = maxSampleX;  chunk->sampleY = maxSampleY;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setEpsilon()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sticker_setTextureIDs
  (JNIEnv *env, jobject obj, jlongArray texIDs)
{
    try
    {
        SphericalChunkClassInfo *classInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunk *chunk = classInfo->getObject(env,obj);
        if (!chunk)
            return;

        chunk->texIDs.clear();
        
        JavaLongArray texArray(env,texIDs);
        for (int ii=0;ii<texArray.len;ii++)
            chunk->texIDs.push_back(texArray.rawLong[ii]);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunk::setTextureIDs()");
    }
}
