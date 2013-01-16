/*
 *  MaplyQuadEarthWithRemoteTiles.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/24/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "MaplyQuadEarthWithRemoteTiles_private.h"

@implementation MaplyQuadEarthWithRemoteTiles
{
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    WhirlyKitNetworkTileQuadSourceBase *dataSource;
}

- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer baseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom handleEdges:(bool)edges
{
    self = [super init];
    if (self)
    {
        WhirlyKitNetworkTileQuadSource *theDataSource = [[WhirlyKitNetworkTileQuadSource alloc] initWithBaseURL:baseURL ext:ext];
        dataSource = theDataSource;
        theDataSource.minZoom = minZoom;
        theDataSource.maxZoom = maxZoom;
        // Note: Should make this flexible
        dataSource.numSimultaneous = 8;
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:theDataSource];
        tileLoader.ignoreEdgeMatching = !edges;
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:theDataSource loader:tileLoader renderer:renderer];
        [layerThread addLayer:quadLayer];
    }
    
    return self;
}

- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer tilespec:(NSDictionary *)jsonDict handleEdges:(bool)edges
{
    self = [super init];
    if (self)
    {
        WhirlyKitNetworkTileSpecQuadSource *theDataSource = [[WhirlyKitNetworkTileSpecQuadSource alloc] initWithTileSpec:jsonDict];
        if (!theDataSource)
            return nil;
        dataSource = theDataSource;
        theDataSource.numSimultaneous = 8;
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:theDataSource];
        tileLoader.ignoreEdgeMatching = !edges;
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:theDataSource loader:tileLoader renderer:renderer];
        [layerThread addLayer:quadLayer];
    }
    
    return self;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene
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

