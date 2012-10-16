//
//  WGQuadEarthWithRemoteTiles.mm
//  WhirlyGlobeComponent
//
//  Created by Steve Gifford on 7/24/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//


#import "WGQuadEarthWithRemoteTiles_private.h"

@implementation WGQuadEarthWithRemoteTiles
{
    WhirlyGlobeQuadTileLoader *tileLoader;
    WhirlyGlobeQuadDisplayLayer *quadLayer;
    WhirlyGlobeNetworkTileQuadSource *dataSource;
}

- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene renderer:(WhirlyKitSceneRendererES1 *)renderer baseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom handleEdges:(bool)edges
{
    self = [super init];
    if (self)
    {
        dataSource = [[WhirlyGlobeNetworkTileQuadSource alloc] initWithBaseURL:baseURL ext:ext];
        dataSource.minZoom = minZoom;
        dataSource.maxZoom = maxZoom;
        // Note: Should make this flextible
        dataSource.numSimultaneous = 8;
        tileLoader = [[WhirlyGlobeQuadTileLoader alloc] initWithDataSource:dataSource];
        tileLoader.ignoreEdgeMatching = !edges;
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyGlobeQuadDisplayLayer alloc] initWithDataSource:dataSource loader:tileLoader renderer:renderer];
        [layerThread addLayer:quadLayer];
    }
    
    return self;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene
{
    [layerThread removeLayer:quadLayer];
    tileLoader = nil;
    quadLayer = nil;
    dataSource = nil;
}

- (NSString *)cacheDir
{
    return dataSource.cacheDir;
}

- (void)setCacheDir:(NSString *)cacheDir
{
    dataSource.cacheDir = cacheDir;
}

@end

