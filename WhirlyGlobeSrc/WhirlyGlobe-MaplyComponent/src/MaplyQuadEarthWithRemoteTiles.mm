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
    NSDictionary *jsonDict;
    NSString *baseURL,*ext;
    int minZoom,maxZoom;
    WhirlyKitQuadTileLoader *tileLoader;
    WhirlyKitQuadDisplayLayer *quadLayer;
    WhirlyKitNetworkTileQuadSourceBase *dataSource;
}

/// Set up a spherical earth layer with a remote set of tiles.
/// Returns nil on failure.
- (id)initWithBaseURL:(NSString *)inBaseURL ext:(NSString *)inExt minZoom:(int)inMinZoom maxZoom:(int)inMaxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    baseURL = inBaseURL;
    ext = inExt;
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
    
    return self;
}

/// Set up a spherical earth layer with a remote set of tiles defined by the tilespec
///  in JSON (that's been parsed into an NSDictionary).
- (id)initWithTilespec:(NSDictionary *)inJsonDict
{
    self = [super init];
    if (!self)
        return nil;
    
    jsonDict = inJsonDict;
    
    return self;
}

- (bool)startLayer:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC
{
    if (jsonDict)
    {
        WhirlyKitNetworkTileSpecQuadSource *theDataSource = [[WhirlyKitNetworkTileSpecQuadSource alloc] initWithTileSpec:jsonDict];
        if (!theDataSource)
            return nil;
        dataSource = theDataSource;
        dataSource.cacheDir = _cacheDir;
        // Note: Should make this flexible
        theDataSource.numSimultaneous = 8;
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:theDataSource];
        tileLoader.ignoreEdgeMatching = !_handleEdges;
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:theDataSource loader:tileLoader renderer:renderer];
        [layerThread addLayer:quadLayer];
    } else {
        WhirlyKitNetworkTileQuadSource *theDataSource = [[WhirlyKitNetworkTileQuadSource alloc] initWithBaseURL:baseURL ext:ext];
        dataSource = theDataSource;
        theDataSource.minZoom = minZoom;
        theDataSource.maxZoom = maxZoom;
        // Note: Should make this flexible
        dataSource.numSimultaneous = 8;
        dataSource.cacheDir = _cacheDir;
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:theDataSource];
        tileLoader.ignoreEdgeMatching = !_handleEdges;
        tileLoader.coverPoles = true;
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:theDataSource loader:tileLoader renderer:renderer];
        [layerThread addLayer:quadLayer];        
    }
    
    return true;
}

- (void)setHandleEdges:(bool)handleEdges
{
    _handleEdges = handleEdges;
    if (tileLoader)
        tileLoader.ignoreEdgeMatching = !_handleEdges;
}

- (void)setCacheDir:(NSString *)cacheDir
{
    _cacheDir = cacheDir;
    if (tileLoader)
        dataSource.cacheDir = _cacheDir;
}

- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene
{
    [layerThread removeLayer:quadLayer];
    tileLoader = nil;
    quadLayer = nil;
    dataSource = nil;
}

@end

