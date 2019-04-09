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

QIFBatchOps_Android::QIFBatchOps_Android(JNIEnv *env)
{
    MakeQIFBatchOps(env,this);
}

QIFBatchOps_Android::~QIFBatchOps_Android()
{
}

QIFFrameAsset_Android::QIFFrameAsset_Android(JNIEnv *env)
{
}

QIFFrameAsset_Android::~QIFFrameAsset_Android()
{
    if (frameAssetObj)
        wkLog("Failed to clean up QIFFrameAsset on Java side");
}

void QIFFrameAsset_Android::cancelFetchJava(QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    loader->getEnv()->CallVoidMethod(frameAssetObj,loader->cancelFrameFetchMethod,loader->frameLoaderObj,batchOps->batchOpsObj);
}

void QIFFrameAsset_Android::clearFrameAssetJava(QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    loader->getEnv()->CallVoidMethod(frameAssetObj,loader->clearFrameMethod,loader->frameLoaderObj,batchOps->batchOpsObj);
}

void QIFFrameAsset_Android::clearRequestJava(QuadImageFrameLoader_Android *loader)
{
    loader->getEnv()->CallVoidMethod(frameAssetObj,loader->clearRequestMethod);
}

void QIFFrameAsset_Android::clear(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::clear(loader,batchOps,changes);

    clearFrameAssetJava(loader,batchOps);
}

bool QIFFrameAsset_Android::updateFetching(QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;

    QIFFrameAsset::updateFetching(loader, newPriority, newImportance);

    loader->getEnv()->CallVoidMethod(frameAssetObj,loader->updateFrameMethod,loader->frameLoaderObj,newPriority,newImportance);

    return true;
}

void QIFFrameAsset_Android::cancelFetch(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::cancelFetch(loader, batchOps);
}

void QIFFrameAsset_Android::loadSuccess(QuadImageFrameLoader *loader,Texture *tex)
{
    QIFFrameAsset::loadSuccess(loader, tex);

    clearRequestJava((QuadImageFrameLoader_Android *)loader);
}

void QIFFrameAsset_Android::loadFailed(QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(loader);

    clearRequestJava((QuadImageFrameLoader_Android *)loader);
}

QIFTileAsset_Android::QIFTileAsset_Android(const QuadTreeNew::ImportantNode &ident)
        : QIFTileAsset(ident)
{
}

QIFTileAsset_Android::~QIFTileAsset_Android()
{
}

QIFFrameAssetRef QIFTileAsset_Android::makeFrameAsset(QuadImageFrameLoader *inLoader)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;

    QIFFrameAsset_Android *frame = new QIFFrameAsset_Android(loader->getEnv());
    MakeQIFFrameAsset(loader->getEnv(),frame);

    return QIFFrameAssetRef(frame);
}

void QIFTileAsset_Android::startFetching(QuadImageFrameLoader *inLoader,int frameToLoad,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    state = Active;

    std::vector<jobject> objVec(frames.size(),NULL);
    for (unsigned int ii=0;ii<frames.size();ii++)
    {
        if (frameToLoad == -1 || frameToLoad == ii) {
            QIFFrameAsset_Android *frame = (QIFFrameAsset_Android *) (frames[ii].get());
            frame->setupFetch(loader);
            objVec[ii] = frame->frameAssetObj;
        }
    }

    // Give the Java side a list of frames to start fetching
    jobjectArray frameArray = BuildObjectArray(loader->getEnv(),QIFFrameAssetClassInfo::getClassInfo(loader->getEnv(),"com/mousebird/maply/QIFFrameAsset")->getClass(),objVec);
    loader->getEnv()->CallVoidMethod(loader->frameLoaderObj,loader->startTileFetchMethod,
            batchOps->batchOpsObj,frameArray,
            ident.x,ident.y,ident.level,0,ident.importance);

    loader->getEnv()->DeleteLocalRef(frameArray);
}

QuadImageFrameLoader_Android::QuadImageFrameLoader_Android(const SamplingParams &params,int numFrames,Mode mode,JNIEnv *inEnv)
        : QuadImageFrameLoader(params,mode), numFrames(numFrames), frameLoaderObj(NULL)
{
    env = inEnv;

    jclass thisClass = QuadImageFrameLoaderClassInfo::getClassInfo()->getClass();
    processBatchOpsMethod = env->GetMethodID(thisClass,"processBatchOps","(Lcom/mousebird/maply/QIFBatchOps;)V");
    startTileFetchMethod = env->GetMethodID(thisClass,"startTileFetch","(Lcom/mousebird/maply/QIFBatchOps;[Lcom/mousebird/maply/QIFFrameAsset;IIIID)V");

    jclass frameClass = QIFFrameAssetClassInfo::getClassInfo(env,"com/mousebird/maply/QIFFrameAsset")->getClass();
    cancelFrameFetchMethod = env->GetMethodID(frameClass,"cancelFetch","(Lcom/mousebird/maply/QIFBatchOps;)V");
    updateFrameMethod = env->GetMethodID(frameClass,"updateFetch","(Lcom/mousebird/maply/QuadLoaderBase;ID)V");
    clearFrameMethod = env->GetMethodID(frameClass,"clearFrameAsset","(Lcom/mousebird/maply/QuadLoaderBase;Lcom/mousebird/maply/QIFBatchOps;)V");
    clearRequestMethod = env->GetMethodID(frameClass, "clearRequest","()V");
}

QuadImageFrameLoader_Android::~QuadImageFrameLoader_Android()
{

}

QIFTileAssetRef QuadImageFrameLoader_Android::makeTileAsset(const QuadTreeNew::ImportantNode &ident)
{
    auto tileAsset = QIFTileAssetRef(new QIFTileAsset_Android(ident));
    tileAsset->setupFrames(this,numFrames);
    return tileAsset;
}

int QuadImageFrameLoader_Android::getNumFrames()
{
    return numFrames;
}

QIFBatchOps *QuadImageFrameLoader_Android::makeBatchOps()
{
    QIFBatchOps_Android *batchOps = new QIFBatchOps_Android(env);

    return batchOps;
}

void QuadImageFrameLoader_Android::processBatchOps(QIFBatchOps *inBatchOps)
{
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    env->CallVoidMethod(frameLoaderObj,processBatchOpsMethod,batchOps->batchOpsObj);
}

}