/*
 *  MaplyQuadEarthTilesLayer.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

#import "MaplyQuadEarthTilesLayer_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "QuadDisplayLayer.h"

using namespace WhirlyKit;

@implementation MaplyQuadEarthTilesLayer
{
    WhirlyKitLayerThread *layerThread;
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    Scene *scene;
    MaplyCoordinateSystem *coordSys;
    NSObject<MaplyTileSource> *tileSource;
    int minZoom,maxZoom;
    int tileSize;
}

- (id)initWithCoordSystem:(MaplyCoordinateSystem *)inCoordSys tileSource:(NSObject<MaplyTileSource> *)inTileSource
{
    self = [super init];
    
    coordSys = inCoordSys;
    tileSource = inTileSource;
    _numSimultaneousFetches = 8;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene renderer:(WhirlyKitSceneRendererES *)renderer
{
    layerThread = inLayerThread;
    scene = inScene;

    // Cache min and max zoom.  Tile sources might do a lookup for these
    minZoom = [tileSource minZoom];
    maxZoom = [tileSource maxZoom];
    tileSize = [tileSource tileSize];

    // Set up tile and and quad layer with us as the data source
    tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:self];
    tileLoader.ignoreEdgeMatching = true;
    tileLoader.coverPoles = true;
    quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:self loader:tileLoader renderer:renderer];

    [layerThread addLayer:quadLayer];

    return true;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)scene
{
    [inLayerThread removeLayer:quadLayer];
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
    
    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, tileSize, [coordSys getCoordSystem], scene->getCoordAdapter(), mbr, ident, attrs);
    return import;
}

/// Called when the layer is shutting down.  Clean up any drawable data and clear out caches.
- (void)shutdown
{
    layerThread = nil;
}

// Note: Should allow the users to set this
- (int)maxSimultaneousFetches
{
    return _numSimultaneousFetches;
}

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    MaplyTileID tileID;
    tileID.x = col;  tileID.y = row;  tileID.level = level;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       UIImage *image = [tileSource imageForTile:tileID];
    
                       NSObject *loadImage = nil;
                       if (image)
                           loadImage = [WhirlyKitLoadedImage LoadedImageWithUIImage:image];
                       else
                           loadImage = [NSNull null];
                       
                       NSArray *args = @[loadImage,@(col),@(row),@(level)];
                       if (layerThread)
                           [self performSelector:@selector(mergeTile:) onThread:layerThread withObject:args waitUntilDone:NO];
                   });
}

// Merge the tile result in on the layer thread
- (void)mergeTile:(NSArray *)args
{
    if (!layerThread)
        return;
    
    WhirlyKitLoadedImage *loadImage = args[0];
    if ([loadImage isKindOfClass:[NSNull class]])
        loadImage = nil;
    int col = [args[1] intValue];
    int row = [args[2] intValue];
    int level = [args[3] intValue];
    
    [tileLoader dataSource: self loadedImage:loadImage forLevel: level col: col row: row];    
}


@end
