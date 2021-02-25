/*
 *  MapboxVectorTilesImageDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on January 24 2018
 *  Copyright 2011-2019 Saildrone
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
#import "loading/MaplyTileSourceNew.h"
#import "math/MaplyCoordinate.h"
#import "vector_styles/MaplyVectorStyle.h"
#import "vector_tiles/MapboxVectorTiles.h"
#import "loading/MaplyQuadImageFrameLoader.h"

@class MapboxVectorStyleSet;

/**
 An interpreter for Mapbox Vector Tiles.
 
 This will turn vector tiles into images, visual objects, or a combination of the two.  Loader interpreters like
    this one can be used by Loaders that talk to ondevice objects (such as MBTiles files) or remote tile
    sources.
 */
@interface MapboxVectorInterpreter : NSObject<MaplyLoaderInterpreter>

/** This version of the init takes an image style set, a vector style set,
    and an offline renderer to build the image tiles.
 
    Image tiles will be used as a background and vectors put on top of them.
    This is very nice for the globe, but requires specialized style sheets.
  */
- (instancetype _Nullable ) initWithImageStyle:(NSObject<MaplyVectorStyleDelegate> *__nonnull)imageStyle
                         offlineRender:(MaplyRenderController *__nonnull)renderControl
                           vectorStyle:(NSObject<MaplyVectorStyleDelegate> *__nonnull)vectorStyle
                                 viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/** This version of the init builds visual features for vector tiles.
 
    This interpreter can be used as overlay data or a full map, depending
    on how your style is configured.
  */
- (instancetype __nullable) initWithVectorStyle:(NSObject<MaplyVectorStyleDelegate> *__nonnull)vectorStyle
                                          viewC:(NSObject<MaplyRenderControllerProtocol> *__nonnull)viewC;

/**
 Set an optional list of unique features we'll filter on.
 Any feature we want to pass through must have the given attribute name and one of the values.
 */
- (void)setUUIDName:(NSString * __nonnull)uuidName uuidValues:(NSArray<NSString *> * __nonnull)uuids;

@end
