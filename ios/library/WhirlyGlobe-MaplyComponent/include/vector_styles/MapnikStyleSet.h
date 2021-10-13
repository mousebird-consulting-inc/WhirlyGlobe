/*
 *  MapnikXmlStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2019 mousebird consulting
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
#import "MapboxVectorTiles.h"

@class MaplyVectorStyleSettings;

@interface MapnikStyleSet : NSObject <NSXMLParserDelegate, MaplyVectorStyleDelegate>

@property (nonatomic, strong, nullable) MaplyVectorStyleSettings *tileStyleSettings;
@property (nonatomic, strong, nullable) NSMutableDictionary *styleDictionary;
@property (nonatomic, weak, nullable) NSObject<MaplyRenderControllerProtocol> *viewC;
@property (nonatomic, readonly) BOOL parsing;
@property (nonatomic, strong, nullable) UIColor *backgroundColor;
@property (nonatomic, assign) NSInteger tileMaxZoom;
@property (nonatomic, assign) NSInteger drawPriorityOffset;
@property (nonatomic, assign) CGFloat alpha;

- (nonnull instancetype)initForViewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

- (void)loadXmlFile:(NSString *__nonnull)filePath;
- (void)loadXmlData:(NSData *__nonnull)docData;
- (void)loadJsonData:(NSData *__nonnull)jsonData;
- (void)loadJsonFile:(NSString*__nonnull)filePath;
- (void)saveAsJSON:(NSString *__nonnull)filePath;
- (void)generateStyles;

@end
