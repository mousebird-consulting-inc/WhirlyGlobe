/*
 *  MaplyTextureAtlas_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/11/14.
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

#import <Foundation/Foundation.h>
#import <WhirlyGlobe.h>
#import "MaplyTexture_private.h"
#import "MaplyQuadImageTilesLayer.h"

// Manages a whole group of texture atlases.  Thread safe.
@interface MaplyTextureAtlasGroup : NSObject

// Construct with the OpenGL memory manager
- (id)initWithScene:(WhirlyKit::Scene *)scene;

// Size of the texture atlases we'll create
- (void)setSize:(int)size;

// Add a texture to an atlas and return
- (bool)addTexture:(WhirlyKit::Texture *)tex subTex:(WhirlyKit::SubTexture &)subTex changes:(WhirlyKit::ChangeSet &)changes;

// Remove a texture from the atlas group
- (void)removeTexture:(WhirlyKit::SimpleIdentity)subTexId changes:(WhirlyKit::ChangeSet &)changes;

// Shut down the atlas group and clean up the associated memory
- (void)clear:(WhirlyKit::ChangeSet &)changes;

@end
