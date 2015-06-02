/*
 *  MapboxVectorStyleSymbol.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
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

#import "MapboxVectorStyleSet.h"

@interface MapboxVectorSymbolLayout : NSObject

/// @brief Field to use when displaying the text
@property (nonatomic) NSString *textField;
/// @brief the biggest we'll let the text get (Note: Is this font or what?)
@property (nonatomic) double textMaxSize;

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC;

@end

@interface MapboxVectorSymbolPaint : NSObject

@property (nonatomic) UIColor *textColor;
@property (nonatomic) UIColor *textHaloColor;

@property (nonatomic) double textSize;
@property (nonatomic) MaplyVectorFunctionStops *textSizeFunc;

- (id)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MaplyMapboxVectorStyleSet *)styleSet viewC:(MaplyBaseViewController *)viewC;

@end

/// @brief Icons and symbols
@interface MapboxVectorLayerSymbol : MaplyMapboxVectorStyleLayer

@property (nonatomic) MapboxVectorSymbolLayout *layout;
@property (nonatomic) MapboxVectorSymbolPaint *paint;

- (id)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MaplyMapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *)viewC;

- (NSArray *)buildObjects:(NSArray *)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *)viewC;

@end
