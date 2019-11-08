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

typedef enum {MBTextCenter,MBTextLeft,MBTextRight,MBTextTop,MBTextBottom,MBTextTopLeft,MBTextTopRight,MBTextBottomLeft,MBTextBottomRight} MapboxTextAnchor;
typedef enum {MBPlacePoint,MBPlaceLine} MapboxSymbolPlacement;
typedef enum {MBTextTransNone,MBTextTransUppercase,MBTextTransLowercase} MapboxTextTransform;

/** Controls the layout logic for Mapbox Symbols.
  */
@interface MapboxVectorSymbolLayout : NSObject

@property (nonatomic) bool visible;
/// How we place the symbole (at a point, or along a line)
@property (nonatomic) MapboxSymbolPlacement placement;
/// If set, turn the text uppercase
@property (nonatomic) MapboxTextTransform textTransform;
/// @brief Field to use when displaying the text
@property (nonatomic,strong) NSArray<NSString *> *textFields;
/// @brief Font to use for display
@property (nonatomic,strong) NSString *textFontName;
/// @brief The maximum line width for wrapping
@property (nonatomic) double textMaxWidth;
/// If set, a function controlling max width
@property (nonatomic,strong) MaplyVectorFunctionStops *textMaxWidthFunc;
/// If set, the immutable text size
@property (nonatomic) double textSize;
/// Text scale from the global settings
@property (nonatomic) double globalTextScale;
/// If set, a function that controls text size
@property (nonatomic,strong) MaplyVectorFunctionStops *textSizeFunc;
/// How the text is laid out in relation to it's attach point
@property (nonatomic) MapboxTextAnchor textAnchor;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

@interface MapboxVectorSymbolPaint : NSObject

// Default text color
@property (nonatomic,strong) UIColor *textColor;
// Possibly a function to describe the text color
@property (nonatomic) MaplyVectorFunctionStops *textColorFunc;
// Opacity
@property (nonatomic,assign) double textOpacity;
// Optional function to describe text opacity
@property (nonatomic) MaplyVectorFunctionStops *textOpacityFunc;
// If there's a halo, this is the color
@property (nonatomic,strong) UIColor *textHaloColor;
// If there's a halo, this is the size
@property (nonatomic) double textHaloWidth;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

/// @brief Icons and symbols
@interface MapboxVectorLayerSymbol : MaplyMapboxVectorStyleLayer

@property (nonatomic,strong) MapboxVectorSymbolLayout *layout;
@property (nonatomic,strong) MapboxVectorSymbolPaint *paint;
/// If set, only one label with its text will be displayed.  Sorted out by the layout manager.
@property (nonatomic) bool uniqueLabel;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end
