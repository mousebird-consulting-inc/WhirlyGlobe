/*
 *  MaplyQuadSamplingLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/27/18.
 *  Copyright 2011-2019 Saildrone Inc
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

#import "MaplyQuadSampler_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyBaseViewController_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplySamplingParams

- (instancetype)init
{
    self = [super init];
    self.singleLevel = true;
    self.forceMinLevel = true;

    return self;
}

- (void)setCoordSys:(MaplyCoordinateSystem *)coordSys
{
    params.coordSys = [coordSys getCoordSystem];
    params.coordBounds.reset();
    params.coordBounds.addPoint(Point2d(coordSys->ll.x,coordSys->ll.y));
    params.coordBounds.addPoint(Point2d(coordSys->ur.x,coordSys->ur.y));
}

- (int)minZoom
{
    return params.minZoom;
}

- (void)setMinZoom:(int)minZoom
{
    params.minZoom = minZoom;
}

- (int)maxZoom
{
    return params.maxZoom;
}

- (void)setMaxZoom:(int)maxZoom
{
    params.maxZoom = maxZoom;
}

- (int)maxTiles
{
    return params.maxTiles;
}

- (void)setMaxTiles:(int)maxTiles
{
    params.maxTiles = maxTiles;
}

- (double)minImportance
{
    return params.minImportance;
}

- (void)setMinImportance:(double)minImportance
{
    params.minImportance = minImportance;
    if (params.minImportanceTop == 0.0)
        params.minImportanceTop = minImportance;
}

- (void)setMinImportance:(double)minImportance forLevel:(int)level
{
    params.setImportanceLevel(minImportance,level);
}

- (double)minImportanceTop
{
    return params.minImportanceTop;
}

- (void)setMinImportanceTop:(double)minImportanceTop
{
    params.minImportanceTop = minImportanceTop;
}

- (bool)coverPoles
{
    return params.coverPoles;
}

- (void)setCoverPoles:(bool)coverPoles
{
    params.coverPoles = coverPoles;
}

- (bool)edgeMatching
{
    return params.edgeMatching;
}

- (void)setEdgeMatching:(bool)edgeMatching
{
    params.edgeMatching = edgeMatching;
}

- (int)tessX
{
    return params.tessX;
}

- (void)setTessX:(int)tessX
{
    params.tessX = tessX;
}

- (int)tessY
{
    return params.tessY;
}

- (void)setTessY:(int)tessY
{
    params.tessY = tessY;
}

- (bool)singleLevel
{
    return params.singleLevel;
}

- (void)setSingleLevel:(bool)singleLevel
{
    params.singleLevel = singleLevel;
}

- (void)setForceMinLevel:(bool)forceMinLevel
{
    params.forceMinLevel = forceMinLevel;
    if (!params.forceMinLevel)
        params.forceMinLevelHeight = 0.0;
}

- (void)setForceMinLevelHeight:(double)forceMinLevelHeight
{
    params.forceMinLevelHeight = forceMinLevelHeight;
    if (params.forceMinLevelHeight != 0.0)
        params.forceMinLevel = true;
}

- (MaplyBoundingBoxD)clipBounds
{
    MaplyBoundingBoxD bbox;
    auto mbr = params.clipBounds;
    bbox.ll.x = mbr.ll().x();  bbox.ll.y = mbr.ll().y();
    bbox.ur.x = mbr.ur().x();  bbox.ur.y = mbr.ur().y();
    
    return bbox;
}

- (void)setClipBounds:(MaplyBoundingBoxD)clipBounds
{
    params.clipBounds.reset();
    params.clipBounds.addPoint(Point2d(clipBounds.ll.x,clipBounds.ll.y));
    params.clipBounds.addPoint(Point2d(clipBounds.ur.x,clipBounds.ur.y));
}

- (bool)hasClipBounds
{
    return params.clipBounds.valid();
}

- (void)setLevelLoads:(NSArray *)levelLoads
{
    params.levelLoads.clear();
    for (NSNumber *num in levelLoads)
        params.levelLoads.push_back([num integerValue]);
}

- (bool)isEqualTo:(MaplySamplingParams *__nonnull)other
{
    return params == other->params;
}

@end

@implementation MaplyQuadSamplingLayer
{
    MaplyBaseViewController * __weak viewC;
}

- (instancetype)initWithParams:(const SamplingParams &)params
{
    self = [super init];
    _params = params;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(SceneRenderer *)inRenderer viewC:(MaplyBaseViewController *)inViewC
{
    viewC = inViewC;
    super.layerThread = inLayerThread;
    
    sampleControl.start(_params,inScene,inRenderer);
    
    _quadLayer = [[WhirlyKitQuadDisplayLayerNew alloc] initWithController:sampleControl.getDisplayControl()];

    [super.layerThread addLayer:_quadLayer];

    return true;
}

- (int)numClients
{
    return sampleControl.getNumClients();
}

- (bool)isLoading
{
    return sampleControl.builderIsLoading();
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    if (_quadLayer)
        [inLayerThread removeLayer:_quadLayer];
    
    [self performSelector:@selector(teardown) onThread:inLayerThread withObject:nil waitUntilDone:NO];
}

- (void)teardown
{
    sampleControl.stop();
    _quadLayer = nil;
}

// Add a new builder delegate to watch tile related events
- (void)addBuilderDelegate:(QuadTileBuilderDelegateRef)delegate
{
    bool notifyDelegate = false;

    @synchronized(self)
    {
        notifyDelegate = sampleControl.addBuilderDelegate(delegate);
    }
    
    NSNumber *whichDelegate = @(delegate->getId());
    MaplyQuadSamplingLayer *weakSelf = self;
    if (notifyDelegate) {
        // Let the caller finish its setup and then do the notification on the layer thread
        // Otherwise this can happen before they're ready
        dispatch_async(dispatch_get_main_queue(),
                       ^{
                           if (weakSelf)
                               [self performSelector:@selector(notifyDelegateStartup:) onThread:weakSelf.quadLayer.layerThread withObject:whichDelegate waitUntilDone:NO];
                       });
    }
}

- (void)notifyDelegateStartup:(NSNumber *)numID
{
    ChangeSet changes;
    sampleControl.notifyDelegateStartup([numID longLongValue],changes);

    [_quadLayer.layerThread addChangeRequests:changes];
}

- (void)removeBuilderDelegate:(QuadTileBuilderDelegateRef)delegate
{
    sampleControl.removeBuilderDelegate(delegate);
}

@end
