/*
 *  MaplyAerisTiles.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 3/4/16.
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

#import <Foundation/Foundation.h>

/** 
    Represents an Aeris layer.
    
    This object contains information about an Aeris weather image layer.
    
    Don't construct these objects except from the MaplyAerisTiles object.  Instead, get them from MaplyAerisTiles.
    @see MaplyAerisTiles
 */
@interface MaplyAerisLayerInfo : NSObject

/** 
    Initialize with the attributes of the Aeris layer.
    
    Initialize the MaplyAerisLayerInfo object with attributes of the corresponding Aeris weather imagery layer.
    
    This initializer should only be called from within the MaplyAerisTiles object.
    
    @param code The code that Aeris uses to identify the layer.
    
    @param name The name of the layer.
    
    @param minZoom The minimum valid zoom level for the layer.
    
    @param maxZoom The maximum valid zoom level for the layer.
    
    @param updatePeriod How often the layer imagery is updated by Aeris, in minutes.
 */
- (nullable instancetype)initWithCode:(NSString *__nonnull)code name:(NSString *__nonnull)name minZoom:(unsigned int)minZoom maxZoom:(unsigned int)maxZoom updatePeriod:(unsigned int)updatePeriod;

/// The code that Aeris uses to identify the layer.
@property (nonatomic, strong) NSString * _Nonnull layerCode;

/// The name of the layer.
@property (nonatomic, strong) NSString * _Nonnull layerName;

/// The minimum valid zoom level for the layer.
@property (nonatomic, assign) unsigned int minZoom;

/// The maximum valid zoom level for the layer.
@property (nonatomic, assign) unsigned int maxZoom;

/// How often the layer imagery is updated by Aeris, in minutes.
@property (nonatomic, assign) unsigned int updatePeriod;

@end

/** 
    MaplyAerisTiles provides access to available layer information.
    
    Instantiate a MaplyAerisTiles object to retrieve information about available Aeris weather imagery layers.
 */
@interface MaplyAerisTiles : NSObject

/** 
    Initialize with Aeris account information.
    
    @param aerisID The ID for the Aeris account.
    
    @param secretKey The secret key for the Aeris account.
 */
- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey;


/// A dictionary that maps Aeris layer codes to the MaplyAerisLayerInfo objects that represent the corresponding layers.
- (nonnull NSDictionary *)layerInfo;

@end

/** 
    A MaplyAerisTileSet object provides tile sources for display in WhirlyGlobe-Maply.
    
    The MaplyAerisTileSet object provides access to weather imagery from Aeris. Instantiate this object with the desired layer and number of frames. Then, initiate a fetch of the most recent frames.  MaplyAerisTileSet will provide an array of tile sources that can be combined using a MaplyMultiplexTileSource to feed a MaplyQuadImageTilesLayer, which will display the imagery on the map or globe.
    @see MaplyTileSource
    @see MaplyMultiplexTileSource
    @see MaplyQuadImageTilesLayer
 */
@interface MaplyAerisTileSet : NSObject

/** 
    Initialize a MaplyAerisTileSet object.
    
    Initialize a MaplyAerisTileSet object with the necessary parameters.
    
    @param aerisID The ID for the Aeris account.
    
    @param secretKey The secret key for the Aeris account.
    
    @param layerInfo The MaplyAerisLayerInfo object describing the weather layer of interest. Retrieve this from a MaplyAerisTiles object.
    
    @param tileSetCount The number of frames of interest.
 */
- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey layerInfo:(MaplyAerisLayerInfo *__nonnull)layerInfo tileSetCount:(unsigned int)tileSetCount;

/** 
    Start the fetch of the most recent imagery frames.
    
    The startFetchWithSucess:failure: method takes blocks as arguments, because fetching the most recent frame information from Aeris is asynchronous. In the success block, put your custom code which adds the tile sources to a MaplyQuadImageTilesLayer.
    
    @param successBlock In this block, write your custom code for adding the Aeris imagery tile sources to a MaplyQuadImageTilesLayer.
    
    @param failureBlock Handle a fetch error in this block.
 */
- (void)startFetchWithSuccess:(nonnull void (^)(NSArray *__nullable tileSources)) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock;

@end
