/*
 *  MaplyVectorStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import <UIKit/UIKit.h>
#import "MaplyQuadPagingLayer.h"

/** @brief Settings that control how vector tiles look in relation to their styles.
    @details These are set based on the sort of device we're on, particularly retina vs. non-retina.  They can be manipulated directly as well for your needs.
  */
@interface MaplyVectorStyleSettings : NSObject

/// @brief Line widths will be scaled by this amount before display.
@property (nonatomic) float lineScale;
/// @brief Text sizes will be scaled by this amount before display.
@property (nonatomic) float textScale;
/// @brief Markers will be scaled by this amount before display.
@property (nonatomic) float markerScale;
/// @brief Importance for markers in the layout engine
@property (nonatomic) float markerImportance;
/// @brief Default marker size when none is specified
@property (nonatomic) float markerSize;

/** @brief The overall map scale calculations will be scaled by this amount.
    @details We use the map scale calculations to figure out what is dispalyed and when.  Not what to load in, mind you, that's a separate, but related calculation.  This controls the scaling of those calculations.  Scale it down to load things in later, up to load them in sooner.
  */
@property (nonatomic) float mapScaleScale;

/// @brief Dashed lines will be scaled by this amount before display.
@property (nonatomic) float dashPatternScale;

/// @brief Use widened vectors (which do anti-aliasing and such)
@property (nonatomic) bool useWideVectors;

/// @brief Where we're using old vectors (e.g. not wide) scale them by this amount
@property (nonatomic) float oldVecWidthScale;

/// @brief If we're using widened vectors, only active them for strokes wider than this.  Defaults to zero.
@property (nonatomic) float wideVecCuttoff;

/// @brief If set, we'll make the areal features selectable.  If not, this saves memory.
@property (nonatomic) bool selectable;

/// @brief If set, icons will be loaded from this directory
@property (nonatomic, strong) NSString * _Nullable iconDirectory;

/// @brief The default font family for all text
@property (nonatomic,strong) NSString * _Nullable fontName;

@end

@protocol MaplyVectorStyle;

/** @brief Protocol for styling the vectors.
 @details You pass in an object which adheres to this protocol and will style
 the vectors read by a MaplyMapnikVectorTiles object.  In general, this will be
 a parsed Mapnik vector file, but you can substitute your own logic as well.
 */
@protocol MaplyVectorStyleDelegate <NSObject>

/** @brief Return the styles that apply to the given feature (attributes).
 */
- (nullable NSArray *)stylesForFeatureWithAttributes:(NSDictionary *__nonnull)attributes
                                              onTile:(MaplyTileID)tileID
                                             inLayer:(NSString *__nonnull)layer
                                               viewC:(MaplyBaseViewController *__nonnull)viewC;

/// @brief Return true if the given layer is meant to display for the given tile (zoom level)
- (BOOL)layerShouldDisplay:(NSString *__nonnull)layer tile:(MaplyTileID)tileID;

/// @brief Return the style associated with the given UUID.
- (nullable NSObject<MaplyVectorStyle> *)styleForUUID:(NSString *__nonnull)uiid viewC:(MaplyBaseViewController *__nonnull)viewC;

@end

/** @brief Base protocol for the vector styles.
    @details Maply Vector Style is the protocol the your vector style needs to
    implement for the vector tile parsers to recognize it.
  */
@protocol MaplyVectorStyle<NSObject>

/// @brief Unique Identifier for this style
- (NSString * _Nonnull) uuid;

/// @brief Set if this geometry is additive (e.g. sticks around) rather than replacement
- (bool) geomAdditive;

/// @brief Construct objects related to this style based on the input data.
- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController * _Nonnull)viewC;

@end
