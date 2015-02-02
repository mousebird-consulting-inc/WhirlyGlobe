/*
 *  TileQuadLoader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/27/12.
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
#import "QuadDisplayController.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "Quadtree.h"
#import "SceneRendererES.h"
#import "QuadDisplayController.h"
#import "TextureAtlas.h"
//#import "ElevationChunk.h"
#import "LoadedTile.h"
#import "Dictionary.h"

namespace WhirlyKit
{

class QuadTileImageDataSource;

/** This protocol outlines the method that a WhirlyKitQuadTileImageDataSource
 compliant object uses to tell the tile loader that a tile has loaded or
 failed to load.  We break it out so that other objects can talk to
 tile loading objects without subclassing WhirlyKitQuadTileLoader.
 */
class QuadTileLoaderSupport
{
public:
    virtual ~QuadTileLoaderSupport() { }
    /// When a data source has finished its fetch for a given tile, it
    ///  calls this method to hand the data (along with key info) back to the
    ///  quad tile loader.
    /// You can pass back a WhirlyKitLoadedTile or a WhirlyKitLoadedImage or
    ///  just a WhirlyKitElevationChunk.

    virtual void loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes) = 0;
};
    
/** Quad Tile Image Data Source is used to load individual images
    to put on top of the simple geometry created by the quad tile loader.
 */
class QuadTileImageDataSource
{
public:
    QuadTileImageDataSource();
    virtual ~QuadTileImageDataSource();

    /// Number of simultaneous fetches this data source can support.
    /// You can change this on the fly, but it won't cancel outstanding fetches.
    virtual int maxSimultaneousFetches() = 0;

    /// The quad loader is letting us know to start loading the image.
    /// We'll call the loader back with the image when it's ready.
    virtual void startFetch(QuadTileLoaderSupport *quadLoader,int level,int col,int row,int frame,Dictionary *attrs) = 0;

    /// Check if the given tile is a local or remote fetch.  This is a hint
    ///  to the pager.  It can display local tiles as a group faster.
    virtual bool tileIsLocal(int level,int col,int row,int frame) = 0;
    
    /// An optional callback provided when a tile is unloaded.
    /// You don't have to do anything
    virtual void tileWasUnloaded(int level,int col,int row) = 0;
};

/** The Globe Quad Tile Loader responds to the Quad Loader protocol and
    creates simple terrain (chunks of the sphere) and asks for images
    to put on top.
 */
class QuadTileLoader : public QuadLoader, public QuadTileLoaderSupport
{
public:
    /// Set this up with an object that'll return an image per tile and a name (for debugging)
    QuadTileLoader(const std::string &name,QuadTileImageDataSource *imageSource,int numFrames);
    virtual ~QuadTileLoader();
    
    /// QuadLoader methods
    virtual void init(QuadDisplayController *inControl,Scene *inScene);
    virtual void reset(ChangeSet &changes);
    virtual void shutdownLayer(ChangeSet &changes);
    virtual bool isReady();
    virtual void loadTile(const Quadtree::NodeInfo &tileInfo,int frame);
    virtual bool canLoadChildrenOfTile(const Quadtree::NodeInfo &tileInfo);
    virtual void unloadTile(const Quadtree::NodeInfo &tileInfo);
    virtual void startUpdates(ChangeSet &changes);
    virtual void updateWithoutFlush();
    virtual void endUpdates(ChangeSet &changes);
    virtual bool shouldUpdate(ViewState *viewState,bool isInitial);
    virtual int numNetworkFetches();
    virtual int numLocalFetches();
    virtual int numFrames();
    virtual int currentFrame();
    virtual bool canLoadFrames();
    
    /// QuadTileLoaderSupport methods
    virtual void loadedImage(QuadTileImageDataSource *dataSource,LoadedImage *loadImage,int level,int col,int row,int frame,ChangeSet &changes);
    
    /// Set up the change requests to make the given image layer the active one
    /// The call is thread safe
    void setCurrentImage(int newImage,ChangeSet &changeRequests);
    
    /// Set up the change requests to make the given images current.
    /// This will also interpolate between the two
    void setCurrentImageStart(int startImage,int endImage,ChangeSet &changeRequests);
    
    /// If you're passing in elevation (even some of the time), set this to the maximum
    ///  sampling you're going to pass in.  If you don't set ths, you may lose tiles.
    void setTesselationSize(int x,int y);

    /// By default we're on, but we can be turned off
    /// This results in changes to the scene
    void setEnable(bool enable,ChangeSet &changes);

    /// Offset for the data being generated
    void setDrawOffset(int inDrawOffset) { drawOffset = inDrawOffset; }
    int getDrawOffset() { return drawOffset; }

    /// Priority order to use in the renderer
    void setDrawPriority(int inDrawPriority) { drawPriority = inDrawPriority; }
    int getDrawPriority() { return drawPriority; }
    
    /// If set, the point at which tile geometry will appear when zoomed in
    void setMinVis(float inMinVis) { minVis = inMinVis; }
    float getMinVis() { return minVis; }
    
    /// If set, the point at which tile geometry will disappear when zoomed outfloat maxVis;
    void setMaxVis(float inMaxVis) { maxVis = inMaxVis; }
    float getMaxVis() { return maxVis; }

    /// If set, the point at which we'll stop doing updates (separate from minVis)
    void setMinPageVis(float inVal) { minPageVis = inVal; }
    float getMinPageVis() { return minPageVis; }
    
    /// If set, the point at which we'll stop doing updates (separate from maxVis)
    void setMaxPageVis(float inVal) { maxPageVis = inVal; }
    float getMaxPageVis() { return maxPageVis; }

    /// If set, the program to use for rendering
    void setProgramId(SimpleIdentity inVal) { programId = inVal; }
    SimpleIdentity getProgramId() { return programId; }
    
    /// If set, we'll include elevation (Z) in the drawables for shaders to use
    void setIncludeElev(bool inVal) { includeElev = inVal; }
    bool getIncludeElev() { return includeElev; }
    
    /// If set (by default) we'll use the elevation (if provided) as real Z values on the vertices
    void setUseElevAsZ(bool inVal) { useElevAsZ = inVal; }
    bool getUseElevAsZ() { return useElevAsZ; }
    
    /// The number of image layers we're expecting to be given.  By default, 1
    void setNumImages(unsigned int inVal) { numImages = inVal; }
    unsigned int getNumImages() { return numImages; }
    
    /// Number of active textures we'll have in drawables.  Informational only.
    int getActiveTextures() { return activeTextures; }
    
    /// Base color for the drawables created by the layer
    void setColor(RGBAColor inColor) { color = inColor; }
    RGBAColor getColor() { return color; }
    
    /// Set this if the tile images are partially transparent
    void setHasAlpha(bool inVal) { hasAlpha = inVal; }
    bool getHasAlpha() { return hasAlpha; }
    
    /// Data layer we're attached to
    QuadDisplayController *getController() { return control; }
    
    /// If set, we'll ignore edge matching.
    /// This can work if you're zoomed in close
    void setIgnoreEdgeMatching(bool inVal) { ignoreEdgeMatching = inVal; }
    bool getIgnoreEdgeMatching() { return ignoreEdgeMatching; }
    
    /// If set, we'll fill in the poles for a projection that doesn't go all the way up or down
    void setCoverPoles(bool inVal) { coverPoles = inVal; }
    bool getCoverPoles() { return coverPoles; }
    
    /// The data type of GL textures we'll be creating.  RGBA by default.
    void setImageType(WhirlyKitTileImageType inType) { imageType = inType; }
    WhirlyKitTileImageType getImageType() { return imageType; }
    
    /// If set (before we start) we'll use dynamic texture and drawable atlases
    void setUseDynamicAtlas(bool inVal) { useDynamicAtlas = inVal; }
    bool getUseDynamicAtlas() { return useDynamicAtlas; }
    
    /// If set we'll scale the input images to the nearest square power of two
    void setTileScale(WhirlyKitTileScaleType inScale) { tileScale = inScale; }
    WhirlyKitTileScaleType getTileScale() { return tileScale; }
    
    /// If the tile scale is fixed, this is the size it's fixed to (256 by default)
    void setFixedTileSize(int inVal) { fixedTileSize = inVal; }
    int getFixedTileSize() { return fixedTileSize; }
    
    /// If set, the default texture atlas size.  Must be a power of two.
    void setTextureAtlasSize(int inVal) { textureAtlasSize = inVal; }
    int getTextureAtlasSize() { return textureAtlasSize; }
    
    /// How many texels we put around the borders of each tile
    void setBorderTexel(int inVal) { borderTexel = inVal; }
    int getBorderTexel() { return borderTexel; }
    
    /// A fudge factor for border texels.  This lets us pretend we have them
    ///  and split the difference between bogus and ugly.
    void setBorderPixelFudge(float fudge) { texAtlasPixelFudge = fudge; }
    float getBorderPixelFudge() { return texAtlasPixelFudge; }
    
protected:
    void clear();
    void refreshParents();
    InternalLoadedTile *getTile(const Quadtree::Identifier &ident);
    void flushUpdates(ChangeSet &changes);
    void runSetCurrentImage(ChangeSet &changes);
    void updateTexAtlasMapping();

    pthread_mutex_t tileLock;

    /// Delegate used to provide images
    QuadTileImageDataSource *imageSource;
    std::string name;

    bool enable;
    int numLoadingFrames;
    float drawOffset;
    int drawPriority;
    
    float minVis;
    float maxVis;
    float minPageVis;
    float maxPageVis;

    WhirlyKit::SimpleIdentity programId;
    bool includeElev;
    bool useElevAsZ;
    
    unsigned int numImages;
    int activeTextures;
    WhirlyKit::RGBAColor color;
    bool hasAlpha;
    
    bool ignoreEdgeMatching;
    bool coverPoles;
    
    WhirlyKitTileImageType imageType;
    bool useDynamicAtlas;
    WhirlyKitTileScaleType tileScale;
    int fixedTileSize;
    int textureAtlasSize;
    int borderTexel;
    float texAtlasPixelFudge;

    TileBuilder *tileBuilder;
    bool doingUpdate;
    int defaultTessX,defaultTessY;
    
    /// Tiles we currently have loaded in the scene
    WhirlyKit::LoadedTileSet tileSet;

    // Parents to update after changes
    std::set<WhirlyKit::Quadtree::Identifier> parents;
    
    /// Change requests queued up between a begin and end
    ChangeSet changeRequests;

    // The images we're currently displaying, when we have more than one
    int currentImage0,currentImage1;

    // Keep track of the slow (e.g. network) fetches
    std::set<WhirlyKit::Quadtree::Identifier> networkFetches,localFetches;
    
};

}
