/*
 *  StickerMAnager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/18/15.
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
#import "Scene_jni.h"
#import "com_mousebird_maply_StickerManager.h"

using namespace WhirlyKit;
using namespace Maply;

static const char *SceneHandleName = "nativeSceneHandle";

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
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        SphericalChunkManager *chunkManager = dynamic_cast<SphericalChunkManager *>(scene->getManager(kWKSphericalChunkManager));
        StickerManagerClassInfo::getClassInfo()->setHandle(env,obj,chunkManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::initialise()");
    }
}

static std::mutex disposeMutex;

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

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_StickerManager_addStickers
        (JNIEnv *env, jobject obj, jobjectArray stickerArr, jobject stickerInfoObj, jobject changeSetObj)
{
    try
    {
        SphericalChunkManager *chunkManager = StickerManagerClassInfo::getClassInfo()->getObject(env,obj);
        SphericalChunkClassInfo *chunkClassInfo = SphericalChunkClassInfo::getClassInfo();
        SphericalChunkInfoRef *chunkInfo = SphericalChunkInfoClassInfo::getClassInfo()->getObject(env,stickerInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !chunkInfo || !changeSet)
        {
            __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "One of the inputs was null in SphericalChunkManager::addSticker()");
            return EmptyIdentity;
        }

        if ((*chunkInfo)->programID == EmptyIdentity) {
            ProgramGLES *prog = (ProgramGLES *)chunkManager->getScene()->findProgramByName(MaplyDefaultTriangleShader);
            if (prog)
                (*chunkInfo)->programID = prog->getId();
        }

        JavaObjectArrayHelper stickerHelp(env,stickerArr);
        std::vector<SphericalChunk> chunks;
        while (jobject stickerObjObj = stickerHelp.getNextObject()) {
            SphericalChunk *chunk = chunkClassInfo->getObject(env,stickerObjObj);
            if (chunk)
                chunks.push_back(*chunk);
        }

        SimpleIdentity chunkId = chunkManager->addChunks(chunks,*(*chunkInfo),*(changeSet->get()));

        return chunkId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::addSticker()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_StickerManager_modifyChunkTextures
(JNIEnv *env, jobject obj, jlong stickerID, jobject stickerInfoObj, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        SphericalChunkInfoRef *chunkInfo = SphericalChunkInfoClassInfo::getClassInfo()->getObject(env,stickerInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !chunkInfo || !changeSet)
            return false;

        chunkManager->modifyChunkTextures(stickerID,(*chunkInfo)->texIDs,*(changeSet->get()));

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::modifyChunkTextures()");
    }

    return false;
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_StickerManager_modifyDrawPriority
(JNIEnv *env, jobject obj, jlong stickerID, jint drawPriority, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return false;

        chunkManager->modifyDrawPriority(stickerID,drawPriority,*(changeSet->get()));

        return true;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::modifyDrawPriority()");
    }

    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_enableStickers
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return;

        JavaLongArray idArray(env,idArrayObj);
        SimpleIDSet ids;
        for (int ii=0;ii<idArray.len;ii++)
        {
            chunkManager->enableChunk(idArray.rawLong[ii],enable,*(changeSet->get()));
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::enableStickers()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_StickerManager_removeStickers
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeSetObj)
{
    try
    {
        StickerManagerClassInfo *classInfo = StickerManagerClassInfo::getClassInfo();
        SphericalChunkManager *chunkManager = classInfo->getObject(env,obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!chunkManager || !changeSet)
            return;
        
        JavaLongArray idArray(env,idArrayObj);
        SimpleIDSet ids;
        for (int ii=0;ii<idArray.len;ii++)
            ids.insert(idArray.rawLong[ii]);
        
        chunkManager->removeChunks(ids,*(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in SphericalChunkManager::removeStickers()");
    }
}
