/*
 *  TileQuadLoader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/27/12.
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

/// @cond
@class WhirlyKitQuadTileLoader;
/// @endcond

/** Type of the image being passed to the tile loader.
    UIImage - A UIImage object.
    NSDataAsImage - An NSData object containing PNG or JPEG data.    
    WKLoadedImageNSDataRawData - An NSData object containing raw RGBA values.
    PVRTC4 - Compressed PVRTC, 4 bit, no alpha
    Placeholder - This is an empty image (so no visual representation)
                that is nonetheless "valid" so its children will be paged.
  */
typedef enum {WKLoadedImageUIImage,WKLoadedImageNSDataAsImage,WKLoadedImageNSDataRawData,WKLoadedImagePVRTC4,WKLoadedImagePlaceholder,WKLoadedImageMax} WhirlyKitLoadedImageType;

/** The Loaded Image is handed back to the Tile Loader when an image
 is finished.  It can either be loaded or empty, or something of that sort.
 */
@interface WhirlyKitLoadedImage : NSObject
{
@public
    /// The data we're passing back
    WhirlyKitLoadedImageType type;
    /// Set if there are any border pixels in the image
    int borderSize;
    /// The UIImage or NSData object
    NSObject *imageData;
    /// Some formats contain no size info (e.g. PVRTC).  In which case, this is set
    int width,height;
}

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

@end

namespace WhirlyKit
{
    
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
    
    /// Build the data needed for a scene representation
    void addToScene(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Scene *scene,WhirlyKitLoadedImage *loadImage,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Scene *scene,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Quadtree *tree,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Dump out to the log
    void Print(WhirlyKit::Quadtree *tree);
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::NodeInfo nodeInfo;
    
    /// Set if this is just a placeholder (no geometry)
    bool placeholder;    
    /// Set if this tile is in the process of loading
    bool isLoading;
    // DrawID for this parent tile
    WhirlyKit::SimpleIdentity drawId;
    // Optional ID for the skirts
    WhirlyKit::SimpleIdentity skirtDrawId;
    // Texture ID for the parent tile
    WhirlyKit::SimpleIdentity texId;
    /// If set, this is a subset of a larger dynamic texture
    WhirlyKit::SubTexture subTex;
    
    // IDs for the various fake child geometry
    WhirlyKit::SimpleIdentity childDrawIds[4];
    WhirlyKit::SimpleIdentity childSkirtDrawIds[4];
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

/** Quad Tile Image Data Source is used to load individual images
    to put on top of the simple geometry created by the quad tile loader.
 */
@protocol WhirlyKitQuadTileImageDataSource<NSObject>
/// Number of simultaneous fetches this data source can support.
/// You can change this on the fly, but it won't cancel outstanding fetches.
- (int)maxSimultaneousFetches;

@optional
/// The quad loader is letting us know to start loading the image.
/// We'll call the loader back with the image when it's ready.
/// This is now deprecated.  Used the other version.
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row __deprecated;

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs;

@end

/// Used to specify the image type for the textures we create
typedef enum {WKTileIntRGBA,WKTileUShort565,WKTileUShort4444,WKTileUShort5551,WKTileUByte,WKTilePVRTC4} WhirlyKitTileImageType;

/// How we'll scale the tiles up or down to the nearest power of 2 (square) or not at all
typedef enum {WKTileScaleUp,WKTileScaleDown,WKTileScaleNone} WhirlyKitTileScaleType;

/** The Globe Quad Tile Loader responds to the Quad Loader protocol and
    creates simple terrain (chunks of the sphere) and asks for images
    to put on top.
 */
@interface WhirlyKitQuadTileLoader : NSObject<WhirlyKitQuadLoader>
{    
    /// Data layer we're attached to
    WhirlyKitQuadDisplayLayer * __weak quadLayer;
    
    /// Tiles we currently have loaded in the scene
    WhirlyKit::LoadedTileSet tileSet;
    
    /// Delegate used to provide images
    NSObject<WhirlyKitQuadTileImageDataSource> * __weak dataSource;
    
    // Parents to update after changes
    std::set<WhirlyKit::Quadtree::Identifier> parents;
    
    /// Change requests queued up between a begin and end
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;
    
    /// Offset for the data being generated
    int drawOffset;
    
    /// Priority order to use in the renderer
    int drawPriority;
    
    /// If set, the point at which tile geometry will disappear when zoomed out
    float maxVis;

    /// If set, the point at which tile geometry will appear when zoomed in
    float minVis;
    
    /// If set, the point at which we'll stop doing updates (separate from minVis)
    float minPageVis;
    
    /// If set, the point at which we'll stop doing updates (separate from maxVis)
    float maxPageVis;
    
    /// Base color for the drawables created by the layer
    WhirlyKit::RGBAColor color;
    
    /// Set this if the tile images are partially transparent
    bool hasAlpha;
    
    /// How many fetches we have going at the moment
    int numFetches;
    
    /// If set, we'll ignore edge matching.
    /// This can work if you're zoomed in close
    bool ignoreEdgeMatching;
    
    /// If set, we'll fill in the poles for a projection that doesn't go all the way up or down
    bool coverPoles;
    
    /// The data type of GL textures we'll be creating.  RGBA by default.
    WhirlyKitTileImageType imageType;
    
    /// If set (before we start) we'll use dynamic texture and drawable atlases
    bool useDynamicAtlas;
    
    /// If set we'll scale the input images to the nearest square power of two
    WhirlyKitTileScaleType tileScale;
}

@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) float minPageVis,maxPageVis;
@property (nonatomic,assign) WhirlyKit::RGBAColor color;
@property (nonatomic,assign) bool hasAlpha;
@property (nonatomic,weak) WhirlyKitQuadDisplayLayer *quadLayer;
@property (nonatomic,assign) bool ignoreEdgeMatching;
@property (nonatomic,assign) bool coverPoles;
@property (nonatomic,assign) WhirlyKitTileImageType imageType;
@property (nonatomic,assign) bool useDynamicAtlas;
@property (nonatomic,assign) WhirlyKitTileScaleType tileScale;

/// Set this up with an object that'll return an image per tile
- (id)initWithDataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

/// When a data source has finished its fetch for a given image, it calls
///  this method to hand that back to the quad tile loader
/// If this isn't called in the layer thread, it will switch over to that thread first.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(NSData *)image pvrtcSize:(int)pvrtcSize forLevel:(int)level col:(int)col row:(int)row __deprecated;

/// When a data source has finished its fetch for a given image, it
///  calls this method to hand the image (along with key info) back to the
///  quad tile loader.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(WhirlyKitLoadedImage *)loadImage forLevel:(int)level col:(int)col row:(int)row;

@end
