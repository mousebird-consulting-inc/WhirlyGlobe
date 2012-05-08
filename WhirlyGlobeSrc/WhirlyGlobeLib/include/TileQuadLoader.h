/*
 *  TileQuadLoader.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/27/12.
 *  Copyright 2011 mousebird consulting
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
#import "GlobeScene.h"
#import "DataLayer.h"
#import "RenderCache.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "Quadtree.h"
#import "SceneRendererES1.h"
#import "GlobeQuadDisplayLayer.h"

/// @cond
@class WhirlyGlobeQuadTileLoader;
/// @endcond

namespace WhirlyGlobe
{
    
/** The Loaded Tile is used to track tiles that have been
 loaded in to memory, but may be in various states.  It's also
 used to fill in child outlines that may be missing.
 */
class LoadedTile
{
public:
    LoadedTile();
    ~LoadedTile() { }
    
    /// Build the data needed for a scene representation
    void addToScene(WhirlyGlobeQuadTileLoader *loader,WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene);
    
    /// Remove data from scene.  This just sets up the changes requests.
    /// They must still be passed to the scene
    void clearContents(WhirlyGlobeQuadTileLoader *loader,WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Update what we're displaying based on the quad tree, particulary for children
    void updateContents(WhirlyGlobeQuadTileLoader *loader,WhirlyGlobeQuadDisplayLayer *layer,WhirlyKit::Quadtree *tree,std::vector<WhirlyKit::ChangeRequest *> &changeRequests);
    
    /// Dump out to the log
    void Print(WhirlyKit::Quadtree *tree);
    
    // Details of which node we're representing
    WhirlyKit::Quadtree::NodeInfo nodeInfo;
    
    /// Set if this parent tile is on
    bool isOn;
    // DrawID for this parent tile
    WhirlyKit::SimpleIdentity drawId;
    // Texture ID for the parent tile
    WhirlyKit::SimpleIdentity texId;
    
    // Set for each child that's on.  That is, that we're drawing as filler.
    bool childIsOn[4];
    // IDs for the various fake child geometry
    WhirlyKit::SimpleIdentity childDrawIds[4];
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
@protocol WhirlyGlobeQuadTileImageDataSource<NSObject>
/// If there is an image, return it.  Null otherwise.
- (NSData *)fetchImageForLevel:(int)level col:(int)col row:(int)row;
@end

/** The Globe Quad Tile Loader responds to the Quad Loader protocol and
    creates simple terrain (chunks of the sphere) and asks for images
    to put on top.
 */
@interface WhirlyGlobeQuadTileLoader : NSObject<WhirlyGlobeQuadLoader>
{    
    /// Tiles we currently have loaded in the scene
    WhirlyGlobe::LoadedTileSet tileSet;    
    
    /// Delegate used to provide images
    NSObject<WhirlyGlobeQuadTileImageDataSource> * __weak dataSource;
    
    // We'll gather up all the changes for one step
    // This avoids flashing
    std::vector<WhirlyKit::ChangeRequest *> changeRequests;    

    // Parents to update after changes
    std::set<WhirlyKit::Quadtree::Identifier> parents;
    
    /// Offset for the data being generated
    int drawOffset;
    
    /// Priority order to use in the renderer
    int drawPriority;
    
    /// Set this if the tiles are partially transparent
    bool hasAlpha;
}

@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) bool hasAlpha;

/// Set this up with an object that'll return an image per tile
- (id)initWithImageDataSource:(NSObject<WhirlyGlobeQuadTileImageDataSource> *)imageSource;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyGlobeQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

// Convenience function for createin a quad tile loader with data source
+ (WhirlyGlobeQuadTileLoader *)loaderWithImageSource:(NSObject<WhirlyGlobeQuadTileImageDataSource> *)imageSource;

@end
