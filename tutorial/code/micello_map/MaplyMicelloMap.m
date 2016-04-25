/*
 *  MaplyMicelloMap.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 4/12/16.
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

#import "MaplyMicelloMap.h"
#import "MaplyVectorObject.h"

@interface MaplyMicelloMapEntity ()

@property (nonatomic, assign) int entityID;
@property (nonatomic, strong) NSDictionary *_Nonnull properties;
@property (nonatomic, strong) NSString *_Nullable intAddress;
@property (nonatomic, assign) double lonDeg;
@property (nonatomic, assign) double latDeg;

@end

@implementation MaplyMicelloMapEntity
@end

@interface MaplyMicelloMapLevel ()

@property (nonatomic, assign) int levelID;
@property (nonatomic, strong) NSString *_Nonnull name;
@property (nonatomic, assign) int zLevel;
@property (nonatomic, assign) bool isMain;
@property (nonatomic, strong) NSArray *_Nonnull features;

@end

@implementation MaplyMicelloMapLevel
@end

@interface MaplyMicelloMapDrawing ()

@property (nonatomic, assign) int drawingID;
@property (nonatomic, assign) bool isRoot;
@property (nonatomic, strong) NSString *_Nonnull name;
@property (nonatomic, strong) NSString *_Nonnull displayName;
@property (nonatomic, strong) NSString *_Nonnull mapType;
@property (nonatomic, strong) NSDictionary *_Nonnull levels;
@property (nonatomic, strong) MaplyMicelloMapLevel *_Nullable mainLevel;

@end

@implementation MaplyMicelloMapDrawing
@end

@implementation MaplyMicelloStyleRule

- (nullable instancetype)initWithKey:(NSString *__nonnull)key value:(NSObject *__nonnull)value desc:(NSDictionary *__nonnull)desc {
    
    self = [super init];
    if (!self)
        return nil;
    
    self.key = key;
    self.value = value;
    self.desc = desc;
    
    return self;
}

@end

@interface MaplyMicelloMap ()

@property (nonatomic, strong) NSDictionary *_Nonnull drawings;
@property (nonatomic, strong) MaplyMicelloMapDrawing *_Nullable rootDrawing;
@property (nonatomic, strong) NSDictionary *_Nonnull entities;
@property (nonatomic, assign) double centerLonDeg;
@property (nonatomic, assign) double centerLatDeg;
@property (nonatomic, strong) NSArray *_Nonnull zLevels;
@property (nonatomic, strong) NSDictionary *_Nonnull zLevelsToLevels;
@property (nonatomic, assign) int zLevel;


@end




@implementation MaplyMicelloMap {
    NSString *_baseURL;
    NSString *_projectKey;

    int _baseDrawPriority;
    
    NSDictionary *_entities;
    MaplyComponentObject *_outlinesCompObj, *_labelsCompObj, *_highlightCompObj;
    NSMutableArray *_fillCompObjs;
    NSMutableArray *_styleRules;
}

- (nullable instancetype)initWithBaseURL:(NSString *__nonnull)baseURL projectKey:(NSString *__nonnull)projectKey baseDrawPriority:(int)baseDrawPriority {
    
    self = [super init];
    if (!self)
        return nil;
    
    _baseURL = baseURL;
    _projectKey = projectKey;
    _baseDrawPriority = baseDrawPriority;
    
    self.fillColor = [UIColor whiteColor];
    self.outlineColor = [UIColor blackColor];
    self.selectedOutlineColor = [UIColor blueColor];
    self.lineWidth = 3.0;
    self.selectedLineWidth = 10.0;
    _fillCompObjs = [NSMutableArray array];
    self.zLevel = -1;
    _styleRules = [NSMutableArray array];
    
    return self;
}

- (void)startFetchMapWithSuccess:(nonnull void (^)()) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    NSURLSession *session = [NSURLSession sharedSession];
    
    NSURL *comMapURL = [NSURL URLWithString:[NSString stringWithFormat:@"%@/com-map?key=%@", _baseURL, _projectKey]];
    
    NSURLSessionDataTask *task = [session dataTaskWithURL:comMapURL completionHandler:
        ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
            
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
            ^{
                NSError *jsonError;
                NSDictionary *comMapDict;
                if (!error)
                    comMapDict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&jsonError];
                if (!error && !jsonError) {
                    // Assign drawings and levels
                    
                    NSDictionary *rawLoc = comMapDict[@"location"];
                    NSArray *rawLocCoords = rawLoc[@"coordinates"];
                    NSNumber *lonDeg = rawLocCoords[0];
                    NSNumber *latDeg = rawLocCoords[1];
                    self.centerLonDeg = lonDeg.doubleValue;
                    self.centerLatDeg = latDeg.doubleValue;
                    
                    NSArray *rawDrawings = comMapDict[@"drawings"];
                    NSMutableDictionary *drawings = [NSMutableDictionary dictionary];
                    MaplyMicelloMapDrawing *rootDrawing;
                    
                    NSMutableSet *zLevelSet = [NSMutableSet set];
                    NSMutableDictionary *zLevelsDict = [NSMutableDictionary dictionary];
                    
                    for (NSDictionary *rawDrawing in rawDrawings) {
                        MaplyMicelloMapDrawing *drawing = [[MaplyMicelloMapDrawing alloc] init];
                        drawing.drawingID = ((NSNumber *)rawDrawing[@"id"]).intValue;
                        NSDictionary *dProperties = rawDrawing[@"properties"];
                        if (dProperties[@"is_root"] && ((NSNumber *)dProperties[@"is_root"]).boolValue) {
                            drawing.isRoot = true;
                            rootDrawing = drawing;
                        }
                        drawing.name = (NSString *)dProperties[@"name"];
                        drawing.displayName = (NSString *)dProperties[@"display_name"];
                        drawing.mapType = (NSString *)dProperties[@"map_type"];
                        
                        drawings[@(drawing.drawingID)] = drawing;
                        
                        NSArray *rawLevels = rawDrawing[@"levels"];
                        NSMutableDictionary *levels = [NSMutableDictionary dictionary];
                        for (NSDictionary *rawLevel in rawLevels) {
                            MaplyMicelloMapLevel *level = [[MaplyMicelloMapLevel alloc] init];
                            level.levelID = ((NSNumber *)rawLevel[@"id"]).intValue;
                            NSDictionary *lProperties = rawLevel[@"properties"];
                            level.name = (NSString *)lProperties[@"name"];
                            level.zLevel = ((NSNumber *)lProperties[@"zlevel"]).intValue;
                            
                            [zLevelSet addObject:@(level.zLevel)];
                            if (!zLevelsDict[@(level.zLevel)])
                                zLevelsDict[@(level.zLevel)] = [NSMutableArray array];
                            
                            [((NSMutableArray *)zLevelsDict[@(level.zLevel)]) addObject:level];
                            
                            if (lProperties[@"main"] && ((NSNumber *)lProperties[@"main"]).boolValue) {
                                level.isMain = true;
                                drawing.mainLevel = level;
                            }
                            
                            levels[@(level.levelID)] = level;
                        }
                        drawing.levels = [NSDictionary dictionaryWithDictionary:levels];
                    }
                    
                    dispatch_async(dispatch_get_main_queue(), ^{
                        self.drawings = [NSDictionary dictionaryWithDictionary:drawings];
                        self.rootDrawing = rootDrawing;
                        self.zLevels = [[zLevelSet allObjects] sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"self" ascending:YES]]];
                        self.zLevelsToLevels = [NSDictionary dictionaryWithDictionary:zLevelsDict];
                        [self startFetchEntitiesWithSuccess:successBlock failure:failureBlock];
                    });
                } else {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        failureBlock([[NSError alloc] initWithDomain:@"MaplyMicelloMap" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to fetch community map JSON file."}]);
                });
            }
        });
    }];
    [task resume];
}

- (void)startFetchEntitiesWithSuccess:(nonnull void (^)()) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    NSURLSession *session = [NSURLSession sharedSession];
    
    NSURL *comEntitiesURL = [NSURL URLWithString:[NSString stringWithFormat:@"%@/com-entity?key=%@", _baseURL, _projectKey]];
    
    NSURLSessionDataTask *task = [session dataTaskWithURL:comEntitiesURL completionHandler:
        ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
            
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
            ^{
                NSError *jsonError;
                NSDictionary *comEntityDict;
                if (!error)
                    comEntityDict = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&jsonError];
                if (!error && !jsonError) {
                    
                    // Assign entities.
                    
                    NSArray *rawEntities = comEntityDict[@"entities"];
                    NSMutableDictionary *entities = [NSMutableDictionary dictionary];
                    for (NSDictionary *rawEntity in rawEntities) {
                        MaplyMicelloMapEntity *entity = [[MaplyMicelloMapEntity alloc] init];
                        entity.entityID = ((NSNumber *)rawEntity[@"id"]).intValue;
                        entity.properties = rawEntity[@"properties"];
                        
                        NSMutableArray *rawIntAddress = entity.properties[@"int_address"];
                        // Flatten internal address array
                        NSString *joined = [rawIntAddress componentsJoinedByString:@", "];
                        entity.intAddress = [joined stringByReplacingOccurrencesOfString:@"$id" withString:((NSNumber *)rawEntity[@"id"]).stringValue];
                        
                        entities[@(entity.entityID)] = entity;
                    }
                    
                    dispatch_async(dispatch_get_main_queue(), ^{
                        self.entities = [NSDictionary dictionaryWithDictionary:entities];
                        successBlock();
                    });
                } else {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        failureBlock([[NSError alloc] initWithDomain:@"MaplyMicelloMap" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to fetch community entity JSON file."}]);
                });
            }
        });
    }];
    [task resume];
}


- (void)addStyleRule:(MaplyMicelloStyleRule *__nonnull)styleRule {
    [_styleRules addObject:styleRule];
}


- (void)addDefaultStyleRules {
    // Add default styles.  The order that these rules are added establishes their order of evaluation (the first rule that matches for a particular geometry wins).  The draw priorities are important for having overlapping geometries display in the correct order.
    
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"is_root" value:@(YES) desc:@{kMaplyDrawPriority:@(_baseDrawPriority)}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"$style" value:@"Background" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+1)}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"meta" value:@"level outline" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+2)}]];

    
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"facility" value:@"unit" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+9), kMaplyColor:[UIColor colorWithRed:0.741 green:0.870 blue:0.980 alpha:1.0]}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"facility" value:@"hallway" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+8), kMaplyColor:[UIColor colorWithRed:0.957 green:0.949 blue:0.882 alpha:1.0]}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"facility" value:@"inaccessible space" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+7), kMaplyColor:[UIColor whiteColor]}]];

    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"structure" value:@"building" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+6), kMaplyColor:[UIColor colorWithRed:0.741 green:0.870 blue:0.980 alpha:1.0]}]];

    
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"traffic" value:@"parking lot" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+3), kMaplyColor:[UIColor colorWithRed:0.855 green:0.875 blue:0.882 alpha:1.0]}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"traffic" value:@"road" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+4), kMaplyColor:[UIColor darkGrayColor]}]];
    [self addStyleRule:[[MaplyMicelloStyleRule alloc] initWithKey:@"structure" value:@"parking structure" desc:@{kMaplyDrawPriority:@(_baseDrawPriority+5), kMaplyColor:[UIColor colorWithRed:0.925 green:0.941 blue:0.945 alpha:1.0]}]];
    
    
}

- (void)startFetchLevel:(MaplyMicelloMapLevel *__nonnull)level success:(nonnull void (^)(MaplyMicelloMapLevel *__nullable level)) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    // Start fetching a particular level.
    
    if (level.features) {
        successBlock(level);
        return;
    }
    
    NSURLSession *session = [NSURLSession sharedSession];
    
    NSURL *comEntitiesURL = [NSURL URLWithString:[NSString stringWithFormat:@"%@/geojson-level-geom/%i?key=%@", _baseURL, level.levelID, _projectKey]];
    
    NSURLSessionDataTask *task = [session dataTaskWithURL:comEntitiesURL completionHandler:
        ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {

            if (!error && data) {
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                ^{
                    NSError *jsonError;
                    NSMutableDictionary *jsonDict = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:&jsonError];
                    NSMutableArray *rawFeatures = jsonDict[@"features"];
                    for (NSMutableDictionary *rawFeature in rawFeatures) {
                        NSMutableDictionary *rawGeometry = rawFeature[@"geometry"];
                        NSString *sType = rawGeometry[@"type"];
                        if ([sType isEqualToString:@"Polygon"]) {
                            
                            // Normalize coordinates to the center of the community map, to retain more precision.
                            NSMutableArray *coords = rawGeometry[@"coordinates"];
                            for (NSMutableArray *ring in coords) {
                                for (NSMutableArray *coord in ring) {
                                    NSNumber *lonDeg = coord[0];
                                    NSNumber *latDeg = coord[1];
                                    coord[0] = @(lonDeg.doubleValue - self.centerLonDeg);
                                    coord[1] = @(latDeg.doubleValue - self.centerLatDeg);
                                }
                            }
                            
                            // Assign location and label_area attributes of the feature to the geometry, so that they can be used directly from the geometry elsewhere.
                            NSMutableDictionary *rawLocation = rawFeature[@"location"];
                            NSMutableDictionary *rawLabelArea = rawFeature[@"label_area"];
                            NSMutableDictionary *rawProperties = rawFeature[@"properties"];
                            if (rawLocation && rawProperties)
                                rawProperties[@"location"] = rawLocation;
                            if (rawLabelArea && rawProperties)
                                rawProperties[@"label_area"] = rawLabelArea;
                        }
                    }
                    MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSONDictionary:jsonDict];
                    NSArray *features = [vecObj splitVectors];
                    
                    // Assign entity center lat/lon from the geometry
                    for (MaplyVectorObject *vObj in features) {
                        NSArray *entities = vObj.attributes[@"entities"];
                        if (entities) {
                            NSNumber *entityID = entities[0];
                            MaplyMicelloMapEntity *entity = self.entities[entityID];
                            if (entity) {
                                vObj.userObject = entity;
                                NSDictionary *location = vObj.attributes[@"location"];
                                NSArray *coords = location[@"coordinates"];
                                entity.lonDeg = ((NSNumber *)coords[0]).doubleValue;
                                entity.latDeg = ((NSNumber *)coords[1]).doubleValue;
                            }
                        }
                    }

                    dispatch_async(dispatch_get_main_queue(), ^{
                        level.features = features;
                        successBlock(level);
                    });
                });
            } else {
                failureBlock([[NSError alloc] initWithDomain:@"MaplyMicelloMap" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to fetch level GeoJSON file."}]);
            }
    }];
    [task resume];
}



- (void)startFetchZLevel:(int)zLevel success:(nonnull void (^)()) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    // Recursively call startFetchLevels:success:failure: for all levels corresponding to the z-level
    NSArray *levels = self.zLevelsToLevels[@(zLevel)];
    if (levels)
        [self startFetchLevels:levels success:successBlock failure:failureBlock];
    else
        failureBlock([[NSError alloc] initWithDomain:@"MaplyMicelloMap" code:0 userInfo:@{NSLocalizedDescriptionKey: @"startFetchZLevel: Invalid Z Level."}]);
   
}


- (void)startFetchLevels:(NSArray *__nonnull)levels success:(nonnull void (^)()) successBlock failure:(nullable void(^)(NSError *__nonnull error)) failureBlock {
    
    MaplyMicelloMapLevel *level = levels[0];
    NSArray *remainingLevels = [levels subarrayWithRange:NSMakeRange(1, levels.count-1)];
    [self startFetchLevel:level success:^(MaplyMicelloMapLevel * _Nullable level) {
        
        // Recursively call startFetchLevels:success:failure: for the remaining levels
        if (remainingLevels.count > 0)
            [self startFetchLevels:remainingLevels success:successBlock failure:failureBlock];
        else
            successBlock();
        
    } failure:^(NSError * _Nonnull error) {
        failureBlock(error);
    }];
    
}

- (MaplyScreenLabel *)labelForFeature:(MaplyVectorObject *)feature {
    NSDictionary *location = feature.attributes[@"location"];
    NSString *displayName = feature.attributes[@"display_name"];
    NSArray *labelArea = feature.attributes[@"label_area"];
    if (location && displayName) {
        NSString *locType = location[@"type"];
        NSArray *coords = location[@"coordinates"];
        if (locType && [locType isEqualToString:@"Point"] && coords) {
            MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
            double lonDeg = ((NSNumber *)coords[0]).doubleValue;
            double latDeg = ((NSNumber *)coords[1]).doubleValue;
            label.loc = MaplyCoordinateMakeWithDegrees(lonDeg, latDeg);
            label.text = displayName;
            label.layoutPlacement = kMaplyLayoutRight;
            label.userObject = feature;
            
            if (labelArea) {
                float width = ((NSNumber *)labelArea[2]).floatValue;
                float height = ((NSNumber *)labelArea[3]).floatValue;
                label.layoutImportance = width*height;
            } else
                label.layoutImportance = 0.0;
            return label;
        }
    }
    return nil;
}

- (unsigned int)styleIndexForFeature:(MaplyVectorObject *)feature {
    // Return the index of the first rule in the styleRules array that successfully matches the feature, or else returns the first invalid index (the size of the array)
    for (unsigned int i=0; i<_styleRules.count; i++) {
        MaplyMicelloStyleRule *styleRule = _styleRules[i];
        NSObject *value = feature.attributes[styleRule.key];
        if (!value)
            continue;
        if ([value isKindOfClass:[NSString class]] && [styleRule.value isKindOfClass:[NSString class]]) {
            if ([(NSString *)value isEqualToString:(NSString *)styleRule.value])
                return i;
        }
        if ([value isKindOfClass:[NSNumber class]] && [styleRule.value isKindOfClass:[NSNumber class]]) {
            if ([(NSNumber *)value isEqualToNumber:(NSNumber *)styleRule.value])
                return i;
        }
    }
    return (unsigned int)_styleRules.count;
}

- (NSDictionary *)vectorDescForStyleIndex:(unsigned int)styleIndex baseDesc:(NSDictionary *)baseDesc {
    if (styleIndex >= _styleRules.count)
        return baseDesc;
    // Apply style rule attributes onto the basic description dictionary.
    MaplyMicelloStyleRule *styleRule = _styleRules[styleIndex];
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:baseDesc];
    [desc addEntriesFromDictionary:styleRule.desc];
    return [NSDictionary dictionaryWithDictionary:desc];
}

- (void)setZLevel:(int)zLevel viewC:(MaplyBaseViewController *__nonnull)viewC {
    
    // Remove any vectors or labels that have been previously placed.
    if (self.zLevel != -1) {
        self.zLevel = -1;
        
        if (_highlightCompObj) {
            [viewC removeObject:_highlightCompObj];
            _highlightCompObj = nil;
        }
        if (_fillCompObjs.count > 0) {
            for (MaplyComponentObject *compObj in _fillCompObjs)
                [viewC removeObject:compObj];
            _fillCompObjs = [NSMutableArray array];
        }
        if (_outlinesCompObj) {
            [viewC removeObject:_outlinesCompObj];
            _outlinesCompObj = nil;
        }
        if (_labelsCompObj) {
            [viewC removeObject:_labelsCompObj];
            _labelsCompObj = nil;
        }

    }
    
    // Fetch the levels corresponding to the z-level.
    [self startFetchZLevel:zLevel success:^{
        
        // Add the vectors and labels
        
        NSArray *levels = self.zLevelsToLevels[@(zLevel)];
        
        NSDictionary *fillSelDesc = @{
                                      kMaplyColor:         self.fillColor,
                                      kMaplyDrawPriority:  @(_baseDrawPriority),
                                      kMaplyFilled:        @(YES),
                                      kMaplyVecCentered :  @(YES),
                                      kMaplyVecCenterX :   @(self.centerLonDeg * M_PI / 180.0),
                                      kMaplyVecCenterY :   @(self.centerLatDeg * M_PI / 180.0),
                                      kMaplySelectable :   @(YES),
                                      };
        
        NSDictionary *fillNoSelDesc = @{
                                        kMaplyColor:         self.fillColor,
                                        kMaplyDrawPriority:  @(_baseDrawPriority),
                                        kMaplyFilled:        @(YES),
                                        kMaplyVecCentered :  @(YES),
                                        kMaplyVecCenterX :   @(self.centerLonDeg * M_PI / 180.0),
                                        kMaplyVecCenterY :   @(self.centerLatDeg * M_PI / 180.0),
                                        kMaplySelectable :   @(NO),
                                        };
        
        
        NSDictionary *outlineDesc = @{
                                      kMaplyColor:           self.outlineColor,
                                      kMaplyDrawPriority:    @(_baseDrawPriority+20),
                                      kMaplyFilled:          @(NO),
                                      kMaplyVecWidth:        @(self.lineWidth),
                                      kMaplyVecCentered :    @(YES),
                                      kMaplyVecCenterX :     @(self.centerLonDeg * M_PI / 180.0),
                                      kMaplyVecCenterY :     @(self.centerLatDeg * M_PI / 180.0),
                                      kMaplySelectable :     @(NO),
                                      };
        
        NSDictionary *labelsDesc = @{
                                     kMaplyDrawPriority: @(_baseDrawPriority+30),
                                     kMaplyFont: [UIFont systemFontOfSize:24.0],
                                     kMaplyTextColor: [UIColor darkGrayColor]
                                     };
        
        NSMutableArray *selFeaturesArrays = [NSMutableArray array];
        NSMutableArray *noSelFeaturesArrays = [NSMutableArray array];
        NSMutableArray *outlineFeatures = [NSMutableArray array];
        NSMutableArray *labels = [NSMutableArray array];
        
        for (int i=0; i<_styleRules.count+1; i++) {
            // Once for each style rule, and once more for no match
            [selFeaturesArrays addObject:[NSMutableArray array]];
            [noSelFeaturesArrays addObject:[NSMutableArray array]];
        }
        
        // For all levels matching the desired z-level, sort features by which style rule will apply.
        for (MaplyMicelloMapLevel *level in levels) {
        
            for (MaplyVectorObject *feature in level.features) {
                [outlineFeatures addObject:feature];
                int styleIndex = [self styleIndexForFeature:feature];
                
                if (feature.attributes[@"entities"]) {
                    [((NSMutableArray *)selFeaturesArrays[styleIndex]) addObject:feature];
                    MaplyScreenLabel *label = [self labelForFeature:feature];
                    if (label)
                        [labels addObject:label];
                } else
                    [((NSMutableArray *)noSelFeaturesArrays[styleIndex]) addObject:feature];
            }
        }
        
        // Add sorted vector features to map.
        for (int i=0; i<_styleRules.count+1; i++) {
            MaplyComponentObject *selCompObj = [viewC addVectors:((NSMutableArray *)selFeaturesArrays[i]) desc:[self vectorDescForStyleIndex:i baseDesc:fillSelDesc]];
            [_fillCompObjs addObject:selCompObj];
            MaplyComponentObject *noSelCompObj = [viewC addVectors:((NSMutableArray *)noSelFeaturesArrays[i]) desc:[self vectorDescForStyleIndex:i baseDesc:fillNoSelDesc]];
            [_fillCompObjs addObject:noSelCompObj];
        }
        
        _outlinesCompObj = [viewC addVectors:outlineFeatures desc:outlineDesc];
        _labelsCompObj = [viewC addScreenLabels:labels desc:labelsDesc];
        
        self.zLevel = zLevel;
        
    } failure:^(NSError * _Nonnull error) {
        
    }];
}


- (MaplyMicelloMapEntity *__nullable)select:(NSObject *__nonnull) selectedObj viewC:(MaplyBaseViewController *__nonnull)viewC {
    
    MaplyVectorObject *vObj;
    if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
        vObj = (MaplyVectorObject *)selectedObj;
    else if ([selectedObj isKindOfClass:[MaplyScreenLabel class]])
        vObj = ((MaplyScreenLabel *)selectedObj).userObject;
    
    if (!vObj)
        return nil;
    MaplyMicelloMapEntity *entity = (MaplyMicelloMapEntity *)vObj.userObject;
    if (!entity)
        return nil;

    // Clear any previous selection outline
    if (_highlightCompObj) {
        [viewC removeObject:_highlightCompObj];
        _highlightCompObj = nil;
    }

    NSDictionary *outlineDesc = @{
                                  kMaplyColor:           self.selectedOutlineColor,
                                  kMaplyDrawPriority:    @(_baseDrawPriority+40),
                                  kMaplyFilled:          @(NO),
                                  kMaplyVecWidth:        @(self.selectedLineWidth),
                                  kMaplyVecCentered :    @(YES),
                                  kMaplyVecCenterX :     @(self.centerLonDeg * M_PI / 180.0),
                                  kMaplyVecCenterY :     @(self.centerLatDeg * M_PI / 180.0),
                                  kMaplySelectable :     @(NO),
                                  };
    
    // Add selection outline for selected vector feature
    _highlightCompObj = [viewC addVectors:@[vObj] desc:outlineDesc];

    // Return entity corresponding to selected vector feature
    return entity;
}

- (void)clearSelectionViewC:(MaplyBaseViewController *__nonnull)viewC {
    // Clear any previous selection outline
    if (_highlightCompObj) {
        [viewC removeObject:_highlightCompObj];
        _highlightCompObj = nil;
    }
}

@end






