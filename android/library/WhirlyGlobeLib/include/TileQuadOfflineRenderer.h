/*
 *  TileQuadOfflineRenderer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/7/13.
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

#import <mutex>
#import <math.h>
#import "WhirlyVector.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "Quadtree.h"
#import "QuadDisplayController.h"
#import "TileQuadLoader.h"

namespace WhirlyKit
{

/// The offline renderer passes over images like so
class QuadTileOfflineImage
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Size of each of the corner pixels in meters
    Point2d cornerSizes[4];

    /// Bounding box for the rendered area
    Mbr mbr;
    
    /// Which frame this is
    int frame;
    
    /// Textures produced by the offline renderer.  Delegate is responsible for cleanup
    WhirlyKit::SimpleIdentity texture;
    
    /// Size of the center pixel in meters
    Point2d centerSize;
    
    /// Textures size of the images being produced
    Point2d texSize;
};
    
class QuadTileOfflineLoader;

// Internal storage for offline tile
class OfflineTile
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    OfflineTile();
    OfflineTile(const WhirlyKit::Quadtree::Identifier &ident);
    OfflineTile(const WhirlyKit::Quadtree::Identifier &ident,int numImages);
    ~OfflineTile();
    
    // Return the size of this tile
    void GetTileSize(int &numX,int &numY);

    // Return the number of loaded frames
    int getNumLoaded();
    
    // Clear out the OpenGL textures
    void clearTextures(Scene *scene);

    // Details of which node we're representing
    WhirlyKit::Quadtree::Identifier ident;
    
    /// Set if this is just a placeholder (no geometry)
    bool placeholder;
    /// Set if this tile is in the process of loading
    int numLoading;
    
    std::vector<Texture *> textures;
};

typedef struct
{
    /// Comparison operator based on node identifier
    bool operator() (const OfflineTile *a,const OfflineTile *b)
    {
        return a->ident < b->ident;
    }
} OfflineTileSorter;

/// A set that sorts loaded MB Tiles by Quad tree identifier
typedef std::set<OfflineTile *,OfflineTileSorter> OfflineTileSet;

/** Fill in this delegate to receive the UIImage this layer
    generates every period seconds.
  */
class QuadTileOfflineDelegate
{
public:
    virtual ~QuadTileOfflineDelegate() { }
    /// Here's the generated image.  Query the loader for extents.
    virtual void offlineRender(QuadTileOfflineLoader *loader,QuadTileOfflineImage *image) = 0;
};
    
/** This version of the quad tile loader requests and tracks images the same
    as the normal one.  Then it assembles them into a single large image
    covering a given set of extents.  This lets us do 'postage stamp' style
    display of a big complex, areal.
  */
class QuadTileOfflineLoader : public QuadLoader, QuadTileLoaderSupport
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Set this up with an object that'll return an image per tile and a name (for debugging)
    QuadTileOfflineLoader(const std::string &name,QuadTileImageDataSource *imageSource);
    ~QuadTileOfflineLoader();
    void clear(bool clearTextures);
    
    /// Set if we're doing any rendering.  On by default.
    void setOn(bool newOn) { on = newOn; }
    bool getOn() { return on; }

    /// Depth of the image stack per tile
    void setNumImages(int newNumImages) { numImages = newNumImages; }
    
    /// Size (in pixels) of the output image we're building
    void setImageSize(int newSizeX,int newSizeY) { sizeX = newSizeX;  sizeY = newSizeY; }
    
    /// If set, the output size is a maximum.  We'll try to track input resolution
    bool setAutoRes(bool newAutoRes) { autoRes = newAutoRes; return true; }
    
    /// The bounding box for the image we're trying to build
    void setMbr(Mbr newMbr);
    
    /// We want the image generate no more often than this
    void setPeriod(TimeInterval newPeriod);
    
    /// When the MBR changes we get a preview render down to this many levels
    void setPreviewLevels(int newPreviewLevels) { previewLevels = newPreviewLevels; }
    
    /// If set, the delegate that receives the image we're generating every period seconds
    void setOutputDelegate(QuadTileOfflineDelegate *newOutputDelegate) { outputDelegate = newOutputDelegate; }

    /// Called when the layer first starts up.  Keep this around if you need it.
    virtual void init(QuadDisplayController *inControl,Scene *inScene) { control = inControl; scene = inScene;}
    
    /// The quad layer uses this to see if a loader is capable of loading
    ///  another tile.  Use this to track simultaneous loads
    virtual bool isReady();
    
    /// Called right before we start a series of updates
    virtual void startUpdates(ChangeSet &changes) { }
    
    /// Called right after we finish a series of updates
    virtual void endUpdates(ChangeSet &changes) { }
    
    /// The quad tree wants to load the given tile.
    /// Call the layer back when the tile is loaded.
    /// This is in the layer thread.
    virtual void loadTile(const Quadtree::NodeInfo &tileInfo,int frame);
    
    /// Quad tree wants to unload the given tile immediately.
    /// This is in the layer thread.
    virtual void unloadTile(const Quadtree::NodeInfo &tileInfo);
    
    /// The layer is checking to see if it's allowed to traverse below the given tile.
    /// If the loader is still trying to load that given tile (or has some other information about it),
    ///  then return false.  If the tile is loaded and the children may be valid, return true.
    virtual bool canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo);
    
    /// Called when the layer is about to shut down.  Clear out any drawables and caches.
    virtual void shutdownLayer(ChangeSet &changes);
    
    /// Number of frames of animation per tile (if we're doing animation)
    virtual int numFrames();
    
    /// If we're doing animation, currently active frame.
    /// If numFrames = 1, this should be -1.  If numFrames > 1, -1 means load all frames at once
    virtual int currentFrame() { return 0; }
    
    /// Set if the loader can handle individual frame loading
    virtual bool canLoadFrames() { return true; }
    
    /// Called right before the view update to determine if we should even be paging
    /// You can use this to temporarily suspend paging.
    /// isInitial is set if this is the first time through
    virtual bool shouldUpdate(ViewState *viewState,bool isInitial) { return on; }
    
    /// If this is filled in, we can do a hard reset while the layer is running.
    /// This is pretty much identical to shutdownLayer:scene: but we expect to run again afterwards.
    virtual void reset(ChangeSet &changes);
    
    void loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes);
    
    void loadedImages(QuadTileImageDataSource *dataSource,const std::vector<LoadedImage *> &loadImages,int level,int col,int row,int frame,ChangeSet &changes);
    
    // Render any changed images to the given depth and flush out new textures
    void imageRenderToLevel(int deep,ChangeSet &changes);
    
    // Return the value of the somethingChanged flag
    bool getSomethingChanged() { return somethingChanged; }

protected:
    std::mutex mut;
    
    Point2d calculateSize();
    Point2d pixelSizeForMbr(const Mbr &theMbr,const Point2d &texSize,const Point2d &texel);
    OfflineTile *getTile(const WhirlyKit::Quadtree::Identifier &ident);
    
    std::string name;
    QuadTileImageDataSource *imageSource;
    
    bool on;
    int numImages;
    int sizeX,sizeY;
    bool autoRes;
    Mbr theMbr;
    TimeInterval period;
    int previewLevels;
    QuadTileOfflineDelegate *outputDelegate;
    
    OfflineTileSet tiles;
    int numFetches;
    TimeInterval lastRender;
    bool somethingChanged;
    int currentMbr;
    // Frames that have gotten new tiles
    std::set<int> updatedFrames;
    
    OpenGLES2Program *prog;
};
    
}
