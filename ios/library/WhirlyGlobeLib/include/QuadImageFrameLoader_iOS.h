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
#import "MaplyTileSourceNew.h"
#import "QuadImageFrameLoader_iOS.h"

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
    virtual void loadSuccess(QuadImageFrameLoader *loader,Texture *tex);
    
    // Clear out state
    virtual void loadFailed(QuadImageFrameLoader *loader);

protected:
    // Returned by the TileFetcher
    MaplyTileFetchRequest *request;
};
    
// iOS version of the tile asset keeps the platform specific stuff around
class QIFTileAsset_ios : public QIFTileAsset
{
public:
    QIFTileAsset_ios(const QuadTreeNew::ImportantNode &ident, int numFrames);;
    virtual ~QIFTileAsset_ios();
        
    // Fetch the tile frames.  Just fetch them all for now.
    virtual void startFetching(QuadImageFrameLoader *loader,QIFBatchOps *batchOps);

protected:
};
    
// iOS version of the QuadFrameLoader
// Mostly just builds the right objects and tweaks things here and there
class QuadImageFrameLoader_ios : public QuadImageFrameLoader
{
public:
    NSObject<MaplyTileFetcher> * __weak tileFetcher;
    NSArray<NSObject<MaplyTileInfoNew> *> *frameInfos;
    
protected:
    // Make an iOS specific tile asset
    virtual QIFTileAssetRef makeTileAsset();
};
    
}
