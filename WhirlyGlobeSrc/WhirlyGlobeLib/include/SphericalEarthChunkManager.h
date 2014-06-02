/*
 *  SphericalEarthChunkManager.h
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
#import <queue>
#import "WhirlyVector.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "TileQuadLoader.h"
#import "Scene.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"

/** This defines a chunk of the globe to overlay with a single
 image.  In general you should use one of the quad layers
 instead.  This is here if you need to control data loading
 image by image, presumably with an active layer.
 */
@interface WhirlyKitSphericalChunk : NSObject

/// Bounding box for the chunk to display
@property (nonatomic,assign) WhirlyKit::Mbr &mbr;
/// Texture we'll wrap over the top
@property (nonatomic,assign) std::vector<WhirlyKit::SimpleIdentity> &texIDs;
/// Format we'll store the textures in
@property (nonatomic) WhirlyKitTileImageType imageFormat;
/// If no texture, we can pass in a UIImage (or NSData that contains common formats).
/// The implication here is that we're going to stick these in an atlas.
@property (nonatomic) WhirlyKitLoadedImage *loadImage;
/// If set, the shader problem we'll use to draw this
@property (nonatomic) WhirlyKit::SimpleIdentity programID;
/// Z offset for the generated geometry
@property (nonatomic,assign) float drawOffset;
/// Sorting priority for the generated geometry
@property (nonatomic,assign) int drawPriority;
/// Sampling along X and Y.
/// If the eps is set, this is the maximum sampling in x/y
@property (nonatomic,assign) int sampleX,sampleY;
/// When eps is set, this is the minimum sampling in x/y
@property (nonatomic,assign) int minSampleX,minSampleY;
/// If not doing static sampling, break it down until its no farther than this from the globe.
/// sampleX,sampleY become maximums
@property (nonatomic,assign) float eps;
/// Chunk is visible this far down
@property (nonatomic,assign) float minVis;
/// Chunk is visible this far out
@property (nonatomic,assign) float maxVis;
/// Distance from the min visible range to start fading
@property (nonatomic,assign) float minVisBand;
/// Distance from the max visible range to start fading
@property (nonatomic,assign) float maxVisBand;
/// Rotation around the middle of the chunk
@property (nonatomic,assign) float rotation;
/// This chunk takes the z buffer into account
@property (nonatomic,assign) bool readZBuffer;
/// This chunk writes itself to the z buffer
@property (nonatomic,assign) bool writeZBuffer;
/// The chunks extents are in this coordinate system.  Geographic if not set.
@property (nonatomic,assign) WhirlyKit::CoordSystem *coordSys;

// Create one or more drawables to represent the chunk.
// Only call this if you know what you're doing
- (void)buildDrawable:(WhirlyKit::BasicDrawable **)draw skirtDraw:(WhirlyKit::BasicDrawable **)skirtDraw enabled:(bool)enable adapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter;

@end

// Used to pass chunks between main and layer threads
@interface WhirlyKitSphericalChunkInfo : NSObject
{
@public
    bool enable;
    WhirlyKit::SimpleIdentity chunkId;
    WhirlyKitSphericalChunk *chunk;
}
@end

namespace WhirlyKit
{
    
class ChunkSceneRep;
typedef boost::shared_ptr<ChunkSceneRep> ChunkSceneRepRef;
typedef std::set<ChunkSceneRepRef,IdentifiableRefSorter> ChunkRepSet;
 
// Used to track requests that come in so we can queue them
typedef enum {ChunkAdd,ChunkRemove,ChunkEnable,ChunkDisable} ChunkRequestType;
class ChunkRequest
{
public:
    ChunkRequest() { }
    ChunkRequest(ChunkRequestType type,WhirlyKitSphericalChunkInfo *chunkInfo,WhirlyKitSphericalChunk *chunk) :
    type(type), chunkId(EmptyIdentity), chunkInfo(chunkInfo), chunk(chunk) { }
    ChunkRequest(ChunkRequestType type,SimpleIdentity chunkId) :
    type(type), chunkId(chunkId), chunkInfo(NULL), chunk(NULL), doEdgeMatching(false) { }
    ChunkRequestType type;
    SimpleIdentity chunkId;
    WhirlyKitSphericalChunkInfo *chunkInfo;
    WhirlyKitSphericalChunk *chunk;
    bool doEdgeMatching;
};

#define kWKSphericalChunkManager "WKSphericalChunkManager"
    
/** The spherical chunk manager handles the geometry associated with
    spherical chunks.  This is thread safe, except for the destructor.
  */
class SphericalChunkManager : public SceneManager
{
public:
    SphericalChunkManager();
    virtual ~SphericalChunkManager();
    
    /// If we're using texture atlases, pass those in
    void setAtlases(DynamicTextureAtlas *inTexAtlas,DynamicDrawableAtlas *inDrawAtlas) { texAtlas = inTexAtlas;  drawAtlas = inDrawAtlas; }
    void setBorderTexel(int inBorderTexel) { borderTexel = inBorderTexel; }
    
    /// Add the given chunk (enabled or disabled)
    SimpleIdentity addChunk(WhirlyKitSphericalChunk *chunk,bool doEdgeMatching,bool enable,ChangeSet &changes);
    
    /// Modify the given chunk (new texture IDs)
    bool modifyChunkTextures(SimpleIdentity chunkID,const std::vector<SimpleIdentity> &texIDs,ChangeSet &changes);
    
    /// Enable or disable the given chunk
    void enableChunk(SimpleIdentity chunkID,bool enable,ChangeSet &changes);
    
    /// Remove the given chunks
    void removeChunks(SimpleIDSet &chunkIDs,ChangeSet &changes);
    
    /// Number of chunks we're representing
    int getNumChunks();
    
    /// Process outstanding requests
    void processRequests(ChangeSet &changes);
    
protected:
    void processChunkRequest(ChunkRequest &request,ChangeSet &changes);
    
    pthread_mutex_t repLock;
    ChunkRepSet chunkReps;
    pthread_mutex_t requestLock;
    // Outstanding requests to process
    std::queue<ChunkRequest> requests;
    int borderTexel;
    pthread_mutex_t atlasLock;
    DynamicTextureAtlas *texAtlas;
    DynamicDrawableAtlas *drawAtlas;
};

}
