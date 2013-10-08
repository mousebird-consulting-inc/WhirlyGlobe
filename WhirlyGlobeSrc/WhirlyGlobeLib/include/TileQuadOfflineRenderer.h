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

/** This version of the quad tile loader requests and tracks images the same
    as the normal one.  Then it assembles them into a single large image
    covering a given set of extents.  This lets us do 'postage stamp' style
    display of a big complex, areal.
  */
@interface WhirlyKitQuadTileOfflineLoader : NSObject<WhirlyKitQuadLoader,WhirlyKitQuadTileLoaderSupport>

/// Set this up with an object that'll return an image per tile and a name (for debugging)
- (id)initWithName:(NSString *)name dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)imageSource;

/// The bounding box for the image we're trying to build
@property (nonatomic,assign) WhirlyKit::Mbr &mbr;

/// Size (in pixels) of the output image we're building
@property (nonatomic,assign) int sizeX,sizeY;

/// Called when the layer shuts down
- (void)shutdownLayer:(WhirlyKitQuadDisplayLayer *)layer scene:(WhirlyKit::Scene *)scene;

/// When a data source has finished its fetch for a given tile, it
///  calls this method to hand the data (along with key info) back to the
///  quad tile loader.
/// You can pass back a WhirlyKitLoadedTile or a WhirlyKitLoadedImage or
///  just a WhirlyKitElevationChunk.
- (void)dataSource:(NSObject<WhirlyKitQuadTileImageDataSource> *)dataSource loadedImage:(id)loadImage forLevel:(int)level col:(int)col row:(int)row;

@end
