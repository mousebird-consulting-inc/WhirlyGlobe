/*
 *  QuadImageFrameLoader_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/8/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "QuadLoading_jni.h"
#import "Geometry_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_QuadImageFrameLoader.h"

using namespace WhirlyKit;

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setLoadFrameModeNative
        (JNIEnv *env, jobject obj, jint mode)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return false;

        if ((*loader)->getLoadMode() == (QuadImageFrameLoader::LoadMode)mode)
            return false;

        PlatformInfo_Android platformInfo(env);
        (*loader)->setLoadMode((QuadImageFrameLoader::LoadMode)mode);

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setLoadFrameModeNative()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_addFocus
        (JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        (*loader)->addFocus();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::addFocus()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getNumFocus
        (JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return 0;

        return (*loader)->getNumFocus();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::getNumFocus()");
    }

    return 0;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setCurrentImageNative
        (JNIEnv *env, jobject obj, jint focusID, jdouble curImage)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return false;

        PlatformInfo_Android platformInfo(env);
        double oldPos = (*loader)->getCurFrame(focusID);
        (*loader)->setCurFrame(&platformInfo,focusID,curImage);

        return (int)oldPos != (int)curImage;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setCurrentImageNative()");
    }

    return false;
}

JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getCurrentImage
        (JNIEnv *env, jobject obj, jint focusID)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return 0.0;

        return (*loader)->getCurFrame(focusID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::getCurrentImage()");
    }

    return 0.0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setRequireTopTiles
        (JNIEnv *env, jobject obj, jboolean loadTopTiles)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        (*loader)->setRequireTopTilesLoaded(loadTopTiles);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setRequireTopTiles()");
    }
}


JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setRenderTargetIDNative
        (JNIEnv *env, jobject obj, jint focusID, jlong targetID)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        (*loader)->setRenderTarget(focusID,targetID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setRenderTargetIDNative()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setTextureSize
        (JNIEnv *env, jobject obj, jint texSize, jint borderSize)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        (*loader)->setTexSize(texSize,borderSize);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setTextureSize()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_setShaderIDNative
        (JNIEnv *env, jobject obj, jint focusID, jlong shaderID)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        (*loader)->setShaderID(focusID,shaderID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::setRenderTargetIDNative()");
    }
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_getStatsNative
        (JNIEnv *env, jobject obj, jintArray totalTilesArr, jintArray tilesToLoadArr)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return 0;

        QuadImageFrameLoader::Stats stats = (*loader)->getStats();
        int numFrames = stats.frameStats.size();
        std::vector<int> totalTiles(numFrames),tilesToLoad(numFrames);
        for (unsigned int ii=0;ii<numFrames;ii++) {
            auto &frame = stats.frameStats[ii];
            totalTiles[ii] = frame.totalTiles;
            tilesToLoad[ii] = frame.tilesToLoad;
        }
        env->SetIntArrayRegion(totalTilesArr,0,totalTiles.size(),&totalTiles[0]);
        env->SetIntArrayRegion(tilesToLoadArr,0,tilesToLoad.size(),&tilesToLoad[0]);

        return stats.numTiles;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::getStatsNative()");
    }

    return 0;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QuadImageFrameLoader_updatePriorities
        (JNIEnv *env, jobject obj)
{
    try {
        QuadImageFrameLoader_AndroidRef *loader = QuadImageFrameLoaderClassInfo::getClassInfo()->getObject(env,obj);
        if (!loader)
            return;

        PlatformInfo_Android platformInfo(env);
        (*loader)->updatePriorities(&platformInfo);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QuadImageFrameLoader::updatePriorities()");
    }
}