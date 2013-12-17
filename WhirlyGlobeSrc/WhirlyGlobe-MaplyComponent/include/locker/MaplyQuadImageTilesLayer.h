/*
 *  MaplyQuadImageTilesLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/13/13.
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

/// The various image formats we support.  RGBA is the default, and most expensive.
typedef enum {MaplyImageIntRGBA,MaplyImageUShort565,MaplyImageUShort4444,MaplyImageUShort5551,MaplyImageUByteRed,MaplyImageUByteGreen,MaplyImageUByteBlue,MaplyImageUByteAlpha,MaplyImageUByteRGB,MaplyImage4Layer8Bit} MaplyQuadImageFormat;

/// Wrap values for certain types of textures
#define MaplyImageWrapNone (0)
#define MaplyImageWrapX (1<<0)
#define MaplyImageWrapY (1<<1)

/** @brief The Maply Quad Image Tiles Layer is for paging image pyramids local or remote.
    @details This layer pages image pyramids.  They can be local or remote, in any coordinate system Maply supports and you provide a MaplyTileSource conformant object to do the actual image tile fetching.
    @details This is the main interface for image pyramid paging and so has a lot of knobs you can twiddle.  The defaults should work fine in most cases.
    @details You probably don't have to implement your own tile source.  Go look at the MaplyRemoteTileSource and MaplyMBTileSource objects, as well as MaplyMultiplexTileSource.  Those will do remote, local, and sources for animation respectively.  There's also MaplyWMSTileSource, but I wouldn't expect to use that.
    @see MaplyRemoteTileSource
    @see MaplyMBTileSource
    @see MaplyMultiplexTileSource
    @see MaplyWMSTileSource
  */
@interface MaplyQuadImageTilesLayer : MaplyViewControllerLayer

/** @brief Initialize with a coordinate system for the image pyramid and the tile source object.  
    @details The initialize expects a coordinate system (probably MaplySphericalMercator) and a tile source.  The tile source can be one of the standard ones listed above, or it can be one of your own that conforms to the MaplyTileSource protocol.
    @param coordSys The coordinate system. This must match what your
            image pyramid is in, or it will look weird.  Very weird.
    @param tileSource This is an object conforming to the MaplyTileSource protocol.  There are several you can pass in, or you can write your own.
  */
- (id)initWithCoordSystem:(MaplyCoordinateSystem *)coordSys tileSource:(NSObject<MaplyTileSource> *)tileSource;

/** @brief Enable/Disable the whole layer.
    @details By default this is on.  If you turn it off, there may be a slight delay before the whole layer disappears.  The layer will keep working, but any geometry will be invisible until you turn it back on.
  */
@property (nonatomic,assign) bool enable;

/** @brief The number of simultaneous fetches the layer will attempt at once.
    @details The toolkit loves its dispatch queues and threads.  By default this number is set to 8 or 16, but if you need to constrain it, you can set it lower (or higher!).  If your tile source can't handle multi-thread access, set this to 1.
  */
@property (nonatomic,assign) int numSimultaneousFetches;

/** @brief If set, we'll generate skirts between different levels of details on the globe.
    @details This option makes zero sense in 2D mode, some sense if 3D map mode (if you're doing elevation) and a lot of sense in globe mode.  What it does is generate skirts along the sides of each tile so you can't see between that tile and the next one when it abuts a different level of detail.
    @details Best to set this explicitly.  For a base map (e.g. the first image layer you put down), it should probably be set.  For following layers it should probably not, but specifics can change.  Try it out.
  */
@property (nonatomic,assign) bool handleEdges;

/** @brief Generate pole geometry for projections that don't go all the way to the poles.
    @details This is for spherical mercator with web extents.  That projection doesn't run all the way to the poles, but it gets fairly close.  If set, we close that gap for the north and south poles and generate the remaining geometry.
    @details We'll pull a texture value from the edge of the images, so build your texture accordingly.  A nice flat color along the north and south border is the best idea.  If not, it'll look a little odd, but still okay.
    @details Though this is designed for MaplySphericalMercator, it may work in similar projections.  It's not going to make any sense for, say UTM, but give it a try.
  */
@property (nonatomic,assign) bool coverPoles;

/** @brief Set the minimum viewer height the layer will be visible at.
    @details This is off by default.  When on the layer will not be visible unless the viewer is above this height.
  */
@property (nonatomic,assign) float minVis;

/** @brief Set the maximum viewer height the layer will be visible at.
    @details This is off by default.  When on the layer will not be visible unless the viewer is below this height.
  */
@property (nonatomic,assign) float maxVis;

/** @brief Controls whether the fetching code runs in a single thread or is spawned asyncronously.
    @details If set, we'll kick off the tile fetches in their own dispatched blocks.  If not set, we'll just do it in the layer thread.
  */
@property (nonatomic,assign) bool asyncFetching;

/** @brief Set the minimum time for an update based on the viewer's location and orientation.
    @details Paging layers watch the viewer to see what it's up to.  When the viewer moves, the layer updates its contents accordingly.  However, the viewer can be moving constantly so we need a way to keep things under control.
    @details This value (in seconds) specifies the minimum time between updates.  In other words, we won't recalculate things more often than this.  Default value is 1/10s.
  */
@property (nonatomic,assign) NSTimeInterval viewUpdatePeriod;

/** @brief Set the minimum movement for an update based on the viewer's location.
    @details This is useful for throttling layer updates based on how far a viewer moves.  This will only kick off a view update if the viewer moves the given distance (in display coordinates).
    @details We do not take orientation into account here, so you'd probably be better looking straight down.  Default is off.
    @details I suggest not using this unless you've already run into the problem this solves.  Specifically that's where you've moving constantly, but in small increments and are burning too much CPU.
    @see viewUpdatePeriod
  */
@property (nonatomic,assign) float minUpdateDist;

/** @brief Have the layer wait until all local tiles are loaded before updating the renderer.
    @details This will have the layer sit on updates until all the local tiles are in.  You won't see the lower levels loading in.  See waitLoadTimeout.
  */
@property (nonatomic,assign) bool waitLoad;

/** @brief The timeout for wait loads.  We can't wait longer than this for local updates to come back.
    @details If waitLoad is on, this is the maximum time we'll wait for local tile fetches to complete.  There's a limit to the volume of scene changes we can let build up int he queue before we have to flush them.
  */
@property (nonatomic,assign) NSTimeInterval waitLoadTimeout;

/** @brief The number of images we're expecting to get per tile.
    @details This is the number of images the layer will ask for per tile.  The default is 1, which is the normal case.  If this is greater than one that typically means we're going to animate between them.
    @details the MaplyTileSource delegate is always expected to provide this many imates.
  */
@property (nonatomic,assign) unsigned int imageDepth;

/** @brief Set the current image we're displaying.
    @details This sets the current image being displayed, and interpolates between it and the next image.  If set to an integer value, you'll get just that image.  If set to a value between integers, you'll get a blend of the two.
    @details This is incompatible with setting an animationPeriod.  Do just one or the other.
   */
@property (nonatomic, assign) float currentImage;

/** @brief If set, we'll use this as the maximum current image value when animating.
    @details By default this is off (-1).  When it's on, we'll consider this the last valid value for currentImage.  This means, when animating, we'll run from 0 to maxCurrentImage.
    @details This is helpful when you have an animation you want to taper off at the end past the last frame.
  */
@property (nonatomic, assign) float maxCurrentImage;

/** @brief The length of time we'll take to switch through all available images (per tile).
    @details If set to non-zero right after layer creation we'll run through all the available images (in each tile) over the given period.  This only makes sense if you've got more than one image per tile.
    @details If you want tighter control use the currentImage property and set your own timer.
  */
@property (nonatomic, assign) float animationPeriod;

/** @brief If set to true, we'll consider the list of images for each tile to be circular when we animate.
    @details When set we'll loop back to the first image when we go past the last.  This is the default.
    @details When not set, we'll run from 0 to maxCurrentImage and then restart.
  */
@property (nonatomic, assign) bool animationWrap;

/** @brief Include the original z values in the tile geometry for a custom shader.
    @details When generating tiles for the globe we project the coordinates from their local system (probably MaplySphericalMercator) into a display system.  If you wanted the original z values, to say, write a custom shader that maps color to elevation, that data is now missing.
    @details If set, this adds the z values back as a separate vertex attribute called "a_elev" for your shader to pick up.
  */
@property (nonatomic, assign) bool includeElevAttrForShader;

/** @brief If true we'll actuall use the elevation values to modify the mesh.
    @details When this is on we'll modify the mesh to actually use elevation values passed in through the elevation delegate. On is the default, logically enough.
    @details Why would you ever turn it off?  If you wanted a flat (or curved for the globe) mesh with elevation values in their seperate attribute array.  You might want a shading effect rather than actual geometry.
  */
@property (nonatomic, assign) bool useElevAsZ;

/** @brief Requires an elevation chunk for every tile we display.
    @details Elevation data is optional on the globe or map.  If it exists, via the MaplyElevationSource delegate on the view controller, then we'll use it to construct the tile.  This property requires elevation for any tile we display.
    @details What this does is prevent flat tiles from showing up if there's more imagery than elevation.
    @details We do make a distinction between missing tiles and tiles that are simply flat (at zero) in the MaplyElevationDatabase, so ocean will work correctly.
  */
@property (nonatomic, assign) bool requireElev;

/** @brief Color for the tile geometry.
    @details The geometry we create for tiles has an RGBA color.  It's white/full alpha by default, but you can set it here.  You might want to do this if you'd like a semi-transparent layer, sort of a shader of course, where you can do whatever you like.
  */
@property (nonatomic) UIColor *color;

/** @brief Maximum number of tiles to load in at once.
    @details This is the maximum number of tiles the pager will have loaded into memory at once.  The default is 128 and that's generally good enough.  However, if your tile size is small, you may want to load in more.
    @details Tile loading can get out of control when using elevation data.  The toolkit calculates potential sceen coverage for each tile so elevation data makes all tiles more important.  As a result the system will happily page in way more data than you may want.  The limit becomes important in elevation mode, so leave it at 128 unless you need to change it.
  */
@property (nonatomic) int maxTiles;

/** @brief Set the shader name to use for generated tiles.
    @details Shader programs are accessed by name.  When you create a shader and tie it into the scene, you'll have the name.  Use that name here to ensure that all tiles are rendered with that MaplyShader.
    @details Be sure to set this immediately after layer creation.  It can't be changed in the middle.
  */
@property (nonatomic) NSString *shaderProgramName;

/** @brief Set the (power of two) size of texture atlases the layer will create.
    @details The system makes extensive use of texture atlases for rendering tiles.  Typically we'll only have one or two gigantic textures will all our imagery and a handfull of drawables.  This is what makes the system fast.  Very fast.
    @details This option controls the size of those texture atlases.  It's set to 2048 by default (2048x2048 texels).  If you're going to change it, set it to 1024, but don't go any lower unless you know something we don't.  It must always be a power of 2.
  */
@property (nonatomic) unsigned int texturAtlasSize;

/** @brief Set the image format for the texture atlases (thus the imagery).
    @details OpenGL ES offers us several image formats that are more efficient than 32 bit RGBA, but they're not always appropriate.  This property lets you choose one of them.  The 16 or 8 bit ones can save a huge amount of space and will work well for some imagery, most maps, and a lot of weather overlays.
    @details Be sure to set this at layer creation, it won't do anything later on.
 
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

/** @brief Control how tiles are indexed, either from the lower left or the upper left.
    @details If set, we'll use the OSM approach (also Google Maps) to y indexing.  That's that default and it's normally what you're run into.
    @details Strictly speaking, TMS addressing (the standard) is flipped the other way.  So if you're tile source looks odd, try setting this to false.
  */
@property (nonatomic) bool flipY;

/** @brief Use the target zoom level shortcut when possible.
    @details This turns on the target zoom level shortcut as described in targetZoomLevel.  When on we'll calculate tile importance that way, that is based on a target zoom level rather than the more complex screen space calculations.
    @details It's on by default and will activate only when this layer's coordinate system is the same as the display system and there's no view matrix (e.g. tilt) set.
  */
@property (nonatomic) bool useTargetZoomLevel;

/** @brief Force a full reload of all tiles.
    @details This will notify the system to flush out all the existing tiles and start reloading from the top.  If everything is cached locally (and the MaplyTileSource objects say so) then this should appear instantly.  If something needs to be fetched or it's taking too long, you'll see these page in from the low to the high level.
    @details This is good for tile sources, like weather, that need to be refreshed every so often.
  */
- (void)reload;

/** @brief The target zoom level for this layer given the current view settings.
    @details Calculates the target zoom level for the middle of the screen.
    @details This only makes sense for flat maps that use the same coordinate system we're using in this tile source.  In addition, the viewer can't have a tilt or any non-2D transform in the view matrix.  If it does, this is meaningless, but it'll return a number anyway.
    @details If all those conditions are met then we can say we're only displaying a single zoom level and this is that.
  */
- (int)targetZoomLevel;

/** @brief Pass back the loaded image(s) for a given tile.
    @details If the tile source implements startFetchForTile: then we'll expect it to do the asynchronous loading.  When it's done loading an image, it calls this.
    @details When we're loading just one image per tile, call this with a UIImage or MaplyImageTile. If we're expecting multiple images (see: imageDepth) then pass in a MaplyImageTile that's been set up appropriately.
    @param images Either one of UIImage or MaplyPlaceholderImage.
    @param tileID The tile we've loaded.
  */
- (void)loadedImages:(id)images forTile:(MaplyTileID)tileID;

@end
