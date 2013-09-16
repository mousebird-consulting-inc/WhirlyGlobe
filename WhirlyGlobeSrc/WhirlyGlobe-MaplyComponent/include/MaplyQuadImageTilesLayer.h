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

/** This is a generic quad earth paging interface.  Hand it your coordinate system,
    bounds, and tile source object and it will page tiles for you.
    In general this is useful for feature data, such as vector features.  The image
    base maps have their own layers.
  */
@interface MaplyQuadImageTilesLayer : MaplyViewControllerLayer

/// Construct with the coordinate system and the tile source
- (id)initWithCoordSystem:(MaplyCoordinateSystem *)coordSys tileSource:(NSObject<MaplyTileSource> *)tileSource;

/// Change the number of fetches allowed at once
@property (nonatomic,assign) int numSimultaneousFetches;

/// Whether or not we use skirts
@property (nonatomic,assign) bool handleEdges;

/// Whether or not we try to cover the poles
@property (nonatomic,assign) bool coverPoles;

/// If true (by default) run the fetching code on an async thread.
/// Set this to 'no' if you've got a very quick turnaround for tiles
@property (nonatomic,assign) bool asyncFetching;

/// The number of images we're expecting per tile.  1 by default.
/// You must provide this number of images in the tileSource delegate.
@property (nonatomic,assign) unsigned int imageDepth;

/// Set the current image we're displaying.
/// Non integer values will get you a blend.  If you set this directly it
///  will disable animation.
@property (nonatomic, assign) float currentImage;

/// If set non-zero we'll switch through all the images over the given period.
@property (nonatomic, assign) float animationPeriod;

/// If set we'll include the original z values in the tile geometry.
/// You need this if you're writing a shader that uses them.
/// The shader attribute is called "a_elev"
@property (nonatomic, assign) bool includeElevAttrForShader;

/// If we've got elevation, we can require a valid elevation tile
///  before we put an image on it.  This ensures we match sparse
///  elevation data sets.
@property (nonatomic, assign) bool requireElev;

/// Color for the geometry we create to put the images on.
/// This is how you can get transparency in there (short of a shader)
@property (nonatomic) UIColor *color;

/// If set, we'll try to use this shader program for the tiles we create.
/// Set this immediately after creation
@property (nonatomic) NSString *shaderProgramName;

/// The size of texture atlases created to hold the textures.
/// This starts out at 2048 by default, it can only be a power of 2, ideally 1024 or 512 (at least)
@property (nonatomic) unsigned int texturAtlasSize;

/// How tile images are stored.  These are RGBA (32 bit) by default.
/// Other options are potentially less expensive, but lossy
@property (nonatomic) MaplyQuadImageFormat imageFormat;

/// Where we'll store cached images.  Leave this nil to
///  disable caching.
@property (nonatomic) NSString *cacheDir;

/// Force the layer to reload all data.  You do this when you change
///  change something about the tile source, usually contents.
- (void)reload;

@end
