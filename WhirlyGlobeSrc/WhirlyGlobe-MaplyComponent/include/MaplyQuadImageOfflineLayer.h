/*
 *  MaplyQuadImageOfflineLayer.h
 *  WhirlyGlobe-MaplyComponent
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

#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"
#import "MaplyTexture.h"

@class MaplyQuadImageOfflineLayer;

/// The offline renderer passes over images like so
@interface MaplyOfflineImage : NSObject
{
@public
    /// Size of each of the corner pixels in meters
    CGSize cornerSizes[4];
}

/// Bounding box for the rendered area
@property (nonatomic) MaplyBoundingBox bbox;

/// Images produced by the renderer represented as MaplyTexture objects
@property (nonatomic) NSArray *textures;

/// Size of the center pixel in meters
@property (nonatomic) CGSize centerSize;

@end


/** @brief Protocol for receiving an offline rendered image.
    @details This is used by the MaplyQuadImageOfflineLayer to hand over images as they are rendered.
  */
@protocol MaplyQuadImageOfflineDelegate<NSObject>

/** @brief Delegate call for the image stack that's been rendered by the MaplyQuadImageOfflineLayer.
    @details This is the callback method to get the images rendered by the MaplyQuadImageOfflineLayer.  You fill this in and then do whatever you may want with the data.  You'll be called on a random thread here, so act accordingly.
    @param layer The offline rendering layer producing the image stack.
    @param images One or more images rendered by the offline layer.  The size of the array will correspond to the depth of the tile source provided to the offline layer.
    @param bbox The bounding box of the images in the coordinate system of the offline layer.
  */
- (void)offlineLayer:(MaplyQuadImageOfflineLayer *)layer images:(MaplyOfflineImage *)offlineImage;

@end

/** @brief The quad image offline layer renders a single image at irregular intervals from a standard input tile source.
    @details The offline renderer will fetch a tile image basemap like a normal MaplyQuadImageTilesLayer.  What it does with it is completely different.  It renders a stack of static images from it and calls a delegate with them.
    @details The images produced correspond to the bbox set. Think of it as providing a window into a tiled base map.  Why?  Eh, it's complicated.
  */
@interface MaplyQuadImageOfflineLayer : MaplyViewControllerLayer

/** @brief Initialize with the coordinate system of the tile source and the tile source.
    @details The coordinate system needs to be either MaplyPlateCarree or MaplySphericalMercator at the moment.  The tile source should provide at least one and possibly several images per tile.
    @param coordSys The coordinate system the offline layer will render in.
    @param tileSource The tile source that needs to provide at least one and possibly more images per tile.
  */
- (id)initWithCoordSystem:(MaplyCoordinateSystem *)coordSys tileSource:(NSObject<MaplyTileSource> *)tileSource;

/** @brief Enable/Disable the whole layer.
    @details By default this is on.  When off, the layer will stop working and calling its delegate.
 */
@property (nonatomic,assign) bool on;

/** @brief The number of simultaneous fetches the layer will attempt at once.
    @details The toolkit loves its dispatch queues and threads.  By default this number is set to 8 or 16, but if you need to constrain it, you can set it lower (or higher!).  If your tile source can't handle multi-thread access, set this to 1.
 */
@property (nonatomic,assign) int numSimultaneousFetches;

/** @brief The number of images we're expecting to get per tile.
    @details This is the number of images the layer will ask for per tile.  The default is 1, which is the normal case.  If this is greater than one that typically means we're going to animate between them.
    @details the MaplyTileSource delegate is always expected to provide this many imates.
 */
@property (nonatomic,assign) unsigned int imageDepth;

/** @brief Control how tiles are indexed, either from the lower left or the upper left.
     @details If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
     @details Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if you're tile source looks odd, try setting this to false.
 */
@property (nonatomic) bool flipY;

/** @brief Maximum number of tiles to load in at once.
     @details This is the maximum number of tiles the pager will have loaded into memory at once.  The default is 128 and that's generally good enough.  However, if your tile size is small, you may want to load in more.
     @details Tile loading can get out of control when using elevation data.  The toolkit calculates potential sceen coverage for each tile so elevation data makes all tiles more important.  As a result the system will happily page in way more data than you may want.  The limit becomes important in elevation mode, so leave it at 128 unless you need to change it.
 */
@property (nonatomic) int maxTiles;

/** @brief Scale the calculated importance by this before using it.
    @details Number less than 1.0 will make tiles less important to load in.
  */
@property (nonatomic) float importanceScale;

/** @brief Number of levels to consider in tiles when previewing.
    @details When we chnage the bounds we'll run off a quick preview render if this is set.  If so, we'll consider an tile above or at this level.
  */
@property (nonatomic) int previewLevels;

/** @brief Size of the image to produce or maximum size if autoRes is on.
    @details This is the size of each of the images passed to the delegate.  If autoRes is on this is the maximum size.
  */
@property (nonatomic) CGSize textureSize;

/** @brief If on the layer will try to match the textureSize to the maximum resolution of the tiles it sees.  On by default.
  */
@property (nonatomic) bool autoRes;

/** @brief Controls whether the fetching code runs in a single thread or is spawned asyncronously.
    @details If set, we'll kick off the tile fetches in their own dispatched blocks.  If not set, we'll just do it in the layer thread.
 */
@property (nonatomic,assign) bool asyncFetching;

/** @brief How often we'll generate a new image.
    @details This is the most often the layer will produce a new set of images and call the delegate.
  */
@property (nonatomic) float period;

/** @brief The bounding box for the images produced by the offline layer.
    @details This bounding box should be in the layer's coordinate system.
  */
@property (nonatomic) MaplyBoundingBox bbox;

/** @brief The delegate called with the image stack produced at irregular intervals.
    @details Set this delegate to get the images out of the offline rendering layer.
  */
@property (nonatomic,weak) NSObject<MaplyQuadImageOfflineDelegate> *delegate;

/// @brief Force the layer to reload its tiles and rerender.
- (void)reload;

@end
