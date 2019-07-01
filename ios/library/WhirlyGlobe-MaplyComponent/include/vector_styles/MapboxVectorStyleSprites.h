/*
 *  MapboxVectorStyleSprites.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/3/19.
 *  Copyright 2011-2019 mousebird consulting
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
#import <control/MaplyBaseViewController.h>
#import "vector_styles/MaplyVectorStyle.h"
#import "vector_tiles/MapboxVectorTiles.h"

/** @brief Holds the sprite information for sub-pieces of a larger sprite image.
    @details Style sheets can refer to icons by name within a sprite sheet.
    This object parses that sprite sheet and generates sub-texture references
    on demand.
  **/
@interface MapboxVectorStyleSprites : NSObject

/// @brief Initialize with the sprite JSON and image
- (id __nullable)initWithJSON:(NSData * __nonnull)spriteJSON image:(UIImage * __nonnull)spriteIMG settings:(MaplyVectorStyleSettings * __nonnull)settings viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

/// @brief Get a reference to the overall sprite texture if you need it.
- (MaplyTexture * __nullable)getWholeTexture;

/// @brief Return a MaplyTexture corresponding to the given name.
- (MaplyTexture * __nullable)getTexture:(NSString * __nonnull)spriteName;

/// @brief Cleans up any view controller assets (e.g. textures)
- (void)shutdown;

@end
