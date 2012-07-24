//
//  WGQuadEarthWithMBTiles.m
//  WhirlyGlobeComponent
//
//  Created by Steve Gifford on 7/24/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "WGQuadEarthWithMBTiles_private.h"

@implementation WGQuadEarthWithMBTiles
{
    WhirlyGlobeQuadTileLoader *tileLoader;
    WhirlyGlobeQuadDisplayLayer *quadLayer;
    WhirlyMBTileQuadSource *dataSource;
}

- (id)initWithWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene renderer:(WhirlyKitSceneRendererES1 *)renderer mbTiles:(NSString *)mbName
{
    self = [super init];
    if (self)
    {
        NSString *infoPath = [[NSBundle mainBundle] pathForResource:mbName ofType:@"mbtiles"];
        if (!infoPath)
        {
            self = nil;
            return nil;
        }
        dataSource = [[WhirlyMBTileQuadSource alloc] initWithPath:infoPath];
        tileLoader = [[WhirlyGlobeQuadTileLoader alloc] initWithDataSource:dataSource];
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

@end

