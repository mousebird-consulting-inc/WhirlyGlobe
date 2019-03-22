/*
 *  QuadImageFrameLoader_Android.h
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

#import <jni.h>
#import "QuadImageFrameLoader.h"

namespace WhirlyKit
{

class QuadImageFrameLoader_Android;

// We batch the cancels and other ops.
// This is passed around to track that
class QIFBatchOps_Android : public QIFBatchOps
{
public:
    QIFBatchOps_Android();
    virtual ~QIFBatchOps_Android();

    // Lists of toCancel and toStart
};

// Android verison of the frame asset
class QIFFrameAsset_Android : public QIFFrameAsset
{
public:
    QIFFrameAsset_Android();
    virtual ~QIFFrameAsset_Android();

    // TODO: Android side setupFetch() method

    // Clear out the texture and reset
    virtual void clear(QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes);

    // Update priority for an existing fetch request
    virtual bool updateFetching(QuadImageFrameLoader *loader,int newPriority,double newImportance);

    // Cancel an outstanding fetch
    virtual void cancelFetch(QuadImageFrameLoader *loader,QIFBatchOps *batchOps);

    // Keep track of the texture ID
    virtual void loadSuccess(QuadImageFrameLoader *loader,Texture *tex);

    // Clear out state
    virtual void loadFailed(QuadImageFrameLoader *loader);

protected:
    // TODO: Set this from the Android side
    bool hasRequest;
    // TODO: Android side needs request
};

// Android version of the tile asset keeps the platform specific stuff around
class QIFTileAsset_Android : public QIFTileAsset
{
public:
    QIFTileAsset_Android(const QuadTreeNew::ImportantNode &ident);;
    virtual ~QIFTileAsset_Android();

    // Fetch the tile frames.  Just fetch them all for now.
    virtual void startFetching(QuadImageFrameLoader *loader,QIFBatchOps *batchOps);

protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset();
};

// Android version of the QuadFrameLoader
// Mostly just builds the right objects and tweaks things here and there
class QuadImageFrameLoader_Android : public QuadImageFrameLoader
{
public:
    // Displaying a single frame
    QuadImageFrameLoader_Android(const SamplingParams &params,int numFrames,Mode mode);
    ~QuadImageFrameLoader_Android();

    // TODO: Android version points to tileFetcher and MaplyTileInfo objects

    /// Number of frames we're representing
    virtual int getNumFrames();

    // Contruct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps();

    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(QIFBatchOps *);

protected:
    // Make an Android specific tile/frame assets
    virtual QIFTileAssetRef makeTileAsset(const QuadTreeNew::ImportantNode &ident);

    int numFrames;
};

typedef std::shared_ptr<QuadImageFrameLoader_Android> QuadImageFrameLoader_AndroidRef;

}