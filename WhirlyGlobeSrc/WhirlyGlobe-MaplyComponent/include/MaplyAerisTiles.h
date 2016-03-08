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

@interface MaplyAerisLayerInfo : NSObject

- (nullable instancetype)initWithCode:(NSString *__nonnull)code name:(NSString *__nonnull)name minZoom:(unsigned int)minZoom maxZoom:(unsigned int)maxZoom updatePeriod:(unsigned int)updatePeriod;

@property (nonatomic, strong) NSString * _Nonnull layerCode;
@property (nonatomic, strong) NSString * _Nonnull layerName;
@property (nonatomic, assign) unsigned int minZoom;
@property (nonatomic, assign) unsigned int maxZoom;
@property (nonatomic, assign) unsigned int updatePeriod;

@end

@interface MaplyAerisTiles : NSObject

- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey;


- (nonnull NSDictionary *)layerInfo;

@end

@interface MaplyAerisTileSet : NSObject

- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey layerInfo:(MaplyAerisLayerInfo *__nonnull)layerInfo tileSetCount:(unsigned int)tileSetCount;

- (void)startFetchWithSuccess:(nonnull void (^)(NSArray *__nullable tileSources)) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock;

@end
