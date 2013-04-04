/*
 *  SphericalEarthChunkLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/29/12.
 *  Copyright 2011-2012 mousebird consulting
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

/** This defines a chunk of the globe to overlay with a single
    image.  In general you should use one of the quad layers
    instead.  This is here if you need to control data loading
    image by image, presumably with an active layer.
  */
@interface WhirlyKitSphericalChunk : NSObject
{
    /// Bounding box for the chunk to display
    WhirlyKit::GeoMbr mbr;
    /// Texture we'll wrap over the top
    WhirlyKit::SimpleIdentity texId;
    /// Z offset for the generated geometry
    float drawOffset;
    /// Sorting priority for the generated geometry
    int drawPriority;
    /// Sampling along X and Y
    int sampleX,sampleY;
    /// Chunk is visible this far down
    float minVis;
    /// Chunk is visible this far out
    float maxVis;
    /// Distance from the min visible range to start fading
    float minVisBand;
    /// Distance from the max visible range to start fading
    float maxVisBand;
    /// Rotation around the middle of the chunk
    float rotation;
}

@property (nonatomic,assign) WhirlyKit::GeoMbr &mbr;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity texId;
@property (nonatomic,assign) float drawOffset;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) int sampleX,sampleY;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) float minVisBand,maxVisBand;
@property (nonatomic,assign) float rotation;

@end

/** The Spherical Chunk layer
  */
@interface WhirlyKitSphericalChunkLayer : NSObject<WhirlyKitLayer>
{
    bool ignoreEdgeMatching;
}

@property (nonatomic,assign) bool ignoreEdgeMatching;

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
