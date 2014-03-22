/*
 *  MaplyVectorTiles.h
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
#import "MaplyVectorStyle.h"

/** A Maply Vector Tiles database contains tiled vector data that
    can be displayed using styles contained within the database.
    The database will act as a paging delegate and is usually hooked
    up to a MaplyPagingLayer to do its thing.
  */
@interface MaplyVectorTiles : NSObject<MaplyPagingDelegate>

/** @brief Parse a color in #rrggbbaa format out of a string.
  */
+ (UIColor *)ParseColor:(NSString *)colorStr;

/** @brief Initialize with a local tiles database and a view controller to display to.
    @details This will start up a maply vector tiles object reading from the given database and building objects in the given view controller.
    @details The vector database will respond to the MaplyPagingDelegate and pull in tiles as needed for display.
  */
- (id)initWithDatabase:(NSString *)tilesDB viewC:(MaplyBaseViewController *)viewC;

/// @brief The minimum level this database covers
@property (nonatomic,readonly) int minLevel;

/// @brief The maximum level this database covers
@property (nonatomic,readonly) int maxLevel;

/// @brief The view controller the vector database paging builds its objects in
@property (nonatomic,weak) MaplyBaseViewController *viewC;

/// @brief Settings that control how objects are built with relation to tiles
@property (nonatomic) MaplyVectorTileStyleSettings *settings;

/// @brief Individual layers parsed out of the vector tiles database
@property (nonatomic,readonly) NSArray *layerNames;

/// @brief An array of the style dictionaries.
/// @details Style dictionaries are used internally to style the vector data.
@property (nonatomic,readonly) NSArray *styles;

@end
