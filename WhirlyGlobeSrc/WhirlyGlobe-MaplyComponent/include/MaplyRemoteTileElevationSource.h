/*
 *  MaplyRemoteTileElevationSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by @jmnavarro on 6/16/15.
 *  Copyright 2011-2015 mousebird consulting
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
#import "MaplyCoordinateSystem.h"
#import "MaplyElevationSource.h"

@interface MaplyRemoteTileElevationInfo : NSObject

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

@property (nonatomic,readonly) NSString *baseURL;

@property (nonatomic) int minZoom;
@property (nonatomic) int maxZoom;

@property (nonatomic, strong) NSString *ext;

@property (nonatomic,assign) float timeOut;

@property (nonatomic) int pixelsPerSide;

@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

@property (nonatomic, strong) NSString *cacheDir;

@property (nonatomic) int cachedFileLifetime;

@property (nonatomic,strong) NSString *queryStr;

//TODO(JM) not needed yet?
//- (void)addBoundingBox:(MaplyBoundingBox *)bbox;
//- (void)addGeoBoundingBox:(MaplyBoundingBox *)bbox;
//- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox;

- (NSURLRequest *)requestForTile:(MaplyTileID)tileID;

- (NSString *)fileNameForTile:(MaplyTileID)tileID;

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;


@end


@class MaplyRemoteTileElevationSource;


@protocol MaplyRemoteTileElevationSourceDelegate <NSObject>

@optional

- (void) remoteTileElevationSource:(id)tileSource tileDidLoad:(MaplyTileID)tileID;

- (MaplyElevationChunk *) remoteTileElevationSource:(id)tileSource
		modifyElevReturn:(MaplyElevationChunk *)elevChunk
		forTile:(MaplyTileID)tileID;

- (void) remoteTileElevationSource:(id)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *)error;

//TODO(JM) need enable/disable elevation?
//- (void)remoteTileElevationSource:(id)tileSource tileDisabled:(MaplyTileID)tileID;
//- (void)remoteTileElevationSource:(id)tileSource tileEnabled:(MaplyTileID)tileID;

- (void)remoteTileElevationSource:(id)tileSource tileUnloaded:(MaplyTileID)tileID;

@end


@interface MaplyRemoteTileElevationSource : NSObject<MaplyElevationSourceDelegate>

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

- (id)initWithInfo:(MaplyRemoteTileElevationInfo *)info;


// Inherited from MaplyElevationSourceDelegate
- (MaplyCoordinateSystem *)getCoordSystem;
- (int)minZoom;
- (int)maxZoom;

- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;

- (void)startFetchLayer:(id)layer tile:(MaplyTileID)tileID;

- (MaplyElevationChunk *)decodeElevationData:(NSData *)data;

@property (nonatomic,strong) MaplyRemoteTileElevationInfo *tileInfo;

@property (nonatomic,weak) NSObject<MaplyRemoteTileElevationSourceDelegate> *delegate;

@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;
@property (nonatomic,strong) NSString *cacheDir;


//TODO(JM) needed?
//+ (void)setTrackConnections:(bool)track;
//+ (int)numOutstandingConnections;

@end


@interface MaplyRemoteTileElevationCesiumSource : MaplyRemoteTileElevationSource

/// @detail Multiply the Z values by this amount
@property (nonatomic) float scale;

@end

@interface MaplyRemoteTileElevationCesiumInfo : MaplyRemoteTileElevationInfo
@end

/** @details The Cesium terrain server uses an odd tiling system with two top level nodes.
 */
@interface MaplyCesiumCoordSystem : MaplyPlateCarree

@end


