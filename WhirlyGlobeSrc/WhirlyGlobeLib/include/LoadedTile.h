/*
 *  LoadedTile.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/13.
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
#import "Scene.h"
#import "GlobeMath.h"
#import "Quadtree.h"
#import "SceneRendererES.h"
#import "QuadDisplayController.h"
#import "TextureAtlas.h"
// Note: Porting
//#import "ElevationChunk.h"
#import "DynamicDrawableAtlas.h"
#import "DynamicTextureAtlas.h"

namespace WhirlyKit
{

/// Used to specify the image type for the textures we create
typedef enum {WKTileIntRGBA,WKTileUShort565,WKTileUShort4444,WKTileUShort5551,WKTileUByteRed,WKTileUByteGreen,WKTileUByteBlue,WKTileUByteAlpha,WKTileUByteRGB,WKTilePVRTC4,WKTileETC2_RGB8,
    WKTileETC2_RGBA8,WKTileETC2_RGB8_PunchAlpha,WKTileEAC_R11,WKTileEAC_R11_Signed,WKTileEAC_RG11,WKTileEAC_RG11_Signed} WhirlyKitTileImageType;

/// How we'll scale the tiles up or down to the nearest power of 2 (square) or not at all
typedef enum {WKTileScaleUp,WKTileScaleDown,WKTileScaleFixed,WKTileScaleNone} WhirlyKitTileScaleType;


/** The Loaded Image is handed back to the Tile Loader when an image
 is finished.  It can either be loaded or empty, or something of that sort.
 */
class LoadedImage
{
public:
    LoadedImage();
    virtual ~LoadedImage();
    
    /// Generate an appropriate texture.
    /// You could overload this, just be sure to respect the border pixels.
    virtual Texture *buildTexture(int borderSize,int width,int height) = 0;
    
    /// This means there's nothing to display, but the children are valid
    virtual bool isPlaceholder() = 0;
    
    /// Return image width
    virtual int getWidth() = 0;
    
    /// Return image height
    virtual int getHeight() = 0;
protected:
};

/** This is a more generic version of the Loaded Image.  It can be a single
 loaded image, a stack of them (for animation) and/or a terrain chunk.
 If you're doing a stack of images, make sure you set up the tile quad loader
 that way.
 */
// Note: Porting
//class LoadedTile
//{
//public:
//    std::vector<LoadedImage *> images;
//    // Note: Porting
//    //@property (nonatomic) WhirlyKitElevationChunk *elevChunk;
//};

// Note: Porting
///** This protocol is used by the data sources to optionally tack some elevation on to a tile
// fetch.  Elevation often comes from a different source and we want to be able to reuse
// our generic image tile fetchers.
// */
//@protocol WhirlyKitElevationHelper
///// Return the elevation data for the given tile or nil if there is none
//- (WhirlyKitElevationChunk *)elevForLevel:(int)level col:(int)col row:(int)row;
//@end

/** The Tile Builder stores data needed to build individual tiles.
    This includes the texture and drawable atlases.
  */
class TileBuilder
{
public:
    TileBuilder(CoordSystem *coordSys,Mbr mbr,WhirlyKit::Quadtree *quadTree);
    ~TileBuilder();
    
    // Check if the given tile overlaps the area we're representing
    bool isValidTile(const Mbr &childMbr);
    
    // Calculate the size of a tile
    void textureSize(int width, int height,int *destWidth,int *destHeight);
    
    // Initialize texture and drawable atlases (only once)
    void initAtlases(WhirlyKitTileImageType imageType,int numImages,int textureAtlasSize,int sampleSizeX,int sampleSizeY);
    
    // Build the edge matching skirt
    void buildSkirt(BasicDrawable *draw,Point3fVector &pts,std::vector<TexCoord> &texCoords,float skirtFactor,bool haveElev);
    
    // Build a given tile
    // Note: Porting
//    bool buildTile(Quadtree::NodeInfo *nodeInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,std::vector<Texture *> *texs,
//                   Point2f texScale,Point2f texOffset,std::vector<WhirlyKitLoadedImage *> *loadImages,WhirlyKitElevationChunk *elevData);
    bool buildTile(Quadtree::NodeInfo *nodeInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,std::vector<Texture *> *texs,
                   Point2f texScale,Point2f texOffset,std::vector<LoadedImage *> *loadImages);
    
    // Build the texture for a tile
    Texture *buildTexture(LoadedImage *loadImage);

    // Flush updates out into the change requests
    bool flushUpdates(ChangeSet &changes);
    
    // Destroy the texture and drawable atlases because they're empty (they'd better be)
    void clearAtlases(ChangeSet &theChangeRequests);
    
    // Update the texture atlas mappings for animations switching
    void updateAtlasMappings();
    
    // See if we're ready to make some changes
    bool isReady();
    
    // Output some debug info
//    void log(NSString *name);
    
    // Bounding box of area we're representing
    Mbr mbr;
    
    // Coordinate system we're working in
    CoordSystem *coordSys;
    
    // How we scale tile images (if we do)
    WhirlyKitTileScaleType tileScale;
    int fixedTileSize;
    
    // The drawables are created with these values
    int drawOffset,drawPriority;
    float minVis,maxVis;
    bool hasAlpha;
    RGBAColor color;
    SimpleIdentity programId;
    
    // Fudge factor for texture atlas
    float texAtlasPixelFudge;
    
    // If set we're using elevation data
    bool includeElev,useElevAsZ;
    
    // If set, no skirts
    bool ignoreEdgeMatching;
    
    // Set if we want pole geometry
    bool coverPoles;
    
    // Image format for textures
    GLenum glFormat;
    WKSingleByteSource singleByteSource;
    
    // Whether we start new drawables enabled or disabled
    bool enabled;

    // Number of samples to use for tiles
    int defaultSphereTessX,defaultSphereTessY;
    int imageDepth;
    DynamicTextureAtlas *texAtlas;
    
    // The texture atlas mappings keep track of textures we've created
    //  in each of the atlases as well as how the drawable atlas is using
    //  them.  We put them here so we can switch them in another thread
    pthread_mutex_t texAtlasMappingLock;
    std::vector<std::vector<SimpleIdentity> > texAtlasMappings;
    std::vector<DynamicDrawableAtlas::DrawTexInfo> drawTexInfo;
    int texelBinSize;
        
    // Drawable atlas to match the texture atlas
    DynamicDrawableAtlas *drawAtlas;
    
    // Number of border texels we need in an image
    int borderTexel;
    
    // Quad tree we're using for loading
    WhirlyKit::Quadtree *tree;
    
    // Scene we're writing to
    Scene *scene;
    
    // Set if we're just drawing lines (doesn't work well anymore)
    bool lineMode;
    
    // Number of textures we're feeding drawables at once
    int activeTextures;
    
    // Set when we create new drawables
    bool newDrawables;
    
    // Set if we're in single level mode.  That is, we're only trying to display a single level.
    bool singleLevel;
};
    
/** The Loaded Tile is used to track tiles that have been
    loaded in to memory, but may be in various states.  It's also
    used to fill in child outlines that may be missing.
 */
class InternalLoadedTile
{
public:
    InternalLoadedTile();
    InternalLoadedTile(const WhirlyKit::Quadtree::Identifier &);
    ~InternalLoadedTile() { }

    /// Calculate the tile's overall size.  Needed later.
    void calculateSize(Quadtree *quadTree,CoordSystemDisplayAdapter *coordAdapt,CoordSystem *coordSys);
 
    // Note: Porting
    /// Build the data needed for a scene representation
//    bool addToScene(TileBuilder *tileBuilder,std::vector<LoadedImage *>loadImages,int currentImage0,int currentImage1,WhirlyKitElevationChunk *loadElev,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    bool addToScene(TileBuilder *tileBuilder,std::vector<LoadedImage *>loadImages,int frame,int currentImage0,int currentImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update the texture in an existing tile.  This is for loading frames of animation
    bool updateTexture(TileBuilder *tileBuilder,LoadedImage *loadImage,int frame,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);

        /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(TileBuilder *tileBuilder,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(TileBuilder *tileBuilder,InternalLoadedTile *childTiles[],int currentImage0,int currentImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Switch to the given images
    void setCurrentImages(TileBuilder *tileBuilder,int whichImage0,int whichImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Turn drawables on/off
    void setEnable(TileBuilder *tileBuilder, bool enable, ChangeSet &theChanges);
    
    /// Dump out to the log
    void Print(TileBuilder *tileBuilder);
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::NodeInfo nodeInfo;
    
    /// Set if this has been initialized (e.g. geometry was built at one point)
    bool isInitialized;
    /// Set if this is just a placeholder (no geometry)
    bool placeholder;
    /// Set if this tile is in the process of loading
    bool isLoading;
    /// Set if we skipped this tile (in flat mode)
    bool isUnknown;
    // DrawID for this parent tile
    WhirlyKit::SimpleIdentity drawId;
    // Optional ID for the skirts
    WhirlyKit::SimpleIdentity skirtDrawId;
    // Texture IDs for the parent tile
    std::vector<WhirlyKit::SimpleIdentity> texIds;
    /// If set, these are subsets of a larger dynamic texture
    std::vector<WhirlyKit::SubTexture> subTexs;
    // Note: Porting
    /// If here, the elevation data needed to build geometry
//    WhirlyKitElevationChunk *elevData;
    /// Center of the tile in display coordinates
    Point3d dispCenter;
    /// Size in display coordinates
    double tileSize;
    /// Where the textures live in the dynamic texture(s)
    DynamicTextureAtlas::TextureRegion texRegion;
    
    // IDs for the various fake child geometry
    WhirlyKit::SimpleIdentity childDrawIds[4];
    WhirlyKit::SimpleIdentity childSkirtDrawIds[4];
};

/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const InternalLoadedTile *a,const InternalLoadedTile *b)
    {
        return a->nodeInfo.ident < b->nodeInfo.ident;
    }
} LoadedTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<InternalLoadedTile *,LoadedTileSorter> LoadedTileSet;
    
}
