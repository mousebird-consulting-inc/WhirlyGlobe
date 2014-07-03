/*
 *  MapnikXmlStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Jesse Crocker, Trailbehind inc. on 3/31/14.
 *  Copyright 2011-2014 mousebird consulting
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
#import "MaplyMapnikVectorTiles.h"

@class MaplyVectorTileStyleSettings;

@interface MapnikStyleSet : NSObject <NSXMLParserDelegate, VectorStyleDelegate>

@property (nonatomic, strong) MaplyVectorTileStyleSettings *tileStyleSettings;
@property (nonatomic, strong) NSMutableDictionary *styleDictionary;
@property (nonatomic, weak) MaplyBaseViewController *viewC;
@property (nonatomic, readonly) BOOL parsing;
@property (nonatomic, strong) UIColor *backgroundColor;
@property (nonatomic, assign) NSInteger tileMaxZoom;
@property (nonatomic, assign) NSInteger drawPriorityOffset;
@property (nonatomic, assign) CGFloat alpha;

- (instancetype)initForViewC:(MaplyBaseViewController *)viewC;
- (void)loadXmlFile:(NSString*)filePath;
- (void)loadXmlData:(NSData *)docData;
- (void)loadJsonData:(NSData *)jsonData;
- (void)loadJsonFile:(NSString*)filePath;
- (void)saveAsJSON:(NSString *)filePath;
- (void)generateStyles;

@end
