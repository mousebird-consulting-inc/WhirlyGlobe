/*
 *  MaplyQuadEarthWithRemoteTiles.h
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

#import "MaplyViewControllerLayer.h"
#import <WhirlyGlobe.h>

@interface MaplyQuadEarthWithRemoteTiles : MaplyViewControllerLayer

/// Set up a spherical earth layer with a remote set of tiles.
/// Returns nil on failure.
- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer baseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom handleEdges:(bool)edges;

/// Set up a spherical earth layer with a remote set of tiles defined by the tilespec
///  in JSON (that's been parsed into an NSDictionary).
- (id)initWithLayerThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer tilespec:(NSDictionary *)jsonDict handleEdges:(bool)edges;

/// Clean up any and all resources
- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

@end
