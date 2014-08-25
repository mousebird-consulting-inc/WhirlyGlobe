/*
 *  MaplyVectorStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
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

#import <UIKit/UIKit.h>
#import "MaplyQuadPagingLayer.h"

/** @brief Settings that control how vector tiles look in relation to their styles.
    @details These are set based on the sort of device we're on, particularly retina vs. non-retina.  They can be manipulated directly as well for your needs.
  */
@interface MaplyVectorTileStyleSettings : NSObject

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

/// @brief If we're using widened vectors, only active them for strokes wider than this.  Defaults to zero.
@property (nonatomic) float wideVecCuttoff;

/// @brief The default font family for all text
@property (nonatomic,strong) NSString *fontName;

@end

/** The Maply Vector Tile Style is an internal representation of the style JSON coming out
    of a Maply Vector Tile database.
  */
@interface MaplyVectorTileStyle : NSObject

/** @brief Construct a style entry from an NSDictionary.
  */
+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC;

/// @brief Unique Identifier for this style
@property (nonatomic) id<NSCopying> uuid;

/// @brief Set if this geometry is additive (e.g. sticks around) rather than replacement
@property (nonatomic) bool geomAdditive;

/// @brief Construct a style entry from an NSDictionary
- (id)initWithStyleEntry:(NSDictionary *)styleEntry viewC:(MaplyBaseViewController *)viewC;

/// Turn the min/maxscaledenom into height ranges for minVis/maxVis
- (void)resolveVisibility:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings desc:(NSMutableDictionary *)desc;

/// @brief Construct objects related to this style based on the input data.
- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;

/// @brief parse a mapnik style template string
- (NSString*)formatText:(NSString*)formatString forObject:(MaplyVectorObject*)vec;

/// @brief The view controller we're constructing objects in
@property (nonatomic,weak) MaplyBaseViewController *viewC;

/// @brief If set, we create selectable objects
/// @details This controls whether the objects we create are selectable.  Off by default.
@property (nonatomic) bool selectable;

@end
