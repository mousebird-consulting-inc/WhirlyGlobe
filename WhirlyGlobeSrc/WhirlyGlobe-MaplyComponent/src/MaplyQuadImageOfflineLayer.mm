/*
 *  MaplyQuadImageOfflineLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/7/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "MaplyQuadImageOfflineLayer.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyViewControllerLayer_private.h"
#import "QuadDisplayLayer.h"
#import "MaplyActiveObject.h"
#import "MaplyActiveObject_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

@interface MaplyQuadImageOfflineLayer() <WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource>
@end

@implementation MaplyQuadImageOfflineLayer
{
    MaplyBaseViewController * __weak _viewC;
    MaplyCoordinateSystem *coordSys;
    WhirlyKitQuadTileOfflineLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    NSObject<MaplyTileSource> *tileSource;
    Scene *scene;
    bool sourceSupportsMulti;
    int minZoom,maxZoom;
    int tileSize;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys tileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    tileSource = inTileSource;
    
    return self;
}

- (void)reload
{
    if ([NSThread currentThread] != super.layerThread)
    {
        [self performSelector:@selector(reload) onThread:super.layerThread withObject:nil waitUntilDone:NO];
        return;
    }
    
    [quadLayer refresh];
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
}

#pragma mark - WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    _viewC = viewC;
    super.layerThread = inLayerThread;
    scene = inScene;
    
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    tileSize = [tileSource tileSize];

    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileOfflineLoader alloc] initWithName:@"Offline" dataSource:self];
    
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];
    quadLayer.maxTiles = _maxTiles;
    
    [super.layerThread addLayer:quadLayer];
    
    return true;
}

/// Return the coordinate system we're working in
- (WhirlyKit::CoordSystem *)coordSystem
{
    return [coordSys getCoordSystem];
}

/// Bounding box used to calculate quad tree nodes.  In local coordinate system.
- (WhirlyKit::Mbr)totalExtents
{
    MaplyCoordinate ll,ur;
    [coordSys getBoundsLL:&ll ur:&ur];
    
    Mbr mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y));
    return mbr;
}

/// Bounding box of data you actually want to display.  In local coordinate system.
/// Unless you're being clever, make this the same as totalExtents.
- (WhirlyKit::Mbr)validExtents
{
    return [self totalExtents];
}

/// Return the minimum quad tree zoom level (usually 0)
- (int)minZoom
{
    return minZoom;
}

/// Return the maximum quad tree zoom level.  Must be at least minZoom
- (int)maxZoom
{
    return maxZoom;
}

/// Return an importance value for the given tile
- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)mbr viewInfo:(WhirlyKitViewState *) viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    if (ident.level == 0)
        return MAXFLOAT;
    
    float import = 0.0;
    import = ScreenImportance(viewState, frameSize, viewState.eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    
    //    NSLog(@"Tiles = %d: (%d,%d), import = %f",ident.level,ident.x,ident.y,import);
    
    return import;
}
/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    super.layerThread = nil;
}

// Note: Should allow the users to set this
- (int)maxSimultaneousFetches
{
    return _numSimultaneousFetches;
}

@end
