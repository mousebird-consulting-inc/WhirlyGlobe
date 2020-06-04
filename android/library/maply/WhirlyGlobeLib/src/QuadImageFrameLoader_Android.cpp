/*
 *  QuadImageFrameLoader_Android.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
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

QIFFrameAsset_Android::QIFFrameAsset_Android(PlatformInfo_Android *threadInfo,QuadFrameInfoRef frameInfo)
: QIFFrameAsset(frameInfo)
{
}

QIFFrameAsset_Android::~QIFFrameAsset_Android()
{
    if (frameAssetObj)
        wkLog("Failed to clean up QIFFrameAsset on Java side");
}

void QIFFrameAsset_Android::cancelFetchJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    threadInfo->env->CallVoidMethod(frameAssetObj,loader->cancelFrameFetchMethod,loader->frameLoaderObj,batchOps->batchOpsObj);
}

void QIFFrameAsset_Android::clearFrameAssetJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    threadInfo->env->CallVoidMethod(frameAssetObj,loader->clearFrameMethod,loader->frameLoaderObj,batchOps->batchOpsObj);
}

void QIFFrameAsset_Android::clearRequestJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader)
{
    threadInfo->env->CallVoidMethod(frameAssetObj,loader->clearRequestMethod);
}

void QIFFrameAsset_Android::clear(PlatformInfo_Android *threadInfo,QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::clear(threadInfo,loader,batchOps,changes);

    clearFrameAssetJava(threadInfo,loader,batchOps);
}

bool QIFFrameAsset_Android::updateFetching(PlatformInfo_Android *threadInfo,QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;

    QIFFrameAsset::updateFetching(loader, newPriority, newImportance);

    threadInfo->env->CallVoidMethod(frameAssetObj,loader->updateFrameMethod,loader->frameLoaderObj,newPriority,newImportance);

    return true;
}

void QIFFrameAsset_Android::cancelFetch(PlatformInfo_Android *threadInfo,QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::cancelFetch(loader, batchOps);
}

void QIFFrameAsset_Android::loadSuccess(PlatformInfo_Android *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs)
{
    QIFFrameAsset::loadSuccess(loader, texs);

    clearRequestJava(threadInfo,(QuadImageFrameLoader_Android *)loader);
}

void QIFFrameAsset_Android::loadFailed(PlatformInfo_Android *threadInfo,QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(loader);

    clearRequestJava(threadInfo,(QuadImageFrameLoader_Android *)loader);
}

QIFTileAsset_Android::QIFTileAsset_Android(PlatformInfo_Android *threadInfo,const QuadTreeNew::ImportantNode &ident)
        : QIFTileAsset(ident)
{
}

QIFTileAsset_Android::~QIFTileAsset_Android()
{
}

QIFFrameAssetRef QIFTileAsset_Android::makeFrameAsset(PlatformThreadInfo *inThreadInfo,QuadFrameInfoRef frameInfo,QuadImageFrameLoader *inLoader)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;

    QIFFrameAsset_Android *frame = new QIFFrameAsset_Android((PlatformInfo_Android *)threadInfo,frameInfo);
    MakeQIFFrameAsset(threadInfo->env,frame);

    return QIFFrameAssetRef(frame);
}

void QIFTileAsset_Android::startFetching(PlatformThreadInfo *inThreadInfo,QuadImageFrameLoader *inLoader,int frameToLoad,QIFBatchOps *inBatchOps)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    state = Active;

    std::vector<jobject> objVec(frames.size(),NULL);
    for (unsigned int ii=0;ii<frames.size();ii++)
    {
        if (frameToLoad == -1 || frameToLoad == ii) {
            QIFFrameAsset_Android *frame = (QIFFrameAsset_Android *) (frames[ii].get());
            frame->setupFetch(loader);
            int priority = loader->calcLoadPriority(ident,ii);
            frame->updateFetching(threadInfo,loader,priority,ident.importance);
            objVec[ii] = frame->frameAssetObj;
        }
    }

    // Give the Java side a list of frames to start fetching
    jobjectArray frameArray = BuildObjectArray(threadInfo->env,QIFFrameAssetClassInfo::getClassInfo(threadInfo->env,"com/mousebird/maply/QIFFrameAsset")->getClass(),objVec);
    threadInfo->env->CallVoidMethod(loader->frameLoaderObj,loader->startTileFetchMethod,
            batchOps->batchOpsObj,frameArray,
            ident.x,ident.y,ident.level,0,ident.importance);

    threadInfo->env->DeleteLocalRef(frameArray);
}

QuadImageFrameLoader_Android::QuadImageFrameLoader_Android(PlatformInfo_Android *threadInfo,const SamplingParams &params,int numFrames,Mode mode,JNIEnv *inEnv)
        : QuadImageFrameLoader(params,mode), numFrames(numFrames), frameLoaderObj(NULL)
{
    jclass thisClass = QuadImageFrameLoaderClassInfo::getClassInfo()->getClass();
    processBatchOpsMethod = threadInfo->env->GetMethodID(thisClass,"processBatchOps","(Lcom/mousebird/maply/QIFBatchOps;)V");
    startTileFetchMethod = threadInfo->env->GetMethodID(thisClass,"startTileFetch","(Lcom/mousebird/maply/QIFBatchOps;[Lcom/mousebird/maply/QIFFrameAsset;IIIID)V");

    jclass frameClass = QIFFrameAssetClassInfo::getClassInfo(threadInfo->env,"com/mousebird/maply/QIFFrameAsset")->getClass();
    cancelFrameFetchMethod = threadInfo->env->GetMethodID(frameClass,"cancelFetch","(Lcom/mousebird/maply/QIFBatchOps;)V");
    updateFrameMethod = threadInfo->env->GetMethodID(frameClass,"updateFetch","(Lcom/mousebird/maply/QuadLoaderBase;ID)V");
    clearFrameMethod = threadInfo->env->GetMethodID(frameClass,"clearFrameAsset","(Lcom/mousebird/maply/QuadLoaderBase;Lcom/mousebird/maply/QIFBatchOps;)V");
    clearRequestMethod = threadInfo->env->GetMethodID(frameClass, "clearRequest","()V");

    frames.resize(numFrames);
}

QuadImageFrameLoader_Android::~QuadImageFrameLoader_Android()
{

}

QIFTileAssetRef QuadImageFrameLoader_Android::makeTileAsset(PlatformThreadInfo *inThreadInfo,const QuadTreeNew::ImportantNode &ident)
{
    PlatformInfo_Android *threadInfo = (PlatformInfo_Android *)inThreadInfo;
    auto tileAsset = QIFTileAssetRef(new QIFTileAsset_Android(threadInfo,ident));
    tileAsset->setupFrames(threadInfo,this,numFrames);
    return tileAsset;
}

int QuadImageFrameLoader_Android::getNumFrames()
{
    return numFrames;
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

    threadInfo->env->CallVoidMethod(frameLoaderObj,processBatchOpsMethod,batchOps->batchOpsObj);
}

}