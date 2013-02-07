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

/// @cond
@class WhirlyKitQuadTileLoader;
/// @endcond

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
    void addToScene(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Scene *scene,NSData *imageData,int pvrtcSize,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Scene *scene,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(WhirlyKitQuadTileLoader *loader,WhirlyKitQuadDisplayLayer *layer,WhirlyKit::Quadtree *tree,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Dump out to the log
    void Print(WhirlyKit::Quadtree *tree);
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::NodeInfo nodeInfo;
    
    /// Set if this parent tile is on
    bool isOn;
    /// Set if this tile is in the process of loading
    bool isLoading;
    // DrawID for this parent tile
    WhirlyKit::SimpleIdentity drawId;
    // Optional ID for the skirts
    WhirlyKit::SimpleIdentity skirtDrawId;
    // Texture ID for the parent tile
    WhirlyKit::SimpleIdentity texId;
    
    // Set for each child that's on.  That is, that we're drawing as filler.
    bool childIsOn[4];
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

/// The quad loader is letting us know to start loading the image.
/// We'll call the loader back with the image when it's ready
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row;
@end

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

/// Set this up with an object that'll return an image per tile
- (id)initWithDataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

/// When a data source has finished its fetch for a given image, it calls
///  this method to hand that back to the quad tile loader
/// If this isn't called in the layer thread, it will switch over to that thread first.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(NSData *)image pvrtcSize:(int)pvrtcSize forLevel:(int)level col:(int)col row:(int)row;

@end
