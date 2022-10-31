/*  QuadImageFrameLoader_iOS.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/18/19.
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

#import "QuadImageFrameLoader_iOS.h"
#import "WhirlyKitLog.h"

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
    
QIFFrameAsset_ios::QIFFrameAsset_ios(QuadFrameInfoRef frameInfo)
: QIFFrameAsset(frameInfo), request(nil)
{
}
    
QIFFrameAsset_ios::~QIFFrameAsset_ios()
{
    request = nil;
}
    
MaplyTileFetchRequest *QIFFrameAsset_ios::setupFetch(QuadImageFrameLoader *loader,MaplyTileID tileID,id fetchInfo,id frameInfo,int priority,double importance)
{
    QIFFrameAsset::setupFetch(loader);
        
    request = [[MaplyTileFetchRequest alloc] init];
    request.tileID = tileID;
    request.fetchInfo = fetchInfo;
    request.tileSource = frameInfo;
    request.priority = priority;
    request.importance = importance;
    
    return request;
}

void QIFFrameAsset_ios::clear(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps,ChangeSet &changes) {
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;
    
    QIFFrameAsset::clear(threadInfo,loader,batchOps,changes);

    if (request) {
        [batchOps->toCancel addObject:request];
        request = nil;
    }
}

bool QIFFrameAsset_ios::updateFetching(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *inLoader,int newPriority,double newImportance)
{
    QuadImageFrameLoader_ios *loader = (QuadImageFrameLoader_ios *)inLoader;
    
    if (!request)
        return false;
    QIFFrameAsset::updateFetching(threadInfo,loader, newPriority, newImportance);
    
    [loader->tileFetcher updateTileFetch:request priority:priority importance:importance];
    
    return true;
}
    
void QIFFrameAsset_ios::cancelFetch(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,QIFBatchOps *inBatchOps)
{
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;
    
    QIFFrameAsset::cancelFetch(threadInfo, loader, batchOps);
    
    if (loadReturnRef)
        loadReturnRef->cancel = true;
    
    if (request)
        [batchOps->toCancel addObject:request];
    request = nil;
}
    
void QIFFrameAsset_ios::loadSuccess(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader,const std::vector<Texture *> &texs)
{
    QIFFrameAsset::loadSuccess(threadInfo,loader, texs);
    request = nil;
}

void QIFFrameAsset_ios::loadFailed(PlatformThreadInfo *threadInfo,QuadImageFrameLoader *loader)
{
    QIFFrameAsset::loadFailed(threadInfo,loader);
    request = nil;
}
    
void QIFFrameAsset_ios::loadSkipped()
{
    QIFFrameAsset::loadSkipped();
    request = nil;
}
    
QIFTileAsset_ios::QIFTileAsset_ios(const QuadTreeNew::ImportantNode &ident)
: QIFTileAsset(ident)
{
}
    
QIFTileAsset_ios::~QIFTileAsset_ios()
{
}
    
QIFFrameAssetRef QIFTileAsset_ios::makeFrameAsset(PlatformThreadInfo *threadInfo,
                                                  const QuadFrameInfoRef &frameInfo,
                                                  QuadImageFrameLoader *loader)
{
    return std::make_shared<QIFFrameAsset_ios>(frameInfo);
}

void QIFTileAsset_ios::startFetching(PlatformThreadInfo *threadInfo,
                                     QuadImageFrameLoader *inLoader,
                                     const QuadFrameInfoRef &frameToLoad,
                                     QIFBatchOps *inBatchOps,
                                     ChangeSet &changes)
{
    QuadImageFrameLoader_ios *loader = (QuadImageFrameLoader_ios *)inLoader;
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;

    state = Active;
    
    MaplyTileID tileID;  tileID.level = ident.level;  tileID.x = ident.x;  tileID.y = ident.y;
    if (!loader->frameInfos)
    {
        // There's no data source, so we always succeed and then the interpreter does the work
        [loader->layer fetchRequestSuccess:nil tileID:tileID frame:-1 data:nil];
        return;
    }

    // Normal remote (or local) fetching case
    int whichFrame = 0;
    for (NSObject<MaplyTileInfoNew> *frameInfo in loader->frameInfos) {
        const auto frame = loader->getFrameInfo(whichFrame);
        // If we're not loading all frames, then just load the one we need
        if (frame && loader->frameShouldLoad(whichFrame) &&
            (!frameToLoad || frameToLoad->getId() == frame->getId()))
        {
            if (const auto frameAsset = std::dynamic_pointer_cast<QIFFrameAsset_ios>(findFrameFor(frame))) {
                id fetchInfo = nil;
                if (frameInfo.minZoom <= tileID.level && tileID.level <= frameInfo.maxZoom)
                    fetchInfo = [frameInfo fetchInfoForTile:tileID flipY:loader->getFlipY()];
                if (fetchInfo) {
                    if (const auto request = frameAsset->setupFetch(loader,tileID,fetchInfo,frameInfo,
                                                                    loader->calcLoadPriority(ident,frame->frameIndex),
                                                                    ident.importance)) {
                        // This means there's no data fetch.  Interpreter does all the work.
                        if ([fetchInfo isKindOfClass:[NSNull class]]) {
                            [loader->layer fetchRequestSuccess:request tileID:tileID frame:frame->frameIndex data:nil];
                        } else {
                            NSObject<QuadImageFrameLoaderLayer> * __weak layer = loader->layer;
                            request.success = ^(MaplyTileFetchRequest *request, id data) {
                                // TODO: do we need to clean anything up if layer==nil?
                                [layer fetchRequestSuccess:request tileID:tileID frame:frame->frameIndex data:data];
                            };
                            request.failure = ^(MaplyTileFetchRequest *request, NSError *error) {
                                // TODO: do we need to clean anything up if layer==nil?
                                [layer fetchRequestFail:request tileID:tileID frame:frame->frameIndex error:error];
                            };
                            [batchOps->toStart addObject:request];
                        }
                    } else {
                        NSString *errStr = [NSString stringWithFormat:@"Loader '%s' failed to set up fetch for tile %d:(%d,%d), frame %d",
                                            loader->getLabel().c_str(), tileID.level, tileID.x, tileID.y, frame->frameIndex];
                        NSError *error = [[NSError alloc] initWithDomain:@"MaplyQIFLoader" code:0
                                                                userInfo:@{NSDebugDescriptionErrorKey:errStr}];
                        [loader->layer fetchRequestFail:request tileID:tileID frame:frame->frameIndex error:error];
                        frameAsset->loadSkipped();
                    }
                } else {
                    frameAsset->loadSkipped();
                }
            }
        }
        whichFrame++;
    }
}

QuadImageFrameLoader_ios::QuadImageFrameLoader_ios(const SamplingParams &params,
                                                   NSObject<MaplyTileInfoNew> *inTileInfo,
                                                   Mode mode,
                                                   FrameLoadMode frameMode) :
    QuadImageFrameLoader(params,mode,frameMode),
    frameInfos(inTileInfo ? @[inTileInfo] : nil),
    tileFetcher(nil),
    layer(nil)
{
    setupFrames();
}

QuadImageFrameLoader_ios::QuadImageFrameLoader_ios(const SamplingParams &params,
                                                   NSArray<NSObject<MaplyTileInfoNew> *> *inFrameInfos,
                                                   Mode mode,
                                                   FrameLoadMode frameMode) :
    QuadImageFrameLoader(params,mode,frameMode),
    frameInfos(inFrameInfos),
    tileFetcher(nil),
    layer(nil)
{
    setupFrames();
}

void QuadImageFrameLoader_ios::setupFrames()
{
    for (unsigned int ii=0;ii<[frameInfos count];ii++) {
        auto frame = std::make_shared<QuadFrameInfo>();
        frame->frameIndex = ii;
        frames.push_back(frame);
    }
}

QIFTileAssetRef QuadImageFrameLoader_ios::makeTileAsset(PlatformThreadInfo *threadInfo,const QuadTreeNew::ImportantNode &ident)
{
    auto tileAsset = std::make_shared<QIFTileAsset_ios>(ident);
    tileAsset->setupFrames(threadInfo,this,[frameInfos count]);
    return tileAsset;
}
    
int QuadImageFrameLoader_ios::getNumFrames() const
{
    return [frameInfos count];
}
    
QIFBatchOps *QuadImageFrameLoader_ios::makeBatchOps(PlatformThreadInfo *threadInfo)
{
    QIFBatchOps_ios *batchOps = new QIFBatchOps_ios();
    
    return batchOps;
}
    
void QuadImageFrameLoader_ios::processBatchOps(PlatformThreadInfo *threadInfo,QIFBatchOps *inBatchOps)
{
    QIFBatchOps_ios *batchOps = (QIFBatchOps_ios *)inBatchOps;

    [tileFetcher cancelTileFetches:batchOps->toCancel];
    [tileFetcher startTileFetches:batchOps->toStart];

    for (auto tile : batchOps->deletes) {
        MaplyTileID tileID;
        tileID.level = tile.level; tileID.x = tile.x; tileID.y = tile.y;
        [layer tileUnloaded:tileID];
    }
    
    batchOps->toCancel = nil;
    batchOps->toStart = nil;
}
    
// Change the tile sources for upcoming loads
void QuadImageFrameLoader_ios::setTileInfos(NSArray<NSObject<MaplyTileInfoNew> *> *tileInfos)
{
    if ([frameInfos count] != [tileInfos count]) {
        wkLogLevel(Error,"QuadImageFrameLoader_iOS: Tried to set new tilInfos that don't match old ones.  Dropping.");
        return;
    }
    
    frameInfos = tileInfos;
}

}
