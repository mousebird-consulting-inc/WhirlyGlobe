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
}

void QIFFrameAsset_Android::cancelFetchJava(QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps)
{
    // TODO: Implement
}

void QIFFrameAsset_Android::clearFrameAssetJava(QuadImageFrameLoader_Android *loader)
{
    // TODO: Implement
}

void QIFFrameAsset_Android::clear(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::clear(loader,batchOps,changes);

    clearFrameAssetJava(loader);
}

bool QIFFrameAsset_Android::updateFetching(QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;

    QIFFrameAsset::updateFetching(loader, newPriority, newImportance);

    // TODO: Android side updateTileFetch

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

    cancelFetch(loader, NULL);
}

void QIFFrameAsset_Android::loadFailed(QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(loader);

    cancelFetch(loader, NULL);
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

    // TODO: Create this with a JAva side object too
    return QIFFrameAssetRef(new QIFFrameAsset_Android(loader->getEnv()));
}

void QIFTileAsset_Android::startFetching(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    state = Active;

    // TODO: Call startTileFetch on the QuadLoaderBase
}

QuadImageFrameLoader_Android::QuadImageFrameLoader_Android(const SamplingParams &params,int numFrames,Mode mode,JNIEnv *inEnv)
        : QuadImageFrameLoader(params,mode), numFrames(numFrames)
{
    env = inEnv;


//    addShaderID = env->GetMethodID(SceneRendererInfo::getClassInfo()->getClass(),"addPreBuiltShader","(Lcom/mousebird/maply/Shader;)V");
//
//    processBatchOpsMethod = env->getMethodID()
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


    // TODO: Call processBatchOps on the QuadLoaderBase
}

}