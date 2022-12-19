/*  QIFBatchOps_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by sjg on 3/25/19.
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
#import "com_mousebird_maply_QIFBatchOps.h"
#import "WhirlyKitLog.h"

using namespace WhirlyKit;

template<> QIFBatchOpsClassInfo *QIFBatchOpsClassInfo::classInfoObj = nullptr;

static jclass tileIDRef = nullptr;
static jmethodID tileIDCtor = nullptr;

static jclass initBatchOps(JNIEnv *env)
{
    if (tileIDRef)
    {
        if (jclass tileIDClass = (jclass)env->NewLocalRef(tileIDRef))
        {
            return tileIDClass;
        }
    }
    if (jclass tileIDClass = env->FindClass("com/mousebird/maply/TileID"))
    {
        tileIDRef = (jclass)env->NewWeakGlobalRef(tileIDClass);
        tileIDCtor = env->GetMethodID(tileIDClass,"<init>","(III)V");
        return tileIDClass;
    }
    return nullptr;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_nativeInit
        (JNIEnv *env, jclass cls)
{
    QIFBatchOpsClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_initialise
        (JNIEnv *env, jobject obj)
{
    // QIFBatchOps objects are owned by QIFBatchOps_Android and should not be created directly.
//    try
//    {
//        QIFBatchOpsClassInfo *info = QIFBatchOpsClassInfo::getClassInfo();
//        PlatformInfo_Android platformInfo(env);
//        auto *batchOps = new QIFBatchOps_Android(&platformInfo, obj);
//        wkLog("Creating QIFBatchOps %llx => %x", batchOps, obj);
//        info->setHandle(env, obj, batchOps);
//    }
//    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_QIFBatchOps_dispose
        (JNIEnv *env,jobject obj)
{
    try
    {
        QIFBatchOpsClassInfo *info = QIFBatchOpsClassInfo::getClassInfo();

        std::lock_guard<std::mutex> lock(disposeMutex);
        //QIFBatchOps_Android *batchOps = info->getObject(env,obj);
        // We don't actually delete the batch ops here.  They're deleted the in the same method they're created
        //delete batchOps;

        info->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_QIFBatchOps_getDeletes
  (JNIEnv *env, jobject obj)
{
    try
    {
        if (jclass tileIDCls = initBatchOps(env))
        if (auto batchOps = QIFBatchOpsClassInfo::get(env, obj))
        {
            std::vector<jobject> objs;
            objs.reserve(batchOps->deletes.size());

            for (const auto &item : batchOps->deletes)
            {
                if (jobject id = env->NewObject(tileIDCls, tileIDCtor, item.x, item.y, item.level))
                {
                    objs.push_back(id);
                }
            }
            return BuildObjectArray(env,tileIDCls,objs);
        }
    }
    MAPLY_STD_JNI_CATCH()
    return nullptr;
}

jobject MakeQIFBatchOps(JNIEnv *env, QIFBatchOps_Android *batchOps)
{
    if (auto *classInfo = QIFBatchOpsClassInfo::getClassInfo(env,"com/mousebird/maply/QIFBatchOps"))
    if (jobject obj = classInfo->makeWrapperObject(env,batchOps))
    {
        PlatformInfo_Android platform(env);
        batchOps->setBatchOpsObj(&platform, obj);
        return obj;
    }
    return nullptr;
}
