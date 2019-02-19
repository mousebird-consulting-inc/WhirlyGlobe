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
#import "MaplyTileSourceNew.h"
#import "MaplyRenderController.h"
#import "MaplyQuadSampler.h"
#import "MaplyRemoteTileFetcher.h"
#import "MaplyQuadLoader.h"

/// Name of the shared MaplyRemoteTileFetcher
extern NSString * _Nonnull const MaplyQuadImageLoaderFetcherName;

/**
  Base object for Maply Quad Image loader.
 
  Look to the subclasses for actual functionality.  This holds methods they share.
  */
@interface MaplyQuadImageLoaderBase : MaplyQuadLoaderBase

// Set the draw priority values for produced tiles
@property (nonatomic) int baseDrawPriority;

// Offset between levels for a calculated draw priority
@property (nonatomic) int drawPriorityPerLevel;

// Base color for geometry produced
@property (nonatomic,retain,nonnull) UIColor *color;

// Write to the z buffer when rendering.  On by default
@property (nonatomic,assign) bool zBufferWrite;

// Read from the z buffer when rendering.  Off by default
@property (nonatomic,assign) bool zBufferRead;

/**
 Shader to use for rendering the image frames.
 
 If not, set we'll pick the default visual shader.
 */
- (void)setShader:(MaplyShader * __nullable)shader;

/**
 An optional render target for this loader.
 
 The loader can draw to a render target rather than to the screen.
 You use this in a multi-pass rendering setup.
 */
- (void)setRenderTarget:(MaplyRenderTarget *__nonnull)renderTarget;

/**
 Set the image format for internal imagery storage.
 
 OpenGL ES offers us several image formats that are more efficient than 32 bit RGBA, but they're not always appropriate.  This property lets you choose one of them.  The 16 or 8 bit ones can save a huge amount of space and will work well for some imagery, most maps, and a lot of weather overlays.
 
 Be sure to set this at layer creation, it won't do anything later on.
 
 | Image Format | Description |
 |:-------------|:------------|
 | MaplyImageIntRGBA | 32 bit RGBA with 8 bits per channel.  The default. |
 | MaplyImageUShort565 | 16 bits with 5/6/5 for RGB and none for A. |
 | MaplyImageUShort4444 | 16 bits with 4 bits for each channel. |
 | MaplyImageUShort5551 | 16 bits with 5/5/5 bits for RGB and 1 bit for A. |
 | MaplyImageUByteRed | 8 bits, where we choose the R and ignore the rest. |
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

/// Use a specific tile fetcher rather than the one shared by everyone else
- (void)setTileFetcher:(NSObject<MaplyTileFetcher> * __nonnull)tileFetcher;

/// Set the interpreter for the data coming back.  If you're just getting images, don't set this.
- (void)setInterpreter:(NSObject<MaplyLoaderInterpreter> * __nonnull)interp;

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
- (nullable instancetype)initWithParams:(MaplySamplingParams *__nonnull)params tileInfo:(NSObject<MaplyTileInfoNew> *__nullable)tileInfo viewC:(MaplyBaseViewController * __nonnull)viewC;

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
