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
#import "LoadedTile.h"

/// @cond
@class WhirlyKitQuadTileLoader;
@protocol WhirlyKitQuadTileImageDataSource;
/// @endcond

/** This protocol outlines the method that a WhirlyKitQuadTileImageDataSource
 compliant object uses to tell the tile loader that a tile has loaded or
 failed to load.  We break it out so that other objects can talk to
 tile loading objects without subclassing WhirlyKitQuadTileLoader.
 */
@protocol WhirlyKitQuadTileLoaderSupport<NSObject>

/// When a data source has finished its fetch for a given tile, it
///  calls this method to hand the data (along with key info) back to the
///  quad tile loader.
/// You can pass back a WhirlyKitLoadedTile or a WhirlyKitLoadedImage or
///  just a WhirlyKitElevationChunk.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row frame:(int)frame;

/// Older version that doesn't support frame loads
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row;

@end

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
- (void)quadTileLoader:(NSObject<WhirlyKitQuadTileLoaderSupport> *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row __deprecated;

/// This version of the load method passes in a mutable dictionary.
/// Store your expensive to generate key/value pairs here.
- (void)quadTileLoader:(NSObject<WhirlyKitQuadTileLoaderSupport> *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs;

/// This version can load frames individually.  It's used for loading
/// animations.  It's not required.
- (void)quadTileLoader:(NSObject<WhirlyKitQuadTileLoaderSupport> *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row frame:(int)frame attrs:(NSMutableDictionary *)attrs;

/// Check if the given tile is a local or remote fetch.  This is a hint
///  to the pager.  It can display local tiles as a group faster.
- (bool)tileIsLocalLevel:(int)level col:(int)col row:(int)row frame:(int)frame;

/// An optional callback provided when a tile is unloaded.
/// You don't have to do anything
- (void)tileWasUnloadedLevel:(int)level col:(int)col row:(int)row;

/// Optional callback for when tile is made visible
- (void)tileWasEnabledLevel:(int)level col:(int)col row:(int)row;

/// Optional callback for when tile is made invisible
- (void)tileWasDisabledLevel:(int)level col:(int)col row:(int)row;

@end

/** The Globe Quad Tile Loader responds to the Quad Loader protocol and
    creates simple terrain (chunks of the sphere) and asks for images
    to put on top.
 */
@interface WhirlyKitQuadTileLoader : NSObject<WhirlyKitQuadLoader,WhirlyKitQuadTileLoaderSupport>

/// Offset for the data being generated
@property (nonatomic,assign) int drawOffset;
/// Priority order to use in the renderer
@property (nonatomic,assign) int drawPriority;
/// If set, the point at which tile geometry will appear when zoomed in
@property (nonatomic,assign) float minVis;
/// If set, the point at which tile geometry will disappear when zoomed outfloat maxVis;
@property (nonatomic,assign) float maxVis;
/// If set, the point at which we'll stop doing updates (separate from minVis)
@property (nonatomic,assign) float minPageVis;
/// If set, the point at which we'll stop doing updates (separate from maxVis)
@property (nonatomic,assign) float maxPageVis;
/// If set, the program to use for rendering
@property (nonatomic,assign) WhirlyKit::SimpleIdentity programId;
/// If set, we'll include elevation (Z) in the drawables for shaders to use
@property (nonatomic,assign) bool includeElev;
/// If set (by default) we'll use the elevation (if provided) as real Z values on the vertices
@property (nonatomic,assign) bool useElevAsZ;
/// The number of image layers we're expecting to be given.  By default, 1
@property (nonatomic,assign) unsigned int numImages;
/// Number of active textures we'll have in drawables.  Informational only.
@property (nonatomic,readonly) int activeTextures;
/// Base color for the drawables created by the layer
@property (nonatomic,assign) WhirlyKit::RGBAColor color;
/// Set this if the tile images are partially transparent
@property (nonatomic,assign) bool hasAlpha;
/// Data layer we're attached to
@property (nonatomic,weak) WhirlyKitQuadDisplayLayer *quadLayer;
/// If set, we'll ignore edge matching.
/// This can work if you're zoomed in close
@property (nonatomic,assign) bool ignoreEdgeMatching;
/// If set, we'll fill in the poles for a projection that doesn't go all the way up or down
@property (nonatomic,assign) bool coverPoles;
/// The data type of GL textures we'll be creating.  RGBA by default.
@property (nonatomic,assign) WhirlyKitTileImageType imageType;
/// If set (before we start) we'll use dynamic texture and drawable atlases
@property (nonatomic,assign) bool useDynamicAtlas;
/// If set we'll scale the input images to the nearest square power of two
@property (nonatomic,assign) WhirlyKitTileScaleType tileScale;
/// If the tile scale is fixed, this is the size it's fixed to (256 by default)
@property (nonatomic,assign) int fixedTileSize;
/// If set, the default texture atlas size.  Must be a power of two.
@property (nonatomic,assign) int textureAtlasSize;
/// How many texels we put around the borders of each tile
@property (nonatomic,assign) int borderTexel;

/// Set this up with an object that'll return an image per tile
- (id)initWithDataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// Set this up with an object that'll return an image per tile and a name (for debugging)
- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// If you're passing in elevation (even some of the time), set this to the maximum
///  sampling you're going to pass in.  If you don't set this, you may lose tiles.
- (void)setTesselationSizeX:(int)x y:(int)y;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

/// When a data source has finished its fetch for a given image, it calls
///  this method to hand that back to the quad tile loader
/// If this isn't called in the layer thread, it will switch over to that thread first.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(NSData *)image pvrtcSize:(int)pvrtcSize forLevel:(int)level col:(int)col row:(int)row __deprecated;

/// When a data source has finished its fetch for a given tile, it
///  calls this method to hand the data (along with key info) back to the
///  quad tile loader.
/// You can pass back a WhirlyKitLoadedTile or a WhirlyKitLoadedImage or
///  just a WhirlyKitElevationChunk.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row frame:(int)frame;

/// Set up the change requests to make the given image layer the active one
/// The call is thread safe
- (void)setCurrentImage:(int)newImage changes:(WhirlyKit::ChangeSet &)changeRequests;

/// Set up the change requests to make the given images current.
/// This will also interpolate between the two
- (void)setCurrentImageStart:(int)startImage end:(int)endImage changes:(WhirlyKit::ChangeSet &)changeRequests;

/// By default we're on, but we can be turned off
- (void)setEnable:(bool)enable;

@end
