/*
 *  MaplyQuadPagingLoader.mm
 *
 *  Created by Steve Gifford on 2/21/91.
 *  Copyright 2012-2019 mousebird consulting
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

#import "MaplyQuadPagingLoader.h"
#import "MaplyQuadSampler_private.h"
#import "MaplyQuadLoader_private.h"
#import "MaplyComponentObject_private.h"
#import "MaplyBaseViewController_private.h"
#import "QuadImageFrameLoader_iOS.h"

// Note: We're sharing the image fetcher
extern NSString * _Nonnull const MaplyQuadImageLoaderFetcherName;

using namespace WhirlyKit;

@interface MaplyQuadPagingLoader()<QuadImageFrameLoaderLayer>
@end

@implementation MaplyObjectLoaderReturn

- (void)addCompObjs:(NSArray<MaplyComponentObject *> *)compObjs
{
    for (MaplyComponentObject *compObj in compObjs)
        loadReturn->compObjs.push_back(compObj->contents);
}

- (NSArray<MaplyComponentObject *> *)getCompObjs
{
    NSMutableArray *ret = [[NSMutableArray alloc] init];
    for (auto compObj : loadReturn->compObjs) {
        MaplyComponentObject *compObjWrap = [[MaplyComponentObject alloc] init];
        compObjWrap->contents = compObj;
        [ret addObject:compObjWrap];
    }
    
    return ret;
}

@end

@implementation MaplyQuadPagingLoader
{
@public
    bool valid;
    
    // Used for initialization and then not
    // Also need to hold on to the CoordinateSystem lest it disappear
    MaplySamplingParams *params;
    
    WhirlyKit::QuadImageFrameLoader_iosRef loader;
}

- (instancetype)initWithParams:(MaplySamplingParams *)inParams
                       tileInfo:(NSObject<MaplyTileInfoNew> *)tileInfo
                     loadInterp:(NSObject<MaplyLoaderInterpreter> *)inLoadInterp
                          viewC:(MaplyBaseViewController * )inViewC
{
    self = [super initWithViewC:inViewC];

    params = inParams;
    loadInterp = inLoadInterp;
    
    // Loader does all the work.  The Obj-C version is just a wrapper
    self->loader = QuadImageFrameLoader_iosRef(new QuadImageFrameLoader_ios(params->params,
                                                                            tileInfo,
                                                                            QuadImageFrameLoader::Object));
    
    self.flipY = true;
    self.debugMode = false;
    self->minLevel = tileInfo.minZoom;
    self->maxLevel = tileInfo.maxZoom;
    self->valid = true;
    
    MaplyQuadPagingLoader * __weak weakSelf = self;

    // Start things out after a delay
    // This lets the caller mess with settings
    dispatch_async(dispatch_get_main_queue(), ^{
        [weakSelf delayedInit];
    });
    
    return self;
}

- (bool)delayedInit
{
    loader->layer = self;
    
    if (!tileFetcher) {
        tileFetcher = [self.viewC addTileFetcher:MaplyQuadImageLoaderFetcherName];
    }
    loader->tileFetcher = tileFetcher;

    samplingLayer = [self.viewC findSamplingLayer:params forUser:self->loader];
    // Do this again in case they changed them
    loader->setSamplingParams(params->params);
    loader->setFlipY(self.flipY);

    return true;
}

- (void)cleanup
{
    ChangeSet changes;
    
    loader->cleanup(changes);
    [samplingLayer.layerThread addChangeRequests:changes];
    
    loader = nil;
}

- (void)shutdown
{
    ChangeSet changes;
    
    valid = false;
    
    if (self->samplingLayer && self->samplingLayer.layerThread)
        [self performSelector:@selector(cleanup) onThread:self->samplingLayer.layerThread withObject:nil waitUntilDone:NO];
    
    [self.viewC releaseSamplingLayer:samplingLayer forUser:loader];
}

// Called on a random dispatch queue
- (void)fetchRequestSuccess:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame data:(NSData *)data;
{
    if (loader->getDebugMode())
        NSLog(@"MaplyQuadImageLoader: Got fetch back for tile %d: (%d,%d) frame %d",tileID.level,tileID.x,tileID.y,frame);
    
    // Ask the interpreter to parse it
    MaplyObjectLoaderReturn *loadData = [[MaplyObjectLoaderReturn alloc] init];
    loadData.tileID = tileID;
    loadData.frame = frame;
    [loadData addTileData:data];
    
    [self performSelector:@selector(mergeFetchRequest:) onThread:self->samplingLayer.layerThread withObject:loadData waitUntilDone:NO];
}

// Called on SamplingLayer.layerThread
- (void)fetchRequestFail:(MaplyTileFetchRequest *)request tileID:(MaplyTileID)tileID frame:(int)frame error:(NSError *)error
{
    // Note: Need to do something more here for single frame cases
    
    NSLog(@"MaplyQuadImageLoader: Failed to fetch tile %d: (%d,%d) frame %d because:\n%@",tileID.level,tileID.x,tileID.y,frame,[error localizedDescription]);
}

// Called on the SamplingLayer.LayerThread
- (void)mergeFetchRequest:(MaplyLoaderReturn *)loadReturn
{
    if (!loader)
        return;
    
    // Don't actually want this one
    if (!loader->isFrameLoading(loadReturn->loadReturn->ident,loadReturn->loadReturn->frame))
        return;
    
    // Do the parsing on another thread since it can be slow
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self->loadInterp dataForTile:loadReturn];
        
        [self performSelector:@selector(mergeLoadedTile:) onThread:self->samplingLayer.layerThread withObject:loadReturn waitUntilDone:NO];
    });
}

// Called on the SamplingLayer.LayerThread
- (void)mergeLoadedTile:(MaplyLoaderReturn *)loadReturn
{
    if (!loader)
        return;
    
    ChangeSet changes;
    loader->mergeLoadedTile(loadReturn->loadReturn.get(),changes);
    
    [samplingLayer.layerThread addChangeRequests:changes];
}


@end
