/*
 *  LoadedTile.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/13.
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
#import <math.h>
#import "WhirlyVector.h"
#import "TextureGroup.h"
#import "Scene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES.h"
#import "QuadDisplayLayer.h"
#import "TextureAtlas.h"
#import "ElevationChunk.h"
#import "DynamicDrawableAtlas.h"
#import "DynamicTextureAtlas.h"

/** Type of the image being passed to the tile loader.
 UIImage - A UIImage object.
 NSDataAsImage - An NSData object containing PNG or JPEG data.
 WKLoadedImageNSDataRawData - An NSData object containing raw RGBA values.
 PVRTC4 - Compressed PVRTC, 4 bit, no alpha
 PKM - ETC2 and EAC textures
 Placeholder - This is an empty image (so no visual representation)
 that is nonetheless "valid" so its children will be paged.
 */
typedef enum {WKLoadedImageUIImage,WKLoadedImageNSDataAsImage,WKLoadedImageNSDataRawData,WKLoadedImagePVRTC4,WKLoadedImageNSDataPKM,WKLoadedImagePlaceholder,WKLoadedImageMax} WhirlyKitLoadedImageType;

/// Used to specify the image type for the textures we create
typedef enum {WKTileIntRGBA,
    WKTileUShort565,
    WKTileUShort4444,
    WKTileUShort5551,
    WKTileUByteRed,WKTileUByteGreen,WKTileUByteBlue,WKTileUByteAlpha,
    WKTileUByteRGB,
    WKTilePVRTC4,
    WKTileETC2_RGB8,WKTileETC2_RGBA8,WKTileETC2_RGB8_PunchAlpha,
    WKTileEAC_R11,WKTileEAC_R11_Signed,WKTileEAC_RG11,WKTileEAC_RG11_Signed,
} WhirlyKitTileImageType;

/// How we'll scale the tiles up or down to the nearest power of 2 (square) or not at all
typedef enum {WKTileScaleUp,WKTileScaleDown,WKTileScaleFixed,WKTileScaleNone} WhirlyKitTileScaleType;


/** The Loaded Image is handed back to the Tile Loader when an image
 is finished.  It can either be loaded or empty, or something of that sort.
 */
@interface WhirlyKitLoadedImage : NSObject

/// The data we're passing back
@property (nonatomic,assign) WhirlyKitLoadedImageType type;
/// Set if there are any border pixels in the image
@property (nonatomic,assign) int borderSize;
/// The UIImage or NSData object
@property (nonatomic) NSObject *imageData;
/// Some formats contain no size info (e.g. PVRTC).  In which case, this is set
@property (nonatomic,assign) int width,height;

/// Return a loaded image made of a standard UIImage
+ (WhirlyKitLoadedImage *)LoadedImageWithUIImage:(UIImage *)image;

/// Return a loaded image made from an NSData object containing PVRTC
+ (WhirlyKitLoadedImage *)LoadedImageWithPVRTC:(NSData *)imageData size:(int)squareSize;

/// Return a loaded image that's just an empty placeholder.
/// This means there's nothing to display, but the children are valid
+ (WhirlyKitLoadedImage *)PlaceholderImage;

/// Return a loaded image made from an NSData object that contains a PNG or JPG.
/// Basically somethign that UIImage will recognize if you initialize it with one.
+ (WhirlyKitLoadedImage *)LoadedImageWithNSDataAsPNGorJPG:(NSData *)imageData;

/// Generate an appropriate texture.
/// You could overload this, just be sure to respect the border pixels.
- (WhirlyKit::Texture *)buildTexture:(int)borderSize destWidth:(int)width destHeight:(int)height;

/// This will extract the pixels out of an image or NSData and store them for later use
- (bool)convertToRawData:(int)borderTexel;

@end

/** This is a more generic version of the Loaded Image.  It can be a single
 loaded image, a stack of them (for animation) and/or a terrain chunk.
 If you're doing a stack of images, make sure you set up the tile quad loader
 that way.
 */
@interface WhirlyKitLoadedTile : NSObject

@property (nonatomic,readonly) NSMutableArray *images;
@property (nonatomic) NSObject<WhirlyKitElevationChunk> *elevChunk;

@end

/** This protocol is used by the data sources to optionally tack some elevation on to a tile
 fetch.  Elevation often comes from a different source and we want to be able to reuse
 our generic image tile fetchers.
 */
@protocol WhirlyKitElevationHelper
/// Return the elevation data for the given tile or nil if there is none
- (NSObject<WhirlyKitElevationChunk> *)elevForLevel:(int)level col:(int)col row:(int)row;
@end

namespace WhirlyKit
{
    
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
    void initAtlases(WhirlyKitTileImageType imageType,GLenum interpType,int numImages,int textureAtlasSize,int sampleSizeX,int sampleSizeY);
    
    // Build the edge matching skirt
    void buildSkirt(BasicDrawable *draw,std::vector<Point3d> &pts,std::vector<TexCoord> &texCoords,float skirtFactor,bool haveElev,const Point3d &theCenter);
    
    // Generate drawables for a no-elevation tile
    void generateDrawables(WhirlyKit::ElevationDrawInfo *drawInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,BasicDrawable **poleDraw);
    
    // Build a given tile
    bool buildTile(Quadtree::NodeInfo *nodeInfo,BasicDrawable **draw,BasicDrawable **skirtDraw,BasicDrawable **poleDraw,std::vector<Texture *> *texs,
              Point2f texScale,Point2f texOffset,int samplingX,int samplingY,std::vector<WhirlyKitLoadedImage *> *loadImages,NSObject<WhirlyKitElevationChunk> *elevData,const Point3d &theCenter,Quadtree::NodeInfo *parentNodeInfo);
    
    // Build the texture for a tile
    Texture *buildTexture(WhirlyKitLoadedImage *loadImage);
    
    // Flush updates out into the change requests
    bool flushUpdates(ChangeSet &changes);
    
    // Destroy the texture and drawable atlases because they're empty (they'd better be)
    void clearAtlases(ChangeSet &theChangeRequests);
    
    // Update the texture atlas mappings for animations switching
    void updateAtlasMappings();
    
    // See if we're ready to make some changes
    bool isReady();
    
    // Output some debug info
    void log(NSString *name);
    
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
    
    // If set we're using elevation data
    bool includeElev,useElevAsZ;
    
    // If set, no skirts
    bool ignoreEdgeMatching;
    
    // Set if we want pole geometry
    bool coverPoles;
    
    // Color overrides for poles, if present
    bool useNorthPoleColor,useSouthPoleColor;
    RGBAColor northPoleColor,southPoleColor;
    
    // Set if we'll use tile centers when generating drawables
    bool useTileCenters;
    
    // Image format for textures
    GLenum glFormat;
    WKSingleByteSource singleByteSource;
    
    // Whether we start new drawables enabled or disabled
    bool enabled;
    
    // Fade for drawables
    float fade;

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
    DynamicDrawableAtlas *drawAtlas,*poleDrawAtlas;
    
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
    
    // If set non-zero we'll render to another target
    SimpleIdentity renderTargetID;
};
    
/** The Loaded Tile is used to track tiles that have been
    loaded in to memory, but may be in various states.  It's also
    used to fill in child outlines that may be missing.
 */
class LoadedTile
{
public:
    LoadedTile();
    LoadedTile(const WhirlyKit::Quadtree::Identifier &);
    ~LoadedTile() { }
    
    /// Calculate the tile's overall size.  Needed later.
    void calculateSize(Quadtree *quadTree,CoordSystemDisplayAdapter *coordAdapt,CoordSystem *coordSys);
    
    /// Build the data needed for a scene representation
    bool addToScene(TileBuilder *tileBuilder,std::vector<WhirlyKitLoadedImage *>loadImages,int frame,int currentImage0,int currentImage1,NSObject<WhirlyKitElevationChunk> *loadElev,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update the texture in an existing tile.  This is for loading frames of animation
    bool updateTexture(TileBuilder *tileBuilder,WhirlyKitLoadedImage *loadImage,int frame,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);

    /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(TileBuilder *tileBuilder,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(TileBuilder *tileBuilder,LoadedTile *childTiles[],int currentImage0,int currentImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests,std::vector<Quadtree::Identifier> &nodesEnabled,std::vector<Quadtree::Identifier> &nodesDisabled);
    
    /// Switch to the given images
    void setCurrentImages(TileBuilder *tileBuilder,int whichImage0,int whichImage1,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Turn drawables on/off
    void setEnable(TileBuilder *tileBuilder, bool enable, ChangeSet &theChanges);

    /// Change the fade on drawables
    // Note: This does nothing for the the non-bigdrawable case
    void setFade(TileBuilder *tileBuilder, float fade, ChangeSet &theChanges);

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
    // Optional ID for the poles
    WhirlyKit::SimpleIdentity poleDrawId;
    // Texture IDs for the parent tile
    std::vector<WhirlyKit::SimpleIdentity> texIds;
    /// If set, these are subsets of a larger dynamic texture
    std::vector<WhirlyKit::SubTexture> subTexs;
    /// If here, the elevation data needed to build geometry
    NSObject<WhirlyKitElevationChunk> *elevData;
    /// Center of the tile in display coordinates
    Point3d dispCenter;
    /// Size in display coordinates
    double tileSize;
    /// Where the textures live in the dynamic texture(s)
    DynamicTextureAtlas::TextureRegion texRegion;
    /// Sampling for surface in X,Y if we're not doing elevation tiles
    int samplingX,samplingY;
    
    // IDs for the various fake child geometry
    WhirlyKit::SimpleIdentity childDrawIds[4];
    WhirlyKit::SimpleIdentity childSkirtDrawIds[4];
    WhirlyKit::SimpleIdentity childPoleDrawIds[4];
};

/// This is a comparison operator for sorting loaded tile pointers by
/// Quadtree node identifier.
typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const LoadedTile *a,const LoadedTile *b)
    {
        return a->nodeInfo.ident < b->nodeInfo.ident;
    }
} LoadedTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<LoadedTile *,LoadedTileSorter> LoadedTileSet;
    
}
