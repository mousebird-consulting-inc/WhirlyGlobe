/*
 *  QIFBatchOps_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/25/19.
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
#import "com_mousebird_maply_QIFBatchOps.h"

using namespace WhirlyKit;

template<> QIFBatchOpsClassInfo *QIFBatchOpsClassInfo::classInfoObj = NULL;


JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_nativeInit
        (JNIEnv *env, jclass cls)
{
    QIFBatchOpsClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_initialise
        (JNIEnv *env, jobject obj)
{
    try {
        QIFBatchOpsClassInfo *info = QIFBatchOpsClassInfo::getClassInfo();
        PlatformInfo_Android platformInfo(env);
        QIFBatchOps_Android *batchOps = new QIFBatchOps_Android(&platformInfo);
        batchOps->batchOpsObj = obj;
        info->setHandle(env, obj, batchOps);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFBatchOps::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_dispose
        (JNIEnv *env,jobject obj)
{
    try {
        QIFBatchOpsClassInfo *info = QIFBatchOpsClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            QIFBatchOps_Android *batchOps = info->getObject(env,obj);
            // We don't actually delete the batch ops here.  They're deleted the in the same method they're created
//            if (!batchOps)
//                return;
//            delete batchOps;

            info->clearHandle(env, obj);
        }

    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in QIFBatchOps::dispose()");
    }
}

jobject MakeQIFBatchOps(JNIEnv *env,QIFBatchOps_Android *batchOps)
{
    QIFBatchOpsClassInfo *classInfo = QIFBatchOpsClassInfo::getClassInfo(env,"com/mousebird/maply/QIFBatchOps");
    jobject obj = classInfo->makeWrapperObject(env,batchOps);
    batchOps->batchOpsObj = obj;

    return obj;
}
