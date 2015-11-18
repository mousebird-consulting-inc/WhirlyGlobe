/*
 *  StickerMAnager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/18/15.
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
#import "com_mousebird_maply_StickerManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

typedef JavaClassInfo<SphericalChunkManager> StickerManagerClassInfo;
template<> StickerManagerClassInfo *StickerManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_nativeInit
(JNIEnv *env, jclass cls)
{
    StickerManagerClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        SphericalChunkManager *chunkManager = dynamic_cast<SphericalChunkManager *>(scene->getManager(kWKSphericalChunkManager));
        StickerManagerClassInfo::getClassInfo()->setHandle(env,obj,chunkManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        StickerManagerClassInfo::getClassInfo()->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_StickerManager_addSticker
(JNIEnv *env, jobject obj, jobject stickerObj, jobject stickerInfoObj, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        SphericalChunkInfo *chunkInfo = (SphericalChunkInfo *)SphericalChunkInfoClassInfo::getClassInfo()->getObject(env,stickerInfoObj);
        SphericalChunk *chunk = SphericalChunkClassInfo::getClassInfo()->getObject(env,stickerObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !chunkInfo || !chunk || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in SphericalChunkManager::addSticker()");
            return EmptyIdentity;
        }
        
        SimpleIdentity chunkId = chunkManager->addChunk(chunk,*chunkInfo,*changeSet);
        
        return chunkId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::addSticker()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_removeStickers
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return;
        
        JavaLongArray idArray(env,idArrayObj);
        SimpleIDSet ids;
        for (int ii=0;ii<idArray.len;ii++)
            ids.insert(idArray.rawLong[ii]);
        
        chunkManager->removeChunks(ids,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::removeStickers()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_enableStickers
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return;
        
        JavaLongArray idArray(env,idArrayObj);
        SimpleIDSet ids;
        for (int ii=0;ii<idArray.len;ii++)
        {
            chunkManager->enableChunk(idArray.rawLong[ii],enable,*changeSet);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::enableStickers()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_StickerManager_modifyChunkTextures
(JNIEnv *env, jobject obj, jlong chunkID, jlongArray texIDsObj, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return false;
        
        JavaLongArray idArray(env,texIDsObj);
        std::vector<SimpleIdentity> texIDs;
        for (int ii=0;ii<idArray.len;ii++)
            texIDs.push_back(idArray.rawLong[ii]);
        
        chunkManager->modifyChunkTextures(chunkID,texIDs,*changeSet);
        
        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::modifyChunkTextures()");
    }
}
