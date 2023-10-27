/*  QIFFrameAsset_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/29/19.
 *  Copyright 2011-2022 mousebird consulting
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

#import "QuadLoading_jni.h"
#import "com_mousebird_maply_QIFFrameAsset.h"

using namespace WhirlyKit;

template<> QIFFrameAssetClassInfo *QIFFrameAssetClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_nativeInit(JNIEnv *env, jclass cls)
{
    QIFFrameAssetClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_initialise(JNIEnv *env, jobject obj)
{
//    try {
//        QIFFrameAssetClassInfo *info = QIFFrameAssetClassInfo::getClassInfo();
//        PlatformInfo_Android platformInfo(env);
//        QIFFrameAsset_Android *frame = new QIFFrameAsset_Android(&platformInfo,nullptr);
//        frame->frameAssetObj = env->NewGlobalRef(obj);
//        info->setHandle(env, obj, frame);
//    } catch (...) {
//        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFFrameAsset::initialise()");
//    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFFrameAsset_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        QIFFrameAssetClassInfo *info = QIFFrameAssetClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QIFFrameAsset_AndroidRef *frame = info->getObject(env,obj);
            if (frame && *frame && (*frame)->getFrameAssetObj())
            {
                env->DeleteGlobalRef((*frame)->getFrameAssetObj());
                (*frame)->setFrameAssetObj(nullptr);
            }

            // Delete our reference to the asset, which may also be referenced from a tile asset in the loader
            delete frame;

            info->clearHandle(env, obj);
        }

    }
    MAPLY_STD_JNI_CATCH()
}

jobject MakeQIFFrameAsset(JNIEnv *env, QIFFrameAsset_AndroidRef frame)
{
    QIFFrameAssetClassInfo *classInfo = QIFFrameAssetClassInfo::getClassInfo(env,"com/mousebird/maply/QIFFrameAsset");
    if (jobject localObj = classInfo->makeWrapperObject(env, new QIFFrameAsset_AndroidRef(frame)))
    if (jobject globalObj = env->NewGlobalRef(localObj))
    {
        frame->setFrameAssetObj(globalObj);
        env->DeleteLocalRef(localObj);
        return globalObj;
    }
    return nullptr;
}

void JNICALL DisposeQIFFrameAsset(JNIEnv *env, QIFFrameAsset_Android *frame)
{
    if (frame)
    {
        std::lock_guard<std::mutex> lock(disposeMutex);
        if (jobject obj = frame->getFrameAssetObj())
        {
            frame->setFrameAssetObj(nullptr);
            env->DeleteGlobalRef(obj);
        }
    }
}

extern "C"
JNIEXPORT jint JNICALL Java_com_mousebird_maply_QIFFrameAsset_getPriority(JNIEnv *env, jobject obj)
{
    try
    {
        if (const auto frame = QIFFrameAssetClassInfo::get(env,obj))
        {
            return (*frame)->getPriority();
        }
    }
    MAPLY_STD_JNI_CATCH()
    return 0;
}
