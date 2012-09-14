/*
 *  WGQuadEarthWithRemoteTiles.mm
 *  WhirlyGlobeComponent
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

#import "WGQuadEarthWithRemoteTiles_private.h"

@implementation WGQuadEarthWithRemoteTiles
{
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    WhirlyKitNetworkTileQuadSource *dataSource;
}

- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene renderer:(WhirlyKitSceneRendererES1 *)renderer baseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom handleEdges:(bool)edges
{
    self = [super init];
    if (self)
    {
        dataSource = [[WhirlyKitNetworkTileQuadSource alloc] initWithBaseURL:baseURL ext:ext];
        dataSource.minZoom = minZoom;
        dataSource.maxZoom = maxZoom;
        // Note: Should make this flextible
        dataSource.numSimultaneous = 8;
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:dataSource];
        tileLoader.ignoreEdgeMatching = !edges;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:dataSource loader:tileLoader renderer:renderer];
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

