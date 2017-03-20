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

- (nonnull instancetype)initWithBaseURL:(NSString *__nonnull)baseURL ext:(NSString *__nonnull)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

@property (nonatomic,readonly,nonnull) NSString *baseURL;
@property (nonatomic, strong,nonnull) NSString *ext;

@property (nonatomic) int minZoom;
@property (nonatomic) int maxZoom;

@property (nonatomic,assign) float timeOut;

@property (nonatomic) int pixelsPerSide;

@property (nonatomic,strong,nullable) MaplyCoordinateSystem *coordSys;

@property (nonatomic, strong,nullable) NSString *cacheDir;

@property (nonatomic) int cachedFileLifetime;

@property (nonatomic,strong,nullable) NSString *queryStr;

//TODO(JM) not needed yet?
//- (void)addBoundingBox:(MaplyBoundingBox *)bbox;
//- (void)addGeoBoundingBox:(MaplyBoundingBox *)bbox;
//- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox;

- (nullable NSURLRequest *)requestForTile:(MaplyTileID)tileID;

- (nullable NSString *)fileNameForTile:(MaplyTileID)tileID;

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;


@end


@class MaplyRemoteTileElevationSource;


@protocol MaplyRemoteTileElevationSourceDelegate <NSObject>

@optional

- (void) remoteTileElevationSource:(id __nonnull)tileSource tileDidLoad:(MaplyTileID)tileID;

- (nullable MaplyElevationChunk *) remoteTileElevationSource:(id __nonnull)tileSource
		modifyElevReturn:(MaplyElevationChunk *__nullable)elevChunk
		forTile:(MaplyTileID)tileID;

- (void) remoteTileElevationSource:(id __nonnull)tileSource tileDidNotLoad:(MaplyTileID)tileID error:(NSError *__nonnull)error;

//TODO(JM) need enable/disable elevation?
//- (void)remoteTileElevationSource:(id)tileSource tileDisabled:(MaplyTileID)tileID;
//- (void)remoteTileElevationSource:(id)tileSource tileEnabled:(MaplyTileID)tileID;

- (void)remoteTileElevationSource:(id __nonnull)tileSource tileUnloaded:(MaplyTileID)tileID;

@end


@interface MaplyRemoteTileElevationSource : NSObject<MaplyElevationSourceDelegate>

- (nullable instancetype)initWithBaseURL:(NSString *__nonnull)baseURL ext:(NSString *__nonnull)ext minZoom:(int)minZoom maxZoom:(int)maxZoom;

- (nullable instancetype)initWithInfo:(MaplyRemoteTileElevationInfo *__nonnull)info;


// Inherited from MaplyElevationSourceDelegate
- (nullable MaplyCoordinateSystem *)getCoordSystem;
- (int)minZoom;
- (int)maxZoom;

- (nullable MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;

- (void)startFetchLayer:(id __nonnull)layer tile:(MaplyTileID)tileID;

- (nonnull MaplyElevationChunk *)decodeElevationData:(NSData *__nonnull)data;

@property (nonatomic,strong,nullable) MaplyRemoteTileElevationInfo *tileInfo;

@property (nonatomic,weak,nullable) NSObject<MaplyRemoteTileElevationSourceDelegate> *delegate;

@property (nonatomic,strong,nullable) MaplyCoordinateSystem *coordSys;
@property (nonatomic,strong,nullable) NSString *cacheDir;


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


