/*  QuadImageFrameLoader_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
 *  Copyright 2011-2021 mousebird consulting
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

#import "QuadImageFrameLoader_Android.h"
#import "QuadLoading_jni.h"

namespace WhirlyKit
{

QIFBatchOps_Android::QIFBatchOps_Android(PlatformInfo_Android *threadInfo)
{
    MakeQIFBatchOps(threadInfo->env,this);
}

QIFBatchOps_Android::~QIFBatchOps_Android()
{
}

QIFFrameAsset_Android::QIFFrameAsset_Android(PlatformInfo_Android *,QuadFrameInfoRef frameInfo)
: QIFFrameAsset(frameInfo)
{
}

QIFFrameAsset_Android::~QIFFrameAsset_Android()
{
    if (frameAssetObj) {
        wkLogLevel(Warn,"Failed to clean up QIFFrameAsset on Java side");
    }
}

void QIFFrameAsset_Android::cancelFetchJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    threadInfo->env->CallVoidMethod(frameAssetObj,loader->cancelFrameFetchMethod,batchOps->batchOpsObj);
}

void QIFFrameAsset_Android::clearFrameAssetJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    if (const auto obj = loader->getFrameLoaderObj())
    {
        threadInfo->env->CallVoidMethod(frameAssetObj,loader->clearFrameMethod,obj,batchOps->batchOpsObj);
    }
}

void QIFFrameAsset_Android::clearRequestJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader)
{
    threadInfo->env->CallVoidMethod(frameAssetObj,loader->clearRequestMethod);
}

void QIFFrameAsset_Android::clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::clear(threadInfo,loader,batchOps,changes);

    clearFrameAssetJava((PlatformInfo_Android *)threadInfo,loader,batchOps);
}

bool QIFFrameAsset_Android::updateFetching(PlatformThreadInfo *inThreadInfo,QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;

    QIFFrameAsset::updateFetching(threadInfo, loader, newPriority, newImportance);

    if (const auto obj = loader->getFrameLoaderObj())
    {
        threadInfo->env->CallVoidMethod(frameAssetObj,loader->updateFrameMethod,obj,newPriority,newImportance);
    }

    return true;
}

void QIFFrameAsset_Android::cancelFetch(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    auto *loader = (QuadImageFrameLoader_Android *)inLoader;
    auto *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::cancelFetch(threadInfo,loader,batchOps);

    if (loadReturnRef) {
        loadReturnRef->cancel = true;
    }

    cancelFetchJava((PlatformInfo_Android*)threadInfo,loader,batchOps);
}

void QIFFrameAsset_Android::loadSuccess(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs)
{
    QIFFrameAsset::loadSuccess(threadInfo, loader, texs);

    clearRequestJava((PlatformInfo_Android *) threadInfo,(QuadImageFrameLoader_Android *)loader);
}

void QIFFrameAsset_Android::loadFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(threadInfo,loader);

    clearRequestJava((PlatformInfo_Android *) threadInfo,(QuadImageFrameLoader_Android *)loader);
}

QIFTileAsset_Android::QIFTileAsset_Android(PlatformInfo_Android *,const QuadTreeNew::ImportantNode &ident)
        : QIFTileAsset(ident)
{
}

QIFTileAsset_Android::~QIFTileAsset_Android()
{
}

QIFFrameAssetRef QIFTileAsset_Android::makeFrameAsset(PlatformThreadInfo *inThreadInfo,const QuadFrameInfoRef &frameInfo,QuadImageFrameLoader *)
{
    //const auto loader = (QuadImageFrameLoader_Android *)inLoader;
    const auto threadInfo = (PlatformInfo_Android *)inThreadInfo;

    auto frame = std::make_shared<QIFFrameAsset_Android>(threadInfo,frameInfo);
    MakeQIFFrameAsset(threadInfo->env,frame.get());

    return frame;
}

void QIFTileAsset_Android::startFetching(PlatformThreadInfo *inThreadInfo,QuadImageFrameLoader *inLoader,
                                         const QuadFrameInfoRef &frameToLoad,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    state = Active;

    std::vector<jobject> objVec(frames.size(),nullptr);
    for (unsigned int ii=0;ii<frames.size();ii++)
    {
        if (!frameToLoad || frameToLoad->frameIndex == -1 || frameToLoad->frameIndex == ii) {
            QIFFrameAsset_Android *frame = (QIFFrameAsset_Android *) (frames[ii].get());
            frame->setupFetch(loader);
            const int priority = loader->calcLoadPriority(ident,ii);
            frame->updateFetching(threadInfo,loader,priority,ident.importance);
            objVec[ii] = frame->frameAssetObj;
        }
    }

    // Give the Java side a list of frames to start fetching
    jobjectArray frameArray = BuildObjectArray(threadInfo->env,QIFFrameAssetClassInfo::getClassInfo(threadInfo->env,"com/mousebird/maply/QIFFrameAsset")->getClass(),objVec);

    if (const auto obj = loader->getFrameLoaderObj())
    {
        threadInfo->env->CallVoidMethod(obj,loader->startTileFetchMethod,
                                        batchOps->batchOpsObj,frameArray,
                                        ident.x,ident.y,ident.level,0,ident.importance);
    }

    threadInfo->env->DeleteLocalRef(frameArray);
}

QuadImageFrameLoader_Android::QuadImageFrameLoader_Android(PlatformInfo_Android *threadInfo,
                                                           const SamplingParams &params,
                                                           int numFrames,Mode mode) :
   QuadImageFrameLoader(params,mode),
   numFrames(numFrames),
   frameLoaderObj(nullptr)
{
    const auto env = threadInfo->env;

    jclass thisClass = QuadImageFrameLoaderClassInfo::getClassInfo()->getClass();
    processBatchOpsMethod = env->GetMethodID(thisClass,"processBatchOps","(Lcom/mousebird/maply/QIFBatchOps;)V");
    startTileFetchMethod = env->GetMethodID(thisClass,"startTileFetch","(Lcom/mousebird/maply/QIFBatchOps;[Lcom/mousebird/maply/QIFFrameAsset;IIIID)V");

    jclass frameClass = QIFFrameAssetClassInfo::getClassInfo(env,"com/mousebird/maply/QIFFrameAsset")->getClass();
    cancelFrameFetchMethod = env->GetMethodID(frameClass,"cancelFetch","(Lcom/mousebird/maply/QIFBatchOps;)V");
    updateFrameMethod = env->GetMethodID(frameClass,"updateFetch","(Lcom/mousebird/maply/QuadLoaderBase;ID)V");
    clearFrameMethod = env->GetMethodID(frameClass,"clearFrameAsset","(Lcom/mousebird/maply/QuadLoaderBase;Lcom/mousebird/maply/QIFBatchOps;)V");
    clearRequestMethod = env->GetMethodID(frameClass, "clearRequest","()V");

    if (jclass tileIDClass = env->FindClass("com/mousebird/maply/TileID"))
    {
        tileIDRef = env->NewGlobalRef(tileIDClass); // make sure the class isn't unloaded
        tileIDX = env->GetFieldID(tileIDClass,"x","I");
        tileIDY = env->GetFieldID(tileIDClass,"y","I");
        tileIDLevel = env->GetFieldID(tileIDClass,"level","I");
    }

    if (jclass arrayListClass = env->FindClass("java/util/ArrayList"))
    {
        arrayListRef = env->NewGlobalRef(arrayListClass);
        arrayListAdd = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");
    }

    frames.resize(numFrames);
    // TODO: Shouldn't we be creating Android side objects for this?
    for (unsigned int ii=0;ii!=numFrames;ii++) {
        auto frameInfo = std::make_shared<QuadFrameInfo>();
        frameInfo->frameIndex = ii;
        frames[ii] = frameInfo;
    }
}

QuadImageFrameLoader_Android::~QuadImageFrameLoader_Android()
{
    // global ref should have been released by now
    if (frameLoaderObj)
    {
        wkLogLevel(Warn, "QuadImageFrameLoader_Android not cleaned up");
    }
}

void QuadImageFrameLoader_Android::teardown(PlatformThreadInfo *threadInfo)
{
    const auto env = ((PlatformInfo_Android*)threadInfo)->env;
    if (frameLoaderObj)
    {
        env->DeleteGlobalRef(frameLoaderObj);
        frameLoaderObj = nullptr;
    }
    if (tileIDRef)
    {
        env->DeleteGlobalRef(tileIDRef);
        tileIDRef = nullptr;
    }
    if (arrayListRef)
    {
        env->DeleteGlobalRef(arrayListRef);
        arrayListRef = nullptr;
    }
}

QuadTreeIdentifier QuadImageFrameLoader_Android::getTileID(JNIEnv* env, jobject tileIDObj) const
{
    return (tileIDObj && tileIDRef) ? QuadTreeIdentifier{
        env->GetIntField(tileIDObj, tileIDX),
        env->GetIntField(tileIDObj, tileIDY),
        env->GetIntField(tileIDObj, tileIDLevel)
    } : QuadTreeIdentifier{ -1, -1, -1 };
}

QIFTileAssetRef QuadImageFrameLoader_Android::makeTileAsset(PlatformThreadInfo *inThreadInfo,const QuadTreeNew::ImportantNode &ident)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    auto tileAsset = std::make_shared<QIFTileAsset_Android>(threadInfo,ident);
    tileAsset->setupFrames(threadInfo,this,numFrames);
    return tileAsset;
}

QIFBatchOps *QuadImageFrameLoader_Android::makeBatchOps(PlatformThreadInfo *inThreadInfo)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    QIFBatchOps_Android *batchOps = new QIFBatchOps_Android(threadInfo);

    return batchOps;
}

void QuadImageFrameLoader_Android::processBatchOps(PlatformThreadInfo *inThreadInfo,QIFBatchOps *inBatchOps)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    if (frameLoaderObj)
    {
        threadInfo->env->CallVoidMethod(frameLoaderObj,processBatchOpsMethod,batchOps->batchOpsObj);
    }
}

}