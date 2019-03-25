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

namespace WhirlyKit
{

QIFBatchOps_Android::QIFBatchOps_Android(JNIEnv *env)
{
    // TODO: Create a corresponding batchOps object
}

QIFBatchOps_Android::~QIFBatchOps_Android()
{
}

void QIFBatchOps_Android::releaseJNI(JNIEnv *env)
{
    // TODO: Delete the batchOps object on the Java side
}

QIFFrameAsset_Android::QIFFrameAsset_Android()
{
}

QIFFrameAsset_Android::~QIFFrameAsset_Android()
{
}


void QIFFrameAsset_Android::clear(QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps,ChangeSet &changes)
{
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::clear(loader,batchOps,changes);

    // TODO: Ask Android side to cancel request (pass in batchOps)
}

bool QIFFrameAsset_Android::updateFetching(QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;

    if (!hasRequest)
        return false;
    QIFFrameAsset::updateFetching(loader, newPriority, newImportance);

    // TODO: Android side updateTileFetch

    return true;
}

void QIFFrameAsset_Android::cancelFetch(QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps)
{
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    QIFFrameAsset::cancelFetch(loader, batchOps);

    if (hasRequest) {
        // TODO: Android side cancel fetch (pass in batchOps)
    }
}

void QIFFrameAsset_Android::loadSuccess(QuadImageFrameLoader *loader,Texture *tex)
{
    QIFFrameAsset::loadSuccess(loader, tex);

    // TODO: Android side clear request
}

void QIFFrameAsset_Android::loadFailed(QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(loader);

    // TODO: Android side clear request
}

QIFTileAsset_Android::QIFTileAsset_Android(const QuadTreeNew::ImportantNode &ident)
        : QIFTileAsset(ident)
{
}

QIFTileAsset_Android::~QIFTileAsset_Android()
{
}

QIFFrameAssetRef QIFTileAsset_Android::makeFrameAsset()
{
    // TODO: Set up Android side object and register it with the TileAsset object so it doesn't get garbage collected
    return QIFFrameAssetRef(new QIFFrameAsset_Android());
}

void QIFTileAsset_Android::startFetching(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_Android *loader = (QuadImageFrameLoader_Android *)inLoader;
    QIFBatchOps_Android *batchOps = (QIFBatchOps_Android *)inBatchOps;

    state = Active;

    // TODO: Android side, set up a fetch for each individual frame with callbacks
}

QuadImageFrameLoader_Android::QuadImageFrameLoader_Android(const SamplingParams &params,int numFrames,Mode mode,JNIEnv *env)
        : QuadImageFrameLoader(params,mode), numFrames(numFrames), env(env)
{
}

QuadImageFrameLoader_Android::~QuadImageFrameLoader_Android()
{
}

QIFTileAssetRef QuadImageFrameLoader_Android::makeTileAsset(const QuadTreeNew::ImportantNode &ident)
{
    auto tileAsset = QIFTileAssetRef(new QIFTileAsset_Android(ident));
    tileAsset->setupFrames(numFrames);
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

    // TODO: Have the Android side process the batch ops
    //       Then delete the object
}

}