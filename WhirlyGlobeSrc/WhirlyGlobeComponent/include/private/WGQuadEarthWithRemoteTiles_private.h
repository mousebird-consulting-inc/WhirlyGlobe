//
//  WGQuadEarthWithRemoteTiles.h
//  WhirlyGlobeComponent
//
//  Created by Steve Gifford on 7/24/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "WGViewControllerLayer_private.h"

@interface WGQuadEarthWithRemoteTiles : WGViewControllerLayer

@property (nonatomic,strong) NSString *cacheDir;

/// Set up a spherical earth layer with an MBTiles archive.
/// Returns nil on failure.
- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene renderer:(WhirlyKitSceneRendererES1 *)renderer baseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom handleEdges:(bool)edges;

/// Clean up any and all resources 
- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyGlobe::GlobeScene *)globeScene;

@end
