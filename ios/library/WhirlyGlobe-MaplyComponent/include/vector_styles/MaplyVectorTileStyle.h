/*  MaplyVectorTileStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import <UIKit/UIKit.h>
#import "MaplyVectorStyle.h"

/** The Maply Vector Tile Style is an internal representation of the style JSON coming out
    of a Maply Vector Tile database.
  */
@interface MaplyVectorTileStyle : NSObject<MaplyVectorStyle>

/** 
    Construct a style entry from an NSDictionary.
  */
+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry settings:(MaplyVectorStyleSettings *)settings viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/// Unique Identifier for this style
@property (nonatomic) long long uuid;

/// Set if this geometry is additive (e.g. sticks around) rather than replacement
@property (nonatomic) bool geomAdditive;

/// Construct a style entry from an NSDictionary
- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/// Turn the min/maxscaledenom into height ranges for minVis/maxVis
- (void)resolveVisibility:(NSDictionary *)styleEntry settings:(MaplyVectorStyleSettings *)settings desc:(NSMutableDictionary *)desc;

/// parse a mapnik style template string
- (NSString*)formatText:(NSString*)formatString forObject:(MaplyVectorObject*)vec;

/// The view controller we're constructing objects in
@property (nonatomic,weak) NSObject<MaplyRenderControllerProtocol> *viewC;

/// If set, we create selectable objects
/// This controls whether the objects we create are selectable.  Off by default.
@property (nonatomic) bool selectable;

/// Parse the various types of color strings
+ (UIColor *) ParseColor:(NSString *)colorStr;

/// Parse an RGB color and fill in the alpha
+ (UIColor *) ParseColor:(NSString *)colorStr alpha:(CGFloat)alpha;

@end
