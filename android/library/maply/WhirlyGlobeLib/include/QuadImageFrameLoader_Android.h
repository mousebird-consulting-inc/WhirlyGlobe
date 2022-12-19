/*  QuadImageFrameLoader_Android.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/22/19.
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
    QIFBatchOps_Android(PlatformInfo_Android *, jobject jobj);
    virtual ~QIFBatchOps_Android();

    // Returns a strong local reference
    jobject getBatchOpsObj(PlatformInfo_Android *) const;
    void setBatchOpsObj(PlatformInfo_Android *, jobject obj);

protected:
    // Weak pointer to the Java side of things
    jobject batchOpsObj = nullptr;
};

// Android version of the frame asset
class QIFFrameAsset_Android : public QIFFrameAsset
{
public:
    QIFFrameAsset_Android(PlatformInfo_Android *,QuadFrameInfoRef frameInfo, jobject jobj = nullptr);
    virtual ~QIFFrameAsset_Android();

    // Clear out the texture and d
    virtual void clear(PlatformThreadInfo *,QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes) override;

    // Update priority for an existing fetch request
    virtual bool updateFetching(PlatformThreadInfo *,QuadImageFrameLoader *loader,int newPriority,double newImportance) override;

    // Cancel an outstanding fetch
    virtual void cancelFetch(PlatformThreadInfo *,QuadImageFrameLoader *loader,QIFBatchOps *batchOps) override;

    // Keep track of the texture ID
    virtual void loadSuccess(PlatformThreadInfo *,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs) override;

    // Clear out state
    virtual void loadFailed(PlatformThreadInfo *,QuadImageFrameLoader *loader) override;

public:
    // Cancel the fetch (with the tile fetcher) on the Java side
    void cancelFetchJava(PlatformInfo_Android *,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps);
    // Dispose of the Java side frame asset object and cancel any fetches
    void clearFrameAssetJava(PlatformInfo_Android *,QuadImageFrameLoader_Android *loader,QIFBatchOps_Android *batchOps);
    // Clear the reference to a fetch request
    void clearRequestJava(PlatformInfo_Android *,QuadImageFrameLoader_Android *loader);

    jobject getFrameAssetObj() const { return frameAssetObj; }
    void setFrameAssetObj(jobject obj);

protected:
    jobject frameAssetObj = nullptr;
};
using QIFFrameAsset_AndroidRef = std::shared_ptr<QIFFrameAsset_Android>;

// Android version of the tile asset keeps the platform specific stuff around
class QIFTileAsset_Android : public QIFTileAsset
{
public:
    QIFTileAsset_Android(PlatformInfo_Android *,const QuadTreeNew::ImportantNode &ident);
    virtual ~QIFTileAsset_Android() = default;

    // Fetch the tile frames.  Just fetch them all for now.
    virtual void startFetching(PlatformThreadInfo *,QuadImageFrameLoader *loader,
                               const QuadFrameInfoRef &frameToLoad,QIFBatchOps *batchOps,ChangeSet &changes) override;

protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset(PlatformThreadInfo *,const QuadFrameInfoRef &frameInfo,QuadImageFrameLoader *) override;
};

// Android version of the QuadFrameLoader
// Mostly just builds the right objects and tweaks things here and there
class QuadImageFrameLoader_Android : public QuadImageFrameLoader
{
public:
    // Displaying a single frame
    QuadImageFrameLoader_Android(PlatformInfo_Android *threadInfo,const SamplingParams &params,int numFrames,Mode mode);
    virtual ~QuadImageFrameLoader_Android();

    /// Number of frames we're representing
    virtual int getNumFrames() const override { return numFrames; }

    jobject getFrameLoaderObj() const { return frameLoaderObj; }
    void setFrameLoaderObj(jobject obj) { oldFrameLoaderObj = frameLoaderObj; frameLoaderObj = obj; }

    jobject oldFrameLoaderObj = nullptr;

    virtual void teardown(PlatformThreadInfo*) override;

    // Construct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps(PlatformThreadInfo *threadInfo) override;

    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(PlatformThreadInfo *threadInfo,QIFBatchOps *) override;

    QuadTreeIdentifier getTileID(JNIEnv* env, jobject tileIDObj) const;

public:
    // Make an Android specific tile/frame assets
    virtual QIFTileAssetRef makeTileAsset(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident) override;

    // QuadLoaderBase methods
    jmethodID processBatchOpsMethod;
    jmethodID startTileFetchMethod;

    // QIFFrameAsset methods
    jmethodID cancelFrameFetchMethod;
    jmethodID updateFrameMethod;
    jmethodID clearFrameMethod;
    jmethodID clearRequestMethod;

    jobject tileIDRef;
    jfieldID tileIDX;
    jfieldID tileIDY;
    jfieldID tileIDLevel;

    jobject arrayListRef;
    jmethodID arrayListAdd;

private:
    int numFrames;
    jobject frameLoaderObj;
};

typedef std::shared_ptr<QuadImageFrameLoader_Android> QuadImageFrameLoader_AndroidRef;

}
