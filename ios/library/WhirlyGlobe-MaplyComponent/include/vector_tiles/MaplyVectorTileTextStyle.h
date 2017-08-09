/*
 *  MaplyVectorTextStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyVectorTiles.h"
#import "MaplyVectorStyle.h"
#import "MaplyVectorTileStyle.h"
#import "MaplyVectorTileStyle.h"

/** 
    Implementation of the text style symbolizer for Maply Vector Tiles.
 */
@interface MaplyVectorTileStyleText : MaplyVectorTileStyle

- (instancetype _Nullable)initWithStyleEntry:(NSDictionary * _Nonnull)styleEntry settings:(MaplyVectorStyleSettings * _Nonnull)settings viewC:(MaplyBaseViewController * _Nonnull)viewC;

/// Construct objects related to this style based on the input data.
- (NSArray * __nullable )buildObjects:(NSArray * _Nonnull)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController * _Nonnull)viewC;


@end
