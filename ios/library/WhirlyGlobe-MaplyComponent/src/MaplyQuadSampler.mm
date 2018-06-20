/*
 *  MaplyQuadSamplingLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/27/18.
 *  Copyright 2011-2018 Saildrone Inc
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
#import "MaplyQuadImageLoader_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@implementation MaplySamplingParams

- (instancetype)init
{
    self = [super init];
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    _minZoom = 0;
    _maxZoom = 0;
    _coverPoles = true;
    _edgeMatching = true;
    _tessX = 10;
    _tessY = 10;
    
    return self;
}

- (bool)isEqualTo:(MaplySamplingParams *__nonnull)other
{
    if (_minZoom != other.minZoom || _maxZoom != other.maxZoom ||
        _coverPoles != other.coverPoles || _edgeMatching != other.edgeMatching ||
        _tessX != other.tessX || _tessY != other.tessY)
        return false;
    
    return _coordSys->coordSystem->isSameAs(other.coordSys->coordSystem);
}

@end

@interface MaplyQuadSamplingLayer() <WhirlyKitQuadDataStructure,WhirlyKitQuadTileBuilderDelegate>
@end

@implementation MaplyQuadSamplingLayer
{
    MaplyBaseViewController * __weak viewC;
    WhirlyKit::Scene *scene;
    WhirlyKitQuadDisplayLayerNew *quadLayer;
    WhirlyKitQuadTileBuilder *builder;
    WhirlyKitSceneRendererES * __weak renderer;
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> builderDelegates;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams * __nonnull)params
{
    self = [super init];
    _params = params;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)inRenderer viewC:(MaplyBaseViewController *)inViewC
{
    viewC = inViewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    renderer = inRenderer;
    
    builder = [[WhirlyKitQuadTileBuilder alloc] initWithCoordSys:[_params.coordSys getCoordSystem]];
    builder.coverPoles = _params.coverPoles;
    builder.edgeMatching = _params.edgeMatching;
    builder.baseDrawPriority = 0;
    builder.drawPriorityPerLevel = 0;
    builder.delegate = self;
    quadLayer = [[WhirlyKitQuadDisplayLayerNew alloc] initWithDataSource:self loader:builder renderer:renderer];
    // Note: Should get this from the loaders
    quadLayer.minImportance = 256*256;
    [super.layerThread addLayer:quadLayer];
    
    return true;
}

- (int)getNumClients
{
    return builderDelegates.size();
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
}

- (WhirlyKit::CoordSystem *)coordSystem
{
    return [_params.coordSys getCoordSystem];
}

- (WhirlyKit::Mbr)totalExtents
{
    MaplyCoordinate ll,ur;
    [_params.coordSys getBoundsLL:&ll ur:&ur];
    
    Mbr mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y));
    return mbr;
}

- (WhirlyKit::Mbr)validExtents
{
    return [self totalExtents];
}

- (int)minZoom
{
    return _params.minZoom;
}

- (int)maxZoom
{
    return _params.maxZoom;
}

- (void)newViewState:(WhirlyKitViewState *)viewState
{
}

- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return MAXFLOAT;
    
    double import = ScreenImportance(viewState, frameSize, viewState.eyeVec, 1, [_params.coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
    return import;
}

- (void)teardown
{
    quadLayer = nil;
}

// Add a new builder delegate to watch tile related events
- (void)addBuilderDelegate:(NSObject<WhirlyKitQuadTileBuilderDelegate> * __nonnull)delegate
{
    @synchronized(self)
    {
        builderDelegates.push_back(delegate);
    }
}

// Remove the given builder delegate that was watching tile related events
- (void)removeBuilderDelegate:(NSObject * __nonnull)delegate
{
    @synchronized(self)
    {
        auto it = std::find(builderDelegates.begin(), builderDelegates.end(), delegate);
        if (it != builderDelegates.end())
            builderDelegates.erase(it);
    }
}

// MARK - WhirlyKitQuadTileBuilderDelegate

- (void)setQuadBuilder:(WhirlyKitQuadTileBuilder * __nonnull)builder layer:(WhirlyKitQuadDisplayLayerNew * __nonnull)layer
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self)
    {
        delegates = builderDelegates;
    }
    
    for (auto delegate : delegates) {
        [delegate setQuadBuilder:builder layer:layer];
    }
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder loadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }
    
    // Disable the tiles.  The delegates will instance them.
    for (auto tile : tiles) {
        for (auto di : tile->drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    }

    for (auto delegate : delegates) {
        [delegate quadBuilder:builder loadTiles:tiles changes:changes];
    }
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder enableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }

    for (auto delegate : delegates) {
        [delegate quadBuilder:builder enableTiles:tiles changes:changes];
    }
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder disableTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }

    for (auto delegate : delegates) {
        [delegate quadBuilder:builder disableTiles:tiles changes:changes];
    }
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull)builder unLoadTiles:(const std::vector<WhirlyKit::LoadedTileNewRef> &)tiles changes:(WhirlyKit::ChangeSet &)changes
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }

    for (auto delegate : delegates) {
        [delegate quadBuilder:builder unLoadTiles:tiles changes:changes];
    }
}

@end
