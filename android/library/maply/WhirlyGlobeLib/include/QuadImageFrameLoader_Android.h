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
#import "Maply_jni.h"

namespace WhirlyKit
{

class QuadImageFrameLoader_Android;

// We batch the cancels and other ops.
// This is passed around to track that
class QIFBatchOps_Android : public QIFBatchOps
{
public:
    QIFBatchOps_Android(PlatformInfo_Android *threadInfo);
    virtual ~QIFBatchOps_Android();

public:
    // Pointer to the Java side of things
    jobject batchOpsObj;
};

// Android verison of the frame asset
class QIFFrameAsset_Android : public QIFFrameAsset
{
public:
    QIFFrameAsset_Android(PlatformInfo_Android *threadInfo,QuadFrameInfoRef frameInfo);
    virtual ~QIFFrameAsset_Android();

    // Clear out the texture and d
    virtual void clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes) override;

    // Update priority for an existing fetch request
    virtual bool updateFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,int newPriority,double newImportance) override;

    // Cancel an outstanding fetch
    virtual void cancelFetch(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *batchOps) override;

    // Keep track of the texture ID
    virtual void loadSuccess(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs) override;

    // Clear out state
    virtual void loadFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader) override;

public:
    // Cancel the fetch (with the tile fetcher) on the Java side
    void cancelFetchJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps);
    // Dispose of the Java side frame asset object and cancel any fetches
    void clearFrameAssetJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps);
    // Clear the reference to a fetch request
    void clearRequestJava(PlatformInfo_Android *threadInfo,QuadImageFrameLoader_Android *loader);

    jobject frameAssetObj;
};

// Android version of the tile asset keeps the platform specific stuff around
class QIFTileAsset_Android : public QIFTileAsset
{
public:
    QIFTileAsset_Android(PlatformInfo_Android *threadInfo,const QuadTreeNew::ImportantNode &ident);
    virtual ~QIFTileAsset_Android();

    // Fetch the tile frames.  Just fetch them all for now.
    virtual void startFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QuadFrameInfoRef frameToLoad,QIFBatchOps *batchOps,ChangeSet &changes) override;

protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset(PlatformThreadInfo *threadInfo,QuadFrameInfoRef frameInfo,QuadImageFrameLoader *) override;
};

// Android version of the QuadFrameLoader
// Mostly just builds the right objects and tweaks things here and there
class QuadImageFrameLoader_Android : public QuadImageFrameLoader
{
public:
    // Displaying a single frame
    QuadImageFrameLoader_Android(PlatformInfo_Android *threadInfo,const SamplingParams &params,int numFrames,Mode mode,JNIEnv *env);
    ~QuadImageFrameLoader_Android();

    /// Number of frames we're representing
    virtual int getNumFrames() override;

    // Contruct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps(PlatformThreadInfo *threadInfo) override;

    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(PlatformThreadInfo *threadInfo,QIFBatchOps *) override;

public:
    // Make an Android specific tile/frame assets
    virtual QIFTileAssetRef makeTileAsset(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident) override;

    int numFrames;

    jobject frameLoaderObj;

    // QuadLoaderBase methods
    jmethodID processBatchOpsMethod;
    jmethodID startTileFetchMethod;

    // QIFFrameAsset methods
    jmethodID cancelFrameFetchMethod;
    jmethodID updateFrameMethod;
    jmethodID clearFrameMethod;
    jmethodID clearRequestMethod;
};

typedef std::shared_ptr<QuadImageFrameLoader_Android> QuadImageFrameLoader_AndroidRef;

}
