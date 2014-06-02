/*
 *  SphericalEarthChunkLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/29/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import <math.h>
#import "WhirlyVector.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TileQuadLoader.h"
#import "SphericalEarthChunkManager.h"

/** The Spherical Chunk layer
  */
@interface WhirlyKitSphericalChunkLayer : NSObject<WhirlyKitLayer>

/// If set, we'll turn off skirts
@property (nonatomic,assign) bool ignoreEdgeMatching;
/// If set we'll use a dynamic texture and drawable atlas
@property (nonatomic,assign) bool useDynamicAtlas;

/// Add a single chunk on the spherical earth.  This returns and ID
///  we can use to remove it later.
- (WhirlyKit::SimpleIdentity)addChunk:(WhirlyKitSphericalChunk *)chunk enable:(bool)enable;

/// Remove the named chunk
- (void)removeChunk:(WhirlyKit::SimpleIdentity)chunkId;

/// Disable/enable the named chunk
- (void)toogleChunk:(WhirlyKit::SimpleIdentity)chunkId enable:(bool)enable;

/// Return the number of active chunks.  Only works in the layer thread.
- (int)numChunk;

@end
