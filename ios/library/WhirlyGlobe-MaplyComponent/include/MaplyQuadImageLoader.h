/*
 *  MaplyQuadImageLoader.h
 *
 *  Created by Steve Gifford on 4/10/18.
 *  Copyright 2012-2018 Saildrone Inc
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
#import "MaplyRemoteTileSource.h"
#import "MaplyRenderController.h"
#import "MaplyQuadSampler.h"
#import "MaplyRemoteTileFetcher.h"

/**
  Passed in to and returned by the Loader Interpreter.
 
  We pass this into the interpreter with the unparsed data.  It parses it and passes that
  data back, possibly with an error.
 */
@interface MaplyLoaderReturn : NSObject

/// Tile this is the image for
@property (nonatomic,assign) MaplyTileID tileID;

/// If set, the frame.  -1 by default
@property (nonatomic,assign) int frame;

/// Data returned from a tile request.  Unparsed.
@property (nonatomic,strong) NSData * __nonnull tileData;

/// If you have more than one tileInfo, you'll get your data back here unparsed.
@property (nonatomic,strong) NSArray * __nullable multiTileData;

/// Can be zero or more UIImage or an NSData containing an image or a MaplyImageTile
@property (nonatomic,strong) NSArray *__nullable images;

/// If any component objects are associated with the tile, these are them.
/// They need to start disabled.  The system will enable and delete them when it is time.
@property (nonatomic,strong) NSArray * __nullable compObjs;

/// These component objects are assumed to be overlaid and so only one
/// set will be displayed at a time.
@property (nonatomic,strong) NSArray * __nullable ovlCompObjs;

/// If this is set, the tile failed to parse and maybe why.
@property (nonatomic,strong) NSError * __nullable error;

@end

/**
    Protocol for turning raw data into images and objects.
 
    The Loader Interpretor converts data returned from a remote source (or cache) into
    images and/or MaplyComponentObjects that have already been added to the view (disabled).
 
    The implementation of these is dependent on the data source.  One for vector tiles,
    another for images, and so on.
  */
@protocol MaplyLoaderInterpreter<NSObject>

/**
 Parse the data coming back from a remote request and turn it into something we can use.
 
 Convert the NSData passed in to image and component objects (e.g. add stuff to the view controller).
 Everything added should be disabled to start.
  */
- (void)parseData:(MaplyLoaderReturn * __nonnull)loadReturn;

@end

/**
    Image loader intrepreter turns NSData objects into MaplyImageTiles.
 
    This is the default interpreter used by the MaplyQuadImageLoader.  It's expecting
    to get a single NSData object containing an PNG or JPEG.  It will turn those
    into data suitable for display.
  */
@interface MaplyImageLoaderInterpreter : NSObject<MaplyLoaderInterpreter>
@end

@class MaplyQuadImageLoaderBase;

/**
    Debugging interpretor that makes tiles.
 
    This loader interpreter sticks a designator in the middle of tiles
    and a line around the edge.  Nice for debugging.
 
    Initialize it with the real loader interpreter you want to use.
  */
@interface MaplyDebugImageLoaderInterpreter : MaplyImageLoaderInterpreter

/// Initialize with the loader we're using and the view controller.
- (instancetype __nonnull)initWithLoader:(MaplyQuadImageLoaderBase * __nonnull)inLoader viewC:(MaplyBaseViewController * __nonnull)viewC;

@end

/// Name of the shared MaplyRemoteTileFetcher
extern NSString * _Nonnull const MaplyQuadImageLoaderFetcherName;

/**
  Base object for Maply Quad Image loader.
 
  Look to the subclasses for actual functionality.  This hold methods they share.
  */
@interface MaplyQuadImageLoaderBase : NSObject

/// Set the draw priority values for produced tiles
@property (nonatomic) int baseDrawPriority;

/// Offset between levels for a calculated draw priority
@property (nonatomic) int drawPriorityPerLevel;

/// Base color for geometry produced
@property (nonatomic,retain,nonnull) UIColor *color;

/// Write to the z buffer when rendering.  On by default
@property (nonatomic,assign) bool zBufferWrite;

/// Read from the z buffer when rendering.  Off by default
@property (nonatomic,assign) bool zBufferRead;

/**
 Shader to use for rendering the image frames.
 
 If not set, we'll pick the default visual shader.
 */
- (void)setShader:(MaplyShader * __nullable)shader;

/**
 An optional render target for this loader.
 
 The loader can draw to a render target rather than to the screen.
 You use this in a multi-pass rendering setup.
 */
- (void)setRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget;

/**
 Scale the importance values passed to the loader and used for a loading cutoff.
 
 A larger value means the tile needs to take up *more* space to be loaded.  So bigger values
 mean less loading.  The importance values are pixels^2.
 */
@property (nonatomic) double importanceScale;

/// Any tiles less important than this (screen area in pixels^2) won't be loaded
@property (nonatomic) double importanceCutoff;

/**
 Set the image format for internal imagery storage.
 
 OpenGL ES offers us several image formats that are more efficient than 32 bit RGBA, but they're not always appropriate.  This property lets you choose one of them.  The 16 or 8 bit ones can save a huge amount of space and will work well for some imagery, most maps, and a lot of weather overlays.
 
 Be sure to set this at layer creation, it won't do anything later on.

 - Remark:
 
 * MaplyImageIntRGBA 32 bit RGBA with 8 bits per channel.  The default.
 * MaplyImageUShort565 16 bits with 5/6/5 for RGB and none for A.
 * *MaplyImageUShort4444* | 16 bits with 4 bits for each channel. |
 * *MaplyImageUShort5551* | 16 bits with 5/5/5 bits for RGB and 1 bit for A. |
 * *MaplyImageUByteRed( | 8 bits, where we choose the R and ignore the rest. |
 
 | MaplyImageUByteGreen | 8 bits, where we choose the G and ignore the rest. |
 | MaplyImageUByteBlue | 8 bits, where we choose the B and ignore the rest. |
 | MaplyImageUByteAlpha | 8 bits, where we choose the A and ignore the rest. |
 | MaplyImageUByteRGB | 8 bits, where we average RGB for the value. |
 | MaplyImage4Layer8Bit | 32 bits, four channels of 8 bits each.  Just like MaplyImageIntRGBA, but a warning not to do anything too clever in sampling. |
 */
@property (nonatomic) MaplyQuadImageFormat imageFormat;

/**
 Number of border texels to set up around image tiles.
 
 For matching image tiles along borders in 3D (probably the globe) we resample the image slightly smaller than we get and make up a boundary around the outside.  This number controls that border size.
 
 By default this is 1.  It's safe to set it to 0 for 2D maps and some overlays.
 */
@property (nonatomic) int borderTexel;

/**
 Control how tiles are indexed, either from the lower left or the upper left.
 
 If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
 
 Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if you're tile source looks odd, try setting this to false.
 
 Default value is true.
 */
@property (nonatomic) bool flipY;

/// Set for a lot of debugging output
@property (nonatomic,assign) bool debugMode;

/// Use a specific tile fetcher rather than the one shared by everyone else
- (void)setTileFetcher:(NSObject<MaplyTileFetcher> * __nonnull)tileFetcher;

/// Set the interpreter for the data coming back.  If you're just getting images, don't set this.
- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> * __nonnull)interp;

/**
 Calculate the bounding box for a single tile in geographic.
 
 This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBox in case of error
 */
- (MaplyBoundingBox)geoBoundsForTile:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in geographic using doubles.
 
 This is a utility method for calculating the extents of a given tile in geographic (e.g. lon/lat).
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates. Returns kMaplyNullBoundingBoxD in case of error
 */
- (MaplyBoundingBoxD)geoBoundsForTileD:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in the local coordinate system.
 
 This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates.
 */
- (MaplyBoundingBox)boundsForTile:(MaplyTileID)tileID;

/**
 Calculate the bounding box for a single tile in the local coordinate system using doubles.
 
 This utility method calculates the bounding box for a tile in the coordinate system used for the layer.
 
 @param tileID The ID for the tile we're interested in.
 
 @return The lower left and upper right corner of the tile in geographic coordinates.
 */
- (MaplyBoundingBoxD)boundsForTileD:(MaplyTileID)tileID;

@end

/**
 The Maply Quad Image Loader is for paging image pyramids local or remote.
 
 This layer pages image pyramids.  They can be local or remote, in any coordinate system Maply supports and you provide a MaplyTileInfoNew conformant object to do the actual image tile fetching.
 
 You probably don't have to implement your own tile source.  Go look at the MaplyRemoteTileInfoNew and MaplyMBTileFetcher objects.  Those will do remote and local fetching.
 */
@interface MaplyQuadImageLoader : MaplyQuadImageLoaderBase

/**
 Initialize with a single tile info object and the sampling parameters.
 
 @param params The sampling parameters describing how to break down the data for projection onto a globe or map.
 @param tileInfo A single tile info object describing where the data is and how to get it.
 @param viewC the View controller (or renderer) to add objects to.
 */
- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params tileInfo:(NSObject<MaplyTileInfoNew> *__nonnull)tileInfo viewC:(MaplyBaseViewController * __nonnull)viewC;

/**
  Initialize with multiple tile sources and sampling parameters.
 
 @param params The sampling parameters describing how to break down the data for projection onto a globe or map.
 @param tileInfos A list of tile info objects to fetch for each tile.  If one fails, the tile fails to load.
 @param viewC the View controller (or renderer) to add objects to.
  */
- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params tileInfos:(NSArray<NSObject<MaplyTileInfoNew> *> *__nonnull)tileInfos viewC:(MaplyBaseViewController * __nonnull)viewC;

/** Turn off the image loader and shut things down.
    This unregisters us with the sampling layer and shuts down the various objects we created.
  */
- (void)shutdown;

@end
