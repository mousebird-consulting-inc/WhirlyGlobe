/*
 *  MaplyAerisTiles.mm
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

#import "MaplyAerisTiles.h"
#import "MaplyRemoteTileSource.h"

@implementation MaplyAerisLayerInfo



- (nullable instancetype)initWithCode:(NSString *__nonnull)code name:(NSString *__nonnull)name minZoom:(unsigned int)minZoom maxZoom:(unsigned int)maxZoom updatePeriod:(unsigned int)updatePeriod {
    
    self = [super init];
    if (!self)
        return nil;
    
    self.layerCode = code;
    self.layerName = name;
    self.minZoom = minZoom;
    self.maxZoom = maxZoom;
    self.updatePeriod = updatePeriod;
    return self;
}



@end



@implementation MaplyAerisTiles {
    
    NSString *_aerisID;
    NSString *_secretKey;
    
}

- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey {
    
    self = [super init];
    if (!self)
        return nil;
    
    _aerisID = aerisID;
    _secretKey = secretKey;
    
    return self;
}

- (nonnull NSDictionary *)layerInfo {
    return @{
             @"radar" : [[MaplyAerisLayerInfo alloc] initWithCode:@"radar" name:@"Radar" minZoom:1 maxZoom:22 updatePeriod:6],
             @"satellite" : [[MaplyAerisLayerInfo alloc] initWithCode:@"satellite" name:@"Infrared Satellite" minZoom:1 maxZoom:22 updatePeriod:30],
             @"sat-global" : [[MaplyAerisLayerInfo alloc] initWithCode:@"sat-global" name:@"Global Satellite" minZoom:1 maxZoom:22 updatePeriod:120],
             @"sat-vis-hires" : [[MaplyAerisLayerInfo alloc] initWithCode:@"sat-vis-hires" name:@"Hi-Res Visible Satellite" minZoom:1 maxZoom:22 updatePeriod:30]
             };
}

@end

@implementation MaplyAerisTileSet {
    NSString *_aerisID;
    NSString *_secretKey;
    MaplyAerisLayerInfo *_layerInfo;
    unsigned int _tileSetCount;
}

- (nullable instancetype)initWithAerisID:(NSString *__nonnull)aerisID secretKey:(NSString *__nonnull)secretKey layerInfo:(MaplyAerisLayerInfo *__nonnull)layerInfo tileSetCount:(unsigned int)tileSetCount {

    self = [super init];
    if (!self)
        return nil;
    
    _aerisID = aerisID;
    _secretKey = secretKey;
    _layerInfo = layerInfo;
    _tileSetCount = tileSetCount;
    
    return self;
}

- (void)startFetchWithSuccess:(nonnull void (^)(NSArray *__nullable tileSources)) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    NSURLSession *session = [NSURLSession sharedSession];
  
    NSURL *jsonURL = [NSURL URLWithString:[NSString stringWithFormat:@"http://maps.aerisapi.com/%@_%@/%@.json", _aerisID, _secretKey, _layerInfo.layerCode]];
    
    NSURLSessionDataTask *task = [session dataTaskWithURL:jsonURL completionHandler:
    ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                       ^{
           NSError *jsonError;
           NSDictionary *layerDict;
           if (!error)
               layerDict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&jsonError];
           if (!error && !jsonError) {
               
               NSMutableArray *tileInfoArray = [NSMutableArray array];
               NSArray *files = layerDict[@"files"];
               NSSortDescriptor *descriptor = [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:NO];
               NSArray *sortedFiles = [files sortedArrayUsingDescriptors:@[descriptor]];
               NSDictionary *fileEntry;
               MaplyRemoteTileInfo *tileSource;
               NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
               for (int i=0; i<MIN(_tileSetCount,sortedFiles.count); i++) {
                   fileEntry = sortedFiles[i];
                   NSString *baseURL = [NSString stringWithFormat:@"http://maps.aerisapi.com/%@_%@/%@/{z}/{x}/{y}/%@", _aerisID, _secretKey, _layerInfo.layerCode, fileEntry[@"time"]];
                   tileSource = [[MaplyRemoteTileInfo alloc] initWithBaseURL:baseURL ext:@"png" minZoom:_layerInfo.minZoom maxZoom:_layerInfo.maxZoom];
                   tileSource.cacheDir = [NSString stringWithFormat:@"%@/MaplyAeris/%@/%@", cacheDir, _layerInfo.layerCode, fileEntry[@"time"]];
                   [tileInfoArray addObject:tileSource];
               }
               dispatch_async(dispatch_get_main_queue(), ^{
                   successBlock(tileInfoArray);
               });
               
           } else {
               dispatch_async(dispatch_get_main_queue(), ^{
                   failureBlock([[NSError alloc] initWithDomain:@"MaplyAerisTileSet" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to reach JSON layer spec"}]);
               });
           }
        });
        
    }];
    [task resume];
}

@end

