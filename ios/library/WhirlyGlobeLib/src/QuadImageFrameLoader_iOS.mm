/*
 *  QuadImageFrameLoader_iOS.mm
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

#import "QuadImageFrameLoader_iOS.h"

namespace WhirlyKit
{
    
QIFBatchOps_ios::QIFBatchOps_ios()
{
    toCancel = [[NSMutableArray alloc] init];
    toStart = [[NSMutableArray alloc] init];
}

QIFBatchOps_ios::~QIFBatchOps_ios()
{
    toCancel = nil;
    toStart = nil;
}
    
QIFFrameAsset_ios::QIFFrameAsset_ios()
: request(nil)
{
}
    
QIFFrameAsset_ios::~QIFFrameAsset_ios()
{
    request = nil;
}
    
MaplyTileFetchRequest *QIFFrameAsset_ios::setupFetch(QuadImageFrameLoader *loader,id fetchInfo,id frameInfo,int priority,double importance) {
    state = Loading;
    
    request = [[MaplyTileFetchRequest alloc] init];
    request.fetchInfo = fetchInfo;
    request.tileSource = frameInfo;
    request.priority = priority;
    request.importance = importance;
    
    return request;
}

void QIFFrameAsset_ios::clear(QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps,ChangeSet &changes) {
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;
    
    QIFFrameAsset::clear(loader,batchOps,changes);
    
    if (request) {
        [batchOps->toCancel addObject:request];
        request = nil;
    }
}

bool QIFFrameAsset_ios::updateFetching(QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_ios *loader = (QuadImageFrameLoader_ios *)inLoader;
    
    if (!request)
        return false;
    QIFFrameAsset::updateFetching(loader, newPriority, newImportance);
    
    [loader->tileFetcher updateTileFetch:request priority:priority importance:importance];
    
    return true;
}
    
void QIFFrameAsset_ios::cancelFetch(QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps)
{
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;
    
    QIFFrameAsset::cancelFetch(loader, batchOps);
    
    if (request)
        [batchOps->toCancel addObject:request];
    request = nil;
}
    
void QIFFrameAsset_ios::loadSuccess(QuadImageFrameLoader *loader,Texture *tex)
{
    QIFFrameAsset::loadSuccess(loader, tex);
    request = nil;
}

void QIFFrameAsset_ios::loadFailed(QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(loader);
    request = nil;
}
    
QIFTileAsset_ios::QIFTileAsset_ios(const QuadTreeNew::ImportantNode &ident)
: QIFTileAsset(ident)
{
}
    
QIFTileAsset_ios::~QIFTileAsset_ios()
{
}
    
QIFFrameAssetRef QIFTileAsset_ios::makeFrameAsset()
{
    return QIFFrameAssetRef(new QIFFrameAsset_ios());
}
    
void QIFTileAsset_ios::startFetching(QuadImageFrameLoader *inLoader,QIFBatchOps *inBatchOps)
{
    QuadImageFrameLoader_ios *loader = (QuadImageFrameLoader_ios *)inLoader;
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;

    state = Active;
    
    int frame = 0;
    for (NSObject<MaplyTileInfoNew> *frameInfo in loader->frameInfos) {
        MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
        QIFFrameAsset_ios *frameAsset = (QIFFrameAsset_ios *)frames[frame].get();
        MaplyTileFetchRequest *request = frameAsset->setupFetch(loader,[frameInfo fetchInfoForTile:tileID],frameInfo,0,ident.importance);
        
        request.success = ^(MaplyTileFetchRequest *request, NSData *data) {
            [loader->layer fetchRequestSuccess:request tileID:tileID frame:frame data:data];
        };
        request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
            [loader->layer fetchRequestFail:request tileID:tileID frame:frame error:error];
        };
        [batchOps->toStart addObject:request];
        
        frame++;
    }
    
}
    
QuadImageFrameLoader_ios::QuadImageFrameLoader_ios(const SamplingParams &params,NSArray<NSObject<MaplyTileInfoNew> *> *inFrameInfos)
    : QuadImageFrameLoader(params), tileFetcher(nil), layer(nil)
{
    frameInfos = inFrameInfos;
}
    
QuadImageFrameLoader_ios::~QuadImageFrameLoader_ios()
{
}

QIFTileAssetRef QuadImageFrameLoader_ios::makeTileAsset(const QuadTreeNew::ImportantNode &ident)
{
    auto tileAsset = QIFTileAssetRef(new QIFTileAsset_ios(ident));
    tileAsset->setupFrames([frameInfos count]);
    return tileAsset;
}
    
int QuadImageFrameLoader_ios::getNumFrames()
{
    return [frameInfos count];
}
    
QIFBatchOps *QuadImageFrameLoader_ios::makeBatchOps()
{
    QIFBatchOps_ios *batchOps = new QIFBatchOps_ios();
    
    return batchOps;
}
    
void QuadImageFrameLoader_ios::processBatchOps(QIFBatchOps *inBatchOps)
{
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;

    [tileFetcher cancelTileFetches:batchOps->toCancel];
    [tileFetcher startTileFetches:batchOps->toStart];
    
    batchOps->toCancel = nil;
    batchOps->toStart = nil;
}

}
