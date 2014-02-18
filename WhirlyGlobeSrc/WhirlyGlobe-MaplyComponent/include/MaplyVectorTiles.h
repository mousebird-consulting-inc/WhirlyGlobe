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

/**
  */
@interface MaplyVectorTiles : NSObject<MaplyPagingDelegate>

+ (UIColor *)ParseColor:(NSString *)colorStr;

/** @brief
  */
- (id)initWithDatabase:(NSString *)tilesDB viewC:(MaplyBaseViewController *)viewC;

@property (nonatomic,readonly) NSString *tilesDir;
@property (nonatomic,readonly) int minLevel;
@property (nonatomic,readonly) int maxLevel;

@property (nonatomic,weak) MaplyBaseViewController *viewC;

@property (nonatomic) MaplyVectorTileStyleSettings *settings;

/// @brief Individual layers parsed out of the vector tiles database
@property (nonatomic,readonly) NSArray *layerNames;

/// @brief An array of the style dictionaries.
/// @details Style dictionaries are used internally to style the vector data.
@property (nonatomic,readonly) NSArray *styles;

@end
