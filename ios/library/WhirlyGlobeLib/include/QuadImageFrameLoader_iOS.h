/*
 *  QuadImageFrameLoader_iOS.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/18/19.
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

#import "QuadImageFrameLoader.h"
#import "loading/MaplyTileSourceNew.h"

// These are implemented by the layer on top of the loader
@protocol QuadImageFrameLoaderLayer
// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data;
// Called on a random dispatch queue
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error;
// Also called on a randomd ispatch queue
- (void)tileUnloaded:(MaplyTileID)tileID;
@end

namespace WhirlyKit
{
    
class QuadImageFrameLoader_ios;
    
// We batch the cancels and other ops.
// This is passed around to track that
class QIFBatchOps_ios : public QIFBatchOps
{
public:
    QIFBatchOps_ios();
    virtual ~QIFBatchOps_ios();
    
public:
    NSMutableArray *toCancel;
    NSMutableArray *toStart;
};
    
// iOS version of the frame asset keeps the FetchRequest around
class QIFFrameAsset_ios : public QIFFrameAsset
{
public:
    QIFFrameAsset_ios();
    virtual ~QIFFrameAsset_ios();
    
    // Put together a fetch request and return it
    MaplyTileFetchRequest *setupFetch(QuadImageFrameLoader *loader,id fetchInfo,id frameInfo,int priority,double importance);
    
    // Clear out the texture and reset
    virtual void clear(QuadImageFrameLoader *loader,QIFBatchOps *batchOps,ChangeSet &changes);
    
    // Update priority for an existing fetch request
    virtual bool updateFetching(QuadImageFrameLoader *loader,int newPriority,double newImportance);

    // Cancel an outstanding fetch
    virtual void cancelFetch(QuadImageFrameLoader *loader,QIFBatchOps *batchOps);
    
    // Keep track of the texture ID
    virtual void loadSuccess(QuadImageFrameLoader *loader,const std::vector<Texture *> &texs);
    
    // Clear out state
    virtual void loadFailed(QuadImageFrameLoader *loader);
    
    // We're not bothering to load it, but pretend like it succeeded
    virtual void loadSkipped();

protected:
    // Returned by the TileFetcher
    MaplyTileFetchRequest *request;
};
    
// iOS version of the tile asset keeps the platform specific stuff around
class QIFTileAsset_ios : public QIFTileAsset
{
public:
    QIFTileAsset_ios(const QuadTreeNew::ImportantNode &ident);
    virtual ~QIFTileAsset_ios();
        
    // Fetch the tile frames.  Just fetch them all for now if frameToLoad is set to -1
    // Otherwise, just fetch the specified frame
    virtual void startFetching(QuadImageFrameLoader *loader,int frameToLoad,QIFBatchOps *batchOps);

protected:
    // Specialized frame asset
    virtual QIFFrameAssetRef makeFrameAsset(QuadImageFrameLoader *loader);
};
    
// iOS version of the QuadFrameLoader
// Mostly just builds the right objects and tweaks things here and there
class QuadImageFrameLoader_ios : public QuadImageFrameLoader
{
public:
    // Displaying a single frame
    QuadImageFrameLoader_ios(const SamplingParams &params,NSObject<MaplyTileInfoNew> *inTileInfo,Mode mode);
    // Displaying multiple animated frames (or one with multiple data sources)
    QuadImageFrameLoader_ios(const SamplingParams &params,NSArray<NSObject<MaplyTileInfoNew> *> *inFrameInfos,Mode mode);
    ~QuadImageFrameLoader_ios();
    
    NSObject<MaplyTileFetcher> * __weak tileFetcher;
    NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos;
    
    // Layer sitting on top of loader
    NSObject<QuadImageFrameLoaderLayer> * __weak layer;

    /// Number of frames we're representing
    virtual int getNumFrames();
    
    // Contruct a platform specific BatchOps for passing to tile fetcher
    // (we don't know about tile fetchers down here)
    virtual QIFBatchOps *makeBatchOps();
    
    // Process whatever ops we batched up during the load phase
    virtual void processBatchOps(QIFBatchOps *);
    
    // Change the tile sources for upcoming loads
    virtual void setTileInfos(NSArray<NSObject<MaplyTileInfoNew> *> *tileInfos);

protected:
    // Make an iOS specific tile/frame assets
    virtual QIFTileAssetRef makeTileAsset(const QuadTreeNew::ImportantNode &ident);
};
    
typedef std::shared_ptr<QuadImageFrameLoader_ios> QuadImageFrameLoader_iosRef;
    
}
