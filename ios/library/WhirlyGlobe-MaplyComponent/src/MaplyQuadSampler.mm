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
    _singleLevel = false;
    _maxTiles = 128;
    _minImportance = 256*256;
    _minImportanceTop = 0.0;
    _levelLoads = nil;
    _hasClipBounds = false;

    return self;
}
    
- (void)setMinImportance:(double)minImportance forLevel:(int)level
{
    if (level >= _importancePerLevel.size()) {
        _importancePerLevel.resize(level+1,-2.0);
    }
    _importancePerLevel[level] = minImportance;
}

- (bool)isClipEqualTo:(MaplySamplingParams *__nonnull)other
{
    if (_hasClipBounds != other.hasClipBounds)
        return false;
    
    MaplyBoundingBoxD otherClipBounds = other.clipBounds;
    if (_clipBounds.ll.x != otherClipBounds.ll.x ||
        _clipBounds.ll.y != otherClipBounds.ll.y ||
        _clipBounds.ur.x != otherClipBounds.ur.x ||
        _clipBounds.ur.y != otherClipBounds.ur.y)
        return false;
    
    return true;
}

- (bool)isEqualTo:(MaplySamplingParams *__nonnull)other
{
    if (_minZoom != other.minZoom || _maxZoom != other.maxZoom ||
        _coverPoles != other.coverPoles || _edgeMatching != other.edgeMatching ||
        _tessX != other.tessX || _tessY != other.tessY ||
        _minImportance != other.minImportance ||
        _minImportanceTop != other.minImportanceTop ||
        _singleLevel != other.singleLevel ||
        _maxTiles != other.maxTiles ||
        ![self isClipEqualTo:other])
        return false;
    
    return _coordSys->coordSystem->isSameAs(other.coordSys->coordSystem);
}

- (void)setClipBounds:(MaplyBoundingBoxD)clipBounds
{
    _hasClipBounds = true;
    _clipBounds = clipBounds;
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
    double importance;
    bool builderStarted;
    bool valid;
}

- (nullable instancetype)initWithParams:(MaplySamplingParams * __nonnull)params
{
    self = [super init];
    _params = params;
    _debugMode = false;
    builderStarted = false;
    valid = true;
    
    return self;
}

- (void)setMinImportance:(double)inImportance
{
    importance = inImportance;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)inRenderer viewC:(MaplyBaseViewController *)inViewC
{
    if (!valid)
        return false;
    
    viewC = inViewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    renderer = inRenderer;

    builder = [[WhirlyKitQuadTileBuilder alloc] initWithCoordSys:[_params.coordSys getCoordSystem]];
    builder.coverPoles = _params.coverPoles;
    builder.edgeMatching = _params.edgeMatching;
    builder.baseDrawPriority = 0;
    builder.drawPriorityPerLevel = 0;
    builder.singleLevel = _params.singleLevel;
    builder.delegate = self;
    quadLayer = [[WhirlyKitQuadDisplayLayerNew alloc] initWithDataSource:self loader:builder renderer:renderer];
    quadLayer.singleLevel = _params.singleLevel;
    quadLayer.levelLoads = _params.levelLoads;
    std::vector<double> importance(_params.maxZoom+1);
    for (int ii=0;ii<=_params.maxZoom;ii++) {
        double import = _params.minImportance;
        if (ii < _params.importancePerLevel.size() && _params.importancePerLevel[ii] > -2.0)
            import = _params.importancePerLevel[ii];
        importance[ii] = import;
    }
    if (_params.minImportanceTop != _params.minImportance)
        importance[_params.minZoom] = _params.minImportanceTop;
    quadLayer.minImportancePerLevel = importance;
    quadLayer.maxTiles = _params.maxTiles;
    [super.layerThread addLayer:quadLayer];

    return true;
}

- (int)numClients
{
    return builderDelegates.size();
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    valid = false;
    if (quadLayer)
        [inLayerThread removeLayer:quadLayer];
    builder = nil;
    quadLayer = nil;
    builderDelegates.clear();
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
    if (_params.hasClipBounds) {
        MaplyCoordinateD ll,ur;
        ll = _params.clipBounds.ll;
        ur = _params.clipBounds.ur;
        
        Mbr mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y));
        return mbr;
    } else
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
    // World spanning level 0 nodes sometimes have problems evaluating
    if (_params.minImportanceTop == 0.0 && ident.level == 0)
        return MAXFLOAT;
    
    double import = ScreenImportance(viewState, frameSize, viewState.eyeVec, 1, [_params.coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
    return import;
}

/// Return true if the tile is visible, false otherwise
- (bool)visibilityForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return true;
    
    return TileIsOnScreen(viewState, frameSize,  [_params.coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
}

- (void)teardown
{
    quadLayer = nil;
}

// Add a new builder delegate to watch tile related events
- (void)addBuilderDelegate:(NSObject<WhirlyKitQuadTileBuilderDelegate> * __nonnull)delegate
{
    bool notifyDelegate = false;

    @synchronized(self)
    {
        builderDelegates.push_back(delegate);
        notifyDelegate = builderStarted;
    }
    
    if (notifyDelegate) {
        // Let the caller finish its setup and then do the notification on the layer thread
        // Otherwise this can happen before they're ready
        dispatch_async(dispatch_get_main_queue(),
                       ^{
                           [self performSelector:@selector(notifyDelegateStartup:) onThread:self->quadLayer.layerThread withObject:delegate waitUntilDone:NO];
                       });
    }
}

- (void)notifyDelegateStartup:(NSObject<WhirlyKitQuadTileBuilderDelegate> * __nonnull)delegate
{
    [delegate setQuadBuilder:builder layer:quadLayer];
    
    // Pretend we just loaded everything (to the delegate)
    WhirlyKit::ChangeSet changes;
    WhirlyKit::TileBuilderDelegateInfo updates = [builder getLoadingState];
    [delegate quadBuilder:builder update:updates changes:changes];
    [quadLayer.layerThread addChangeRequests:changes];
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
    builderStarted = true;
    
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self)
    {
        delegates = builderDelegates;
    }
    
    for (auto delegate : delegates) {
        [delegate setQuadBuilder:builder layer:layer];
    }
}

- (WhirlyKit::QuadTreeNew::NodeSet)quadBuilder:(WhirlyKitQuadTileBuilder *__nonnull )builder
            loadTiles:(const WhirlyKit::QuadTreeNew::ImportantNodeSet &)loadTiles
            unloadTilesToCheck:(const WhirlyKit::QuadTreeNew::NodeSet &)unloadTiles
            targetLevel:(int)targetLevel
{
    QuadTreeNew::NodeSet toKeep;
    for (auto delegate : builderDelegates) {
        auto thisToKeep = [delegate quadBuilder:builder loadTiles:loadTiles unloadTilesToCheck:unloadTiles targetLevel:targetLevel];
        toKeep.insert(thisToKeep.begin(),thisToKeep.end());
    }
    
    return toKeep;
}

- (void)quadBuilder:(WhirlyKitQuadTileBuilder *)builder update:(const WhirlyKit::TileBuilderDelegateInfo &)updates changes:(WhirlyKit::ChangeSet &)changes
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }

    // Disable the tiles.  The delegates will instance them.
    for (auto tile : updates.loadTiles) {
        for (auto di : tile->drawInfo) {
            changes.push_back(new OnOffChangeRequest(di.drawID,false));
        }
    }

    for (auto delegate : delegates) {
        [delegate quadBuilder:builder update:updates changes:changes];
    }
    
    if (_debugMode) {
        NSLog(@"SamplingLayer quadBuilder:update changes = %d",(int)changes.size());
    }
}

- (void)quadBuilderPreSceneFlush:(WhirlyKitQuadTileBuilder *)builder
{
    std::vector<NSObject<WhirlyKitQuadTileBuilderDelegate> *> delegates;
    @synchronized(self) {
        delegates = builderDelegates;
    }
    
    for (auto delegate : delegates) {
        [delegate quadBuilderPreSceneFlush:builder];
    }
}

- (void)quadBuilderShutdown:(WhirlyKitQuadTileBuilder * _Nonnull)builder
{
    builder = nil;
}

@end
