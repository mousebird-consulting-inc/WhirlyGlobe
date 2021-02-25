/*
 *  SphericalEarthChunkManager.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/29/12.
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

#import <math.h>
#import <queue>
#import "WhirlyVector.h"
#import "LoadedTileNew.h"
#import "Scene.h"
#import "BaseInfo.h"
#import "ImageTile.h"
#import "BasicDrawableBuilder.h"

namespace WhirlyKit
{
// Used to track scene data associated with a chunk
class ChunkSceneRep : public Identifiable
{
public:
    ChunkSceneRep(SimpleIdentity theId) : Identifiable(theId) { }
    ChunkSceneRep() { }
    SimpleIDSet drawIDs;
    SimpleIDSet texIDs;
    
    // Remove elements from the scene
    void clear(Scene *scene,ChangeSet &changeRequests)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            changeRequests.push_back(new RemDrawableReq(*it));
        for (SimpleIDSet::iterator it = texIDs.begin();
             it != texIDs.end(); ++it)
            changeRequests.push_back(new RemTextureReq(*it));
    }
    
    // Enable drawables
    void enable(ChangeSet &changes)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            changes.push_back(new OnOffChangeRequest(*it, true));
    }
    
    // Disable drawables
    void disable(ChangeSet &changes)
    {
        for (SimpleIDSet::iterator it = drawIDs.begin();
             it != drawIDs.end(); ++it)
            changes.push_back(new OnOffChangeRequest(*it, false));
    }
};
    
// Info object for spherical chunks
class SphericalChunkInfo : public BaseInfo
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SphericalChunkInfo();
    SphericalChunkInfo(const Dictionary &);

    RGBAColor color;
    bool doEdgeMatching;
    std::vector<SimpleIdentity> texIDs;
};
typedef std::shared_ptr<SphericalChunkInfo> SphericalChunkInfoRef;

/** This defines a chunk of the globe to overlay with a single
 image.  In general you should use one of the quad layers
 instead.  This is here if you need to control data loading
 image by image, presumably with an active layer.
 */
class SphericalChunk : public Identifiable
{
    friend class SphericalChunkManager;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    SphericalChunk();

    /// Bounding box for the chunk to display
    Mbr mbr;

    /// Texture we'll wrap over the top
    std::vector<SimpleIdentity> texIDs;

    /// If no texture, we can pass in a UIImage (or NSData that contains common formats).
    /// The implication here is that we're going to stick these in an atlas.
    ImageTileRef loadImage;

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
    CoordSystemRef coordSys;
    
protected:
    void buildSkirt(SceneRenderer *sceneRender,BasicDrawableBuilderRef draw,Point3fVector &pts,int pointOffset,std::vector<TexCoord> &texCoords,const SphericalChunkInfo &chunkInfo);
    // Create one or more drawables to represent the chunk.
    // Only call this if you know what you're doing
    void buildDrawable(SceneRenderer *sceneRender,BasicDrawableBuilderRef draw,bool buildSkirt,BasicDrawableBuilderRef skirtDraw,bool enable,CoordSystemDisplayAdapter *coordAdapter,const SphericalChunkInfo &chunkInfo);
    void calcSampleX(int &thisSampleX,int &thisSampleY,Point3f *dispPts);
};

typedef std::shared_ptr<ChunkSceneRep> ChunkSceneRepRef;
typedef std::set<ChunkSceneRepRef,IdentifiableRefSorter> ChunkRepSet;
 
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
    void setBorderTexel(int inBorderTexel) { borderTexel = inBorderTexel; }
    
    /// Add the given chunk (enabled or disabled)
    SimpleIdentity addChunks(const std::vector<SphericalChunk> &chunks,const SphericalChunkInfo &chunkInfo,ChangeSet &changes);
    
    /// Modify the given chunk (new texture IDs)
    bool modifyChunkTextures(SimpleIdentity chunkID,const std::vector<SimpleIdentity> &texIDs,ChangeSet &changes);
    
    /// Modify the draw priority
    bool modifyDrawPriority(SimpleIdentity chunkID,int drawPriority,ChangeSet &changes);
    
    /// Enable or disable the given chunk
    void enableChunk(SimpleIdentity chunkID,bool enable,ChangeSet &changes);
    
    /// Remove the given chunks
    void removeChunks(SimpleIDSet &chunkIDs,ChangeSet &changes);
    
    /// Number of chunks we're representing
    int getNumChunks();
    
    /// Process outstanding requests
    void processRequests(ChangeSet &changes);
    
protected:
    std::mutex repLock;
    ChunkRepSet chunkReps;
    int borderTexel;
};

}
