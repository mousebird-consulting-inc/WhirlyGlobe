/*
 *  DynamicTextureAtlas.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/28/13.
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

#import <vector>
#import <set>

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "Texture.h"
#import "TextureAtlas.h"

namespace WhirlyKit
{

/** The dynamic texture can have pieces of itself replaced in the layer thread while
    being used in the renderer.  It's used to implement dynamic texture atlases.
  */
class DynamicTexture : virtual public TextureBase
{
public:
    /// Constructor for sorting
    DynamicTexture(const std::string &name);
    DynamicTexture(SimpleIdentity myId) : TextureBase(myId), layoutGrid(NULL) { }
    virtual void setup(int texSize,int cellSize,TextureType type,bool clearTextures);
    virtual ~DynamicTexture();
    
    /// Represents a region in the texture
    class Region
    {
    public:
        Region();
        int sx,sy,ex,ey;
    };
    
    /// Create an appropriately empty texture in OpenGL ES
    virtual bool createInRenderer(const RenderSetupInfo *setupInfo) = 0;

    /// Render side only.  Don't call this.  Destroy the OpenGL ES version
    virtual void destroyInRenderer(const RenderSetupInfo *setupInfo,Scene *scene) = 0;
    
    /// Set the interpolation type used for min and mag
    void setInterpType(TextureInterpType inType) { interpType = inType; }
    TextureInterpType getInterpType() { return interpType; }
    
    /// Add the given texture at the given location.
    /// This is probably called on the layer thread
    void addTexture(Texture *tex,const Region &region);
    
    /// Add the data at a given location in the texture
    virtual void addTextureData(int startX,int startY,int width,int height,RawDataRef data) = 0;
    
    /// Clear out the area given
    void clearRegion(const Region &region,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData);
    virtual void clearTextureData(int startX,int startY,int width,int height,ChangeSet &changes,bool mainThreadMerge,unsigned char *emptyData) = 0;
    
    /// Set or clear a given region
    void setRegion(const Region &region,bool enable);
    
    /// Look for an open region of the given cell extents
    bool findRegion(int cellsX,int cellsY,Region &region);
    
    /// Return a list of released regions
    void getReleasedRegions(std::vector<DynamicTexture::Region> &toClear);
    
    /// Add a region to the list of ones to be cleared.
    /// This is called by the renderer
    void addRegionToClear(const Region &region);
    
    /// Return true if this isn't representing any regions
    bool empty();
    
    /// Number of sub textures we're currently representing
    int &getNumRegions() { return numRegions; }
    
    /// Return texture cell utilization
    void getUtilization(int &numCell,int &usedCell);
    
protected:
    /// Used for debugging
    std::string name;
    
    /// Number of texels on a side
    int texSize;
    /// Number of texels in a cell
    int cellSize;
    /// Number of cells on a side
    int numCell;
    /// Interpolation type
    TextureInterpType interpType;
    /// Texture memory format
    TextureType type;

    // Use to track where sub textures are
    bool *layoutGrid;
    
    std::mutex regionLock;
    /// These regions have been released by the renderer
    std::vector<Region> releasedRegions;
    
    /// Number of active regions (as far as the texture is concerned)
    int numRegions;

    /// If set, overwrite texture data with empty pixels
    bool clearTextures;
};
    
typedef std::shared_ptr<DynamicTexture> DynamicTextureRef;

typedef std::vector<DynamicTextureRef> DynamicTextureVec;

// Used to sort dynamic texture vectors
typedef struct
{
    bool operator () (const DynamicTextureVec *a,const DynamicTextureVec *b) const { return a->at(0)->getId() < b->at(0)->getId(); }
} DynamicTextureVecSorter;

/// Copy data into a dynamic texture (on the main thread)
class DynamicTextureAddRegion : public ChangeRequest
{
public:
    DynamicTextureAddRegion(SimpleIdentity texId,int startX,int startY,int width,int height,RawDataRef data)
    : texId(texId), startX(startX), startY(startY), width(width), height(height), data(data) { }

    /// Add the region.  Never call this.
    void execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity texId;
    int startX,startY,width,height;
    RawDataRef data;
};
    
/// Tell a dynamic texture that a region has been released for use
class DynamicTextureClearRegion : public ChangeRequest
{
public:
    /// Construct with the dynamic texture ID and the region to clear
    DynamicTextureClearRegion(SimpleIdentity texId,const DynamicTexture::Region &region) : texId(texId), region(region) { }
    /// This version takes a time
    DynamicTextureClearRegion(SimpleIdentity texId,const DynamicTexture::Region &region,TimeInterval inWhen) : texId(texId), region(region) { when = inWhen; }

    /// Clear the region from the given dynamic texture.  Never call this.
    void execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view);

protected:
    SimpleIdentity texId;
    DynamicTexture::Region region;
};

/** The dynamic texture atlas manages a variable number of dynamic textures into which it will stuff
    individual textures.  You use it by adding your individual Textures and passing the
    change requests on to the layer thread (or Scene).  You can also clear your Textures later
    by region.
  */
class DynamicTextureAtlas
{
public:
    /// This maps a given texture to its location in a dynamic texture
    class TextureRegion
    {
    public:
        TextureRegion();
        bool operator < (const TextureRegion &that) const { return subTex.getId() < that.subTex.getId(); }
        
        SubTexture subTex;
        SimpleIdentity dynTexId;
        DynamicTexture::Region region;
    };

    /// Construct with the square size of the textures, the cell size (in pixels) and the pixel format
    DynamicTextureAtlas(const std::string &name,int texSize,int cellSize,TextureType format,int imageDepth=1,bool mainThreadMerge=false);
    virtual ~DynamicTextureAtlas();
    
    /// Set the interpolation type used for min and mag
    void setInterpType(TextureInterpType inType);
    TextureInterpType getInterpType();
    
    /// Return the dynamic texture's format
    TextureType getFormat();
    
    /// Fudge factor for border pixels.  We'll add this/pixelSize to the lower left
    ///  and subtract this/pixelSize from the upper right for each texture application.
    void setPixelFudgeFactor(float pixFudge);
    
    /// Try to add the texture to one of our dynamic textures, or create one.
    bool addTexture(SceneRenderer *sceneRender,const std::vector<Texture *> &textures,int frame,Point2f *realSize,Point2f *realOffset,SubTexture &subTex,ChangeSet &changes,int borderPixels,int bufferPixels=0,TextureRegion *outTexRegion=NULL);
    
    /// Update one of the frames of a multi-frame texture atlas
    bool updateTexture(Texture *,int frame,const TextureRegion &texRegion,ChangeSet &changes);
    
    /// Free up the space for a texture from one of the dynamic textures
    void removeTexture(const SubTexture &subTex,ChangeSet &changes,TimeInterval when);
    
    /// Return the IDs for the dynamic textures we're using
    void getTextureIDs(std::vector<SimpleIdentity> &texIDs,int which);
    
    /// Return the texture ID for a given frame, corresponding to the base Tex ID
    SimpleIdentity getTextureIDForFrame(SimpleIdentity baseTexID,int which);
    
    /// Check if the dynamic texture atlas is empty.
    /// Call cleanup() first
    bool empty();
    
    /// Look for any textures that should be cleaned up
    void cleanup(ChangeSet &changes,TimeInterval when);

    /// Clear out the active dynamic textures.  Caller deals with the
    ///  change requests.
    void teardown(ChangeSet &changes);
    
    /// Get some basic info out
    void getUsage(int &numRegions,int &dynamicTextures);
    
    /// Print out some utilization info
    void log();

protected:
    std::string name;
    
    /// Texture memory format
    TextureType format;
    /// Interpolation type
    TextureInterpType interpType;

    int imageDepth;
    int texSize;
    int cellSize;
    /// Interpolation type
    float pixelFudge;
    bool mainThreadMerge;

    /// If set, overwrite texture data with empty pixels
    bool clearTextures;
    
    // On some devices we can't clear with a NULL, we have to use an actual buffer
    std::vector<unsigned char> emptyPixelBuffer;

    typedef std::set<TextureRegion> TextureRegionSet;
    TextureRegionSet regions;
    typedef std::set<DynamicTextureVec *,DynamicTextureVecSorter> DynamicTextureSet;
    DynamicTextureSet textures;
};

}
