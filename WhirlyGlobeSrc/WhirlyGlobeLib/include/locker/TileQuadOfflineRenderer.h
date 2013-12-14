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
#import "TileQuadLoader.h"

@class WhirlyKitQuadTileOfflineLoader;

/// The offline renderer passes over images like so
@interface WhirlyKitQuadTileOfflineImage : NSObject
{
@public
    /// Size of each of the corner pixels in meters
    CGSize cornerSizes[4];
}

/// Bounding box for the rendered area
@property (nonatomic) WhirlyKit::Mbr &mbr;

/// Textures produced by the offline renderer.  Delegate is responsible for cleanup
@property (nonatomic,assign) std::vector<WhirlyKit::SimpleIdentity> &textures;

/// Size of the center pixel in meters
@property (nonatomic) CGSize centerSize;

@end

/** Fill in this delegate to receive the UIImage this layer
    generates every period seconds.
  */
@protocol WhirlyKitQuadTileOfflineDelegate <NSObject>

/// Here's the generated image.  Query the loader for extents.
- (void)loader:(WhirlyKitQuadTileOfflineLoader *)loader image:(WhirlyKitQuadTileOfflineImage *)image;

@end

/** This version of the quad tile loader requests and tracks images the same
    as the normal one.  Then it assembles them into a single large image
    covering a given set of extents.  This lets us do 'postage stamp' style
    display of a big complex, areal.
  */
@interface WhirlyKitQuadTileOfflineLoader : NSObject<WhirlyKitQuadLoader,WhirlyKitQuadTileLoaderSupport>

/// Set this up with an object that'll return an image per tile and a name (for debugging)
- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// Set if we're doing any rendering.  On by default.
@property (nonatomic,assign) bool on;

/// Depth of the image stack per tile
@property (nonatomic,assign) int numImages;

/// Size (in pixels) of the output image we're building
@property (nonatomic,assign) int sizeX,sizeY;

/// If set, the output size is a maximum.  We'll try to track input resolution
@property (nonatomic,assign) bool autoRes;

/// The bounding box for the image we're trying to build
@property (nonatomic,assign) WhirlyKit::Mbr &mbr;

/// We want the image generate no more often than this
@property (nonatomic,assign) NSTimeInterval period;

/// When the MBR changes we get a preview render down to this many levels
@property (nonatomic,assign) int previewLevels;

/// If set, the delegate that receives the image we're generating every period seconds
@property (nonatomic,weak) NSObject<WhirlyKitQuadTileOfflineDelegate> *outputDelegate;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

/// When a data source has finished its fetch for a given tile, it
///  calls this method to hand the data (along with key info) back to the
///  quad tile loader.
/// You can pass back a WhirlyKitLoadedTile or a WhirlyKitLoadedImage or
///  just a WhirlyKitElevationChunk.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row;

@end
