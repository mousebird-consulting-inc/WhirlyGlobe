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
#import "TileQuadLoader.h"
#import "Scene.h"
#import "DynamicTextureAtlas.h"
#import "DynamicDrawableAtlas.h"
#import "BaseInfo.h"

namespace WhirlyKit
{

// Info object for spherical chunks
class SphericalChunkInfo : public BaseInfo
{
public:
    SphericalChunkInfo(const Dictionary &dict);
    
    RGBAColor color;
    bool readZBuffer,writeZBuffer;
    bool doEdgeMatching;
};

/** This defines a chunk of the globe to overlay with a single
 image.  In general you should use one of the quad layers
 instead.  This is here if you need to control data loading
 image by image, presumably with an active layer.
 */
class SphericalChunk : public Identifiable
{
    friend class SphericalChunkManager;
public:
    
    SphericalChunk();

    /// Bounding box for the chunk to display
    Mbr mbr;

    /// Texture we'll wrap over the top
    std::vector<SimpleIdentity> texIDs;

    /// Format we'll store the textures in
    TileImageType imageFormat;

    /// If no texture, we can pass in a UIImage (or NSData that contains common formats).
    /// The implication here is that we're going to stick these in an atlas.
    LoadedImage *loadImage;

    /// If set, the shader program we'll use to draw this
    SimpleIdentity programID;

    /// Sampling along X and Y.
    /// If the eps is set, this is the maximum sampling in x/y
    int sampleX,sampleY;

    /// When eps is set, this is the minimum sampling in x/y
    int minSampleX,minSampleY;

    /// If not doing static sampling, break it down until its no farther than this from the globe.
    /// sampleX,sampleY become maximums
    float eps;

    /// Rotation around the middle of the chunk
    float rotation;
    
    /// The chunks extents are in this coordinate system.  Geographic if not set.
    CoordSystem *coordSys;
    
protected:
    void buildSkirt(BasicDrawable *draw,std::vector<Point3f> &pts,std::vector<TexCoord> &texCoords,const SphericalChunkInfo &chunkInfo);
    // Create one or more drawables to represent the chunk.
    // Only call this if you know what you're doing
    void buildDrawable(BasicDrawable **draw,BasicDrawable **skirtDraw,bool enable,CoordSystemDisplayAdapter *coordAdapter,const SphericalChunkInfo &chunkInfo);
    void calcSampleX(int &thisSampleX,int &thisSampleY,Point3f *dispPts);
};

class ChunkSceneRep;
typedef std::shared_ptr<ChunkSceneRep> ChunkSceneRepRef;
typedef std::set<ChunkSceneRepRef,IdentifiableRefSorter> ChunkRepSet;
 
// Used to track requests that come in so we can queue them
typedef enum {ChunkAdd,ChunkRemove,ChunkEnable,ChunkDisable} ChunkRequestType;
class ChunkRequest
{
public:
    ChunkRequest(ChunkRequestType type,const SphericalChunkInfo &chunkInfo,SphericalChunk *chunk) :
  type(type), chunkId(EmptyIdentity), chunkInfo(chunkInfo), chunk(chunk) { chunkId = chunk->getId(); }
    ChunkRequest(ChunkRequestType type,const SphericalChunkInfo &chunkInfo,SimpleIdentity chunkId) :
    type(type), chunkId(chunkId), chunk(NULL), chunkInfo(chunkInfo), doEdgeMatching(false) { }
    ~ChunkRequest() { if (chunk)  delete chunk; }
    ChunkRequestType type;
    SimpleIdentity chunkId;
    SphericalChunkInfo chunkInfo;
    SphericalChunk *chunk;
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
    SimpleIdentity addChunk(SphericalChunk *chunk,const SphericalChunkInfo &chunkInfo,ChangeSet &changes);
    
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
