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

#import "loading/MaplyQuadPagingLoader.h"
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

- (id)initWithLoader:(MaplyQuadLoaderBase *)loader
{
    return [super initWithLoader:loader];
}

- (void)addCompObj:(MaplyComponentObject *)compObj
{
    if (compObj)
        loadReturn->compObjs.push_back(compObj->contents);
}

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
        compObjWrap->contents = std::dynamic_pointer_cast<ComponentObject_iOS>(compObj);
        [ret addObject:compObjWrap];
    }
    
    return ret;
}

@end

@implementation MaplyQuadPagingLoader

- (instancetype)initWithParams:(MaplySamplingParams *)inParams
                       tileInfo:(NSObject<MaplyTileInfoNew> *)tileInfo
                     loadInterp:(NSObject<MaplyLoaderInterpreter> *)inLoadInterp
                          viewC:(MaplyBaseViewController * )inViewC
{
    self = [super initWithViewC:inViewC];

    params = inParams->params;
    params.generateGeom = false;
    
    loadInterp = inLoadInterp;
    
    // Loader does all the work.  The Obj-C version is just a wrapper
    self->loader = QuadImageFrameLoader_iosRef(new QuadImageFrameLoader_ios(params,
                                                                            tileInfo,
                                                                            QuadImageFrameLoader::Object));

    self.flipY = true;
    self.debugMode = false;
    self->minLevel = tileInfo.minZoom;
    self->maxLevel = tileInfo.maxZoom;
    self->valid = true;
    
    // Start things out after a delay
    // This lets the caller mess with settings
    [self performSelector:@selector(delayedInit) withObject:nil afterDelay:0.0];
    
    return self;
}

- (bool)delayedInit
{
    loader->layer = self;
    
    if (!tileFetcher) {
        tileFetcher = [self.viewC addTileFetcher:MaplyQuadImageLoaderFetcherName];
    }
    loader->tileFetcher = tileFetcher;
    loader->layer = self;

    samplingLayer = [[self.viewC getRenderControl] findSamplingLayer:params forUser:self->loader];
    // Do this again in case they changed them
    loader->setSamplingParams(params);
    loader->setFlipY(self.flipY);

    [loadInterp setLoader:self];

    return true;
}

- (MaplyLoaderReturn *)makeLoaderReturn
{
    return [[MaplyObjectLoaderReturn alloc] initWithLoader:self];
}

- (void)reload
{
    [super reload];
}

@end
