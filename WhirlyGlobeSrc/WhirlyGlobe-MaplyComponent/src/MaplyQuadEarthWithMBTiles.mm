/*
 *  MaplyQuadEarthWithMBTiles.mm
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

#import "MaplyQuadEarthWithMBTiles_private.h"

@implementation MaplyQuadEarthWithMBTiles
{
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    WhirlyKitMBTileQuadSource *dataSource;
}

- (id)initWithWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer mbTiles:(NSString *)mbName handleEdges:(bool)edges
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
        dataSource = [[WhirlyKitMBTileQuadSource alloc] initWithPath:infoPath];
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:dataSource];
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:dataSource loader:tileLoader renderer:renderer];
        tileLoader.ignoreEdgeMatching = !edges;
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

@end

