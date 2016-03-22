/*
 *  MaplyAerisTiles.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 3/4/16.
 *  Copyright 2011-2016 mousebird consulting
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

/** @brief Represents an Aeris layer.
    @details This object contains information about an Aeris weather image layer.
    @details Don't construct these objects except from the MaplyAerisTiles object.  Instead, get them from MaplyAerisTiles.
    @see MaplyAerisTiles
 */
@interface MaplyAerisLayerInfo : NSObject

/** @brief Initialize with the attributes of the Aeris layer.
    @details Initialize the MaplyAerisLayerInfo object with attributes of the corresponding Aeris weather imagery layer.
    @details This initializer should only be called from within the MaplyAerisTiles object.
    @param code The code that Aeris uses to identify the layer.
    @param name The name of the layer.
    @param minZoom The minimum valid zoom level for the layer.
    @param maxZoom The maximum valid zoom level for the layer.
    @param updatePeriod How often the layer imagery is updated by Aeris, in minutes.
 */
- (nullable instancetype)initWithCode:(NSString *__nonnull)code name:(NSString *__nonnull)name minZoom:(unsigned int)minZoom maxZoom:(unsigned int)maxZoom updatePeriod:(unsigned int)updatePeriod;

/// @brief The code that Aeris uses to identify the layer.
@property (nonatomic, strong) NSString * _Nonnull layerCode;

/// @brief The name of the layer.
@property (nonatomic, strong) NSString * _Nonnull layerName;

/// @brief The minimum valid zoom level for the layer.
@property (nonatomic, assign) unsigned int minZoom;

/// @brief The maximum valid zoom level for the layer.
@property (nonatomic, assign) unsigned int maxZoom;

/// @brief How often the layer imagery is updated by Aeris, in minutes.
@property (nonatomic, assign) unsigned int updatePeriod;

@end

/** @brief MaplyAerisTiles provides access to available layer information.
    @details Instantiate a MaplyAerisTiles object to retrieve information about available Aeris weather imagery layers.
 */
@interface MaplyAerisTiles : NSObject

/** @brief Initialize with Aeris account information.
    @param aerisID The ID for the Aeris account.
    @param secretKey The secret key for the Aeris account.
 */
- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey;


/// @brief A dictionary that maps Aeris layer codes to the MaplyAerisLayerInfo objects that represent the corresponding layers.
- (nonnull NSDictionary *)layerInfo;

@end

/** @brief A MaplyAerisTileSet object provides tile sources for display in WhirlyGlobe-Maply.
    @details The MaplyAerisTileSet object provides access to weather imagery from Aeris. Instantiate this object with the desired layer and number of frames. Then, initiate a fetch of the most recent frames.  MaplyAerisTileSet will provide an array of tile sources that can be combined using a MaplyMultiplexTileSource to feed a MaplyQuadImageTilesLayer, which will display the imagery on the map or globe.
    @see MaplyTileSource
    @see MaplyMultiplexTileSource
    @see MaplyQuadImageTilesLayer
 */
@interface MaplyAerisTileSet : NSObject

/** @brief Initialize a MaplyAerisTileSet object.
    @details Initialize a MaplyAerisTileSet object with the necessary parameters.
    @param aerisID The ID for the Aeris account.
    @param secretKey The secret key for the Aeris account.
    @param layerInfo The MaplyAerisLayerInfo object describing the weather layer of interest. Retrieve this from a MaplyAerisTiles object.
    @param tileSetCount The number of frames of interest.
 */
- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey layerInfo:(MaplyAerisLayerInfo *__nonnull)layerInfo tileSetCount:(unsigned int)tileSetCount;

/** @brief Start the fetch of the most recent imagery frames.
    @details The startFetchWithSucess:failure: method takes blocks as arguments, because fetching the most recent frame information from Aeris is asynchronous. In the success block, put your custom code which adds the tile sources to a MaplyQuadImageTilesLayer.
    @param successBlock In this block, write your custom code for adding the Aeris imagery tile sources to a MaplyQuadImageTilesLayer.
    @param failureBlock Handle a fetch error in this block.
 */
- (void)startFetchWithSuccess:(nonnull void (^)(NSArray *__nullable tileSources)) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock;

@end
