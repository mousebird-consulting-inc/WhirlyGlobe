/*
 *  QIFFrameAsset_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/29/19.
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
#import "com_mousebird_maply_QIFFrameAsset.h"

using namespace WhirlyKit;

template<> QIFFrameAssetClassInfo *QIFFrameAssetClassInfo::classInfoObj = NULL;


JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_nativeInit
        (JNIEnv *env, jclass cls)
{
    QIFFrameAssetClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_initialise
        (JNIEnv *env, jobject obj)
{
    try {
        QIFFrameAssetClassInfo *info = QIFFrameAssetClassInfo::getClassInfo();
        PlatformInfo_Android platformInfo(env);
        QIFFrameAsset_Android *frame = new QIFFrameAsset_Android(&platformInfo,NULL);
        frame->frameAssetObj = env->NewGlobalRef(obj);
        info->setHandle(env, obj, frame);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFFrameAsset::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_dispose
        (JNIEnv *env, jobject obj)
{
    try {
        QIFFrameAssetClassInfo *info = QIFFrameAssetClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QIFFrameAsset_Android *frame = info->getObject(env,obj);
            if (!frame)
                return;
            if (frame->frameAssetObj) {
                env->DeleteGlobalRef(frame->frameAssetObj);
                frame->frameAssetObj = NULL;
            }
            // These frames are actually reference counted by the TileAsset so we don't delete them here
//            delete frame;

            info->clearHandle(env, obj);
        }

    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFFrameAsset::dispose()");
    }
}

jobject MakeQIFFrameAsset(JNIEnv *env,QIFFrameAsset_Android *frame)
{
    QIFFrameAssetClassInfo *classInfo = QIFFrameAssetClassInfo::getClassInfo(env,"com/mousebird/maply/QIFFrameAsset");
    jobject obj = classInfo->makeWrapperObject(env,frame);
    frame->frameAssetObj = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);

    return obj;
}

JNIEXPORT jint JNICALL Java_com_mousebird_maply_QIFFrameAsset_getPriority
        (JNIEnv *env, jobject obj)
{
    try {
        QIFFrameAsset_Android *frame = QIFFrameAssetClassInfo::getClassInfo()->getObject(env,obj);
        if (!frame)
            return 0;
        return frame->getPriority();
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFFrameAsset::getPriority()");
    }

    return 0;
}
