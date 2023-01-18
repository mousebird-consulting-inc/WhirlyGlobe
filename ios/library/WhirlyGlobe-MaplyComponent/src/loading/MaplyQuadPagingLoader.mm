/*  MaplyQuadPagingLoader.mm
 *
 *  Created by Steve Gifford on 2/21/91.
 *  Copyright 2012-2022 mousebird consulting
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
    for (auto compObj : loadReturn->compObjs)
    {
        if (MaplyComponentObject *compObjWrap = [[MaplyComponentObject alloc] init])
        {
            compObjWrap->contents = std::dynamic_pointer_cast<ComponentObject_iOS>(compObj);
            [ret addObject:compObjWrap];
        }
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
    if (!(self = [super initWithViewC:inViewC]))
    {
        return nil;
    }

    params = inParams->params;
    params.generateGeom = false;
    
    loadInterp = inLoadInterp;

    // Loader does all the work.  The Obj-C version is just a wrapper
    const auto mode = QuadImageFrameLoader::Object;
    self->loader = std::make_shared<QuadImageFrameLoader_ios>(params, tileInfo, mode);

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

- (bool)tryDelayedInit
{
    if (![super delayedInit])
    {
        return false;
    }

    const auto __strong vc = self.viewC;
    const auto ldr = loader;

    if (vc && !tileFetcher)
    {
        tileFetcher = [vc addTileFetcher:MaplyQuadImageLoaderFetcherName];
    }

    if (ldr)
    {
        ldr->tileFetcher = tileFetcher;
        ldr->layer = self;
    }

    samplingLayer = [[vc getRenderControl] findSamplingLayer:params forUser:ldr];

    // Do this again in case they changed them
    if (ldr)
    {
        ldr->setSamplingParams(params);
        ldr->setFlipY(self.flipY);
    }

    [loadInterp setLoader:self];

    [super postDelayedInit];

    return true;
}

- (bool)delayedInit
{
    const auto __strong vc = self.viewC;
    try
    {
        return [self tryDelayedInit];
    }
    catch (const std::exception &ex)
    {
        NSLog(@"Exception in MaplyQuadPagingLoader.delayedInit: %s", ex.what());
        [vc report:@"QuadPagingLoader-DelayedInit"
             exception:[[NSException alloc] initWithName:@"STL Exception"
                                                  reason:[NSString stringWithUTF8String:ex.what()]
                                                userInfo:nil]];
    }
    catch (NSException *ex)
    {
        NSLog(@"Exception in MaplyQuadPagingLoader.delayedInit: %@", ex.description);
        [vc report:@"QuadPagingLoader-DelayedInit" exception:ex];
    }
    catch (...)
    {
        NSLog(@"Exception in MaplyQuadPagingLoader.delayedInit");
        [vc report:@"QuadPagingLoader-DelayedInit"
             exception:[[NSException alloc] initWithName:@"C++ Exception"
                                                  reason:@"Unknown"
                                                userInfo:nil]];
    }
    return false;
}

- (MaplyLoaderReturn *)makeLoaderReturn
{
    return [[MaplyObjectLoaderReturn alloc] initWithLoader:self];
}

- (void)reload
{
    // Called before it's set up.  Dude.  Calm down.
    if (!samplingLayer)
        return;
    
    [super reload];
}

- (void)reloadArea:(MaplyBoundingBox)bound
{
    [super reloadArea:bound];
}

- (void)reloadAreas:(NSArray*)bounds
{
    [super reloadAreas:bounds];
}

@end
