/*
 *  MapboxVectorTilesPagingDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyTileSource.h"
#import "MapboxVectorTiles.h"

/**
 Provides on demand creation for Mapnik style vector tiles.
 
 Create one of these to read Mapnik PBF style tiles from a remote
 or local source.  This handles the geometry creation, calls a delegate
 for the styling and can read from remote or local data files.
 */
@interface MapboxVectorTilesPagingDelegate : NSObject <MaplyPagingDelegate>

/// One or more tile sources to fetch data from per tile
@property (nonatomic, readonly, nullable) NSArray *tileSources;

/// Access token to use with the remote service
@property (nonatomic, strong, nonnull) NSString *accessToken;

/// Handles the actual Mapnik vector tile parsing
@property (nonatomic, strong, nullable) MapboxVectorTileParser *tileParser;

/// Minimum zoom level available
@property (nonatomic, assign) int minZoom;

/// Maximum zoom level available
@property (nonatomic, assign) int maxZoom;

/**
 Init with a single remote tile source.
 */
- (nonnull instancetype)initWithTileSource:(NSObject<MaplyTileSource> *__nonnull)tileSource style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/**
 Init with a list of tile sources.
 
 These are MaplyRemoteTileInfo objects and will be combined by the
 MaplyMapnikVectorTiles object for display.
 */
- (nonnull instancetype)initWithTileSources:(NSArray *__nonnull)tileSources style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/**
 Init with the filename of an MBTiles archive containing PBF tiles.
 
 This will read individual tiles from an MBTiles archive containging PBF.
 
 The file should be local.
 */
- (nonnull instancetype)initWithMBTiles:(MaplyMBTileSource *__nonnull)tileSource style:(NSObject<MaplyVectorStyleDelegate> *__nonnull)style viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

@end
