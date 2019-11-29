/*
 *  MapboxVectorTilesImageDelegate.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on January 24 2018
 *  Copyright 2011-2018 Saildrone
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
#import "MaplyQuadPagingLayer.h"
#import "MaplyTileSource.h"
#import "MaplyCoordinate.h"
#import "MaplyVectorStyle.h"
#import "MapboxVectorTiles.h"
#import "MaplyQuadImageLoader.h"

@class MapboxVectorStyleSet;

/**
 A tile source for Mapbox vector tiles that renders into images (partially).
 
 This tile source will render some data into images for use by the QuadImages layer and then
 */
@interface MapboxVectorImageInterpreter : NSObject<MaplyLoaderInterpreter>

/* This variant takes two styles, one for the image data and one
   for the overlaid vectors.  It also needs an offline renderer to
   render to the image
  */
- (instancetype _Nullable ) initWithLoader:(MaplyQuadImageLoader *__nonnull)loader
                                imageStyle:(MapboxVectorStyleSet *__nonnull)imageStyle
                               offlineRender:(MaplyRenderController *__nonnull)renderControl
                                 vectorStyle:(MapboxVectorStyleSet *__nonnull)vectorStyle
                                       viewC:(MaplyBaseViewController *__nonnull)viewC;

/* This variant just needs a style for the vectors.  It will provide an image the
    color of the background for the image loader.
 */
- (instancetype _Nullable ) initWithLoader:(MaplyQuadImageLoader *__nonnull)loader
                                     style:(MapboxVectorStyleSet *__nonnull)vectorStyle
                                     viewC:(MaplyBaseViewController *__nonnull)viewC;

@end
