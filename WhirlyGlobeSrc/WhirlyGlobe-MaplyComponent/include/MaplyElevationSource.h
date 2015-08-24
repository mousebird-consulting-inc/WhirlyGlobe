/*
 *  MaplyElevationSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/29/13.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MaplyComponentObject.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"

/** @brief The elevation chunk is a base class for representing elevation data in Maply.
    @details Elevation can be grids or triangles, or whatever.  They follow an internal (hidden)
    protocol for interpolation and conversion into visual data.
  */
@interface MaplyElevationChunk : NSObject

@end

/** @brief The Maply Elevation Chunk holds elevations for a single tile.
 @details This object holds elevation data for a single tile.
 Each tile overlaps the the eastern and northern neighbors by one
 cell.  Why?  Because we're generating triangles with these and
 they're independent.  Draw a picture, it'll make sense.
 @details Data values are currently onl 32 bit floating point.
 There may be a type parameter in the future.
 */
@interface MaplyElevationGridChunk : MaplyElevationChunk

/** @brief Initialize with the data (with floats) and size.
 @details This initializer takes an NSData object with the given number of
 samples.  Don't forget to add an extra one along the eastern and
 northern edges to match up with those tiles.  You still need to
 pass in as many samples as you're saying you are.  So if you say
 11 x 11, then there need to be 121 floats in the NSData object.
 @param data The NSData full of float (32 bit) samples.
 @param sizeX Number of samples in X.
 @param sizeY Number of samples in Y.
 */
- (nonnull instancetype)initWithGridData:(NSData *__nonnull)data sizeX:(unsigned int)sizeX sizeY:(unsigned int)sizeY;

@end

/** @brief Initialize the elevation data with a Cesium terrain tile.
    @details Processes Cesium's terrain tile format into a form we can use.
  */
@interface MaplyElevationCesiumChunk : MaplyElevationChunk

/** @brief Initialize with the Cesium elevation data and size.
 @details This initializer takes an NSData object with the format
	specified in: http://cesiumjs.org/data-and-assets/terrain/formats/quantized-mesh-1.0.html
 @param data The NSData with the specified format
 @param sizeX tile size in X.
 @param sizeY tile size in Y.
 */
- (nonnull instancetype)initWithCesiumData:(NSData *__nonnull)data sizeX:(unsigned int)sizeX sizeY:(unsigned int)sizeY;

/// @brief If set, this cales the elevation
@property (nonatomic) float scale;

@end

/** @brief Elevation Source Delegate provides elevation data
     for a given tile.
    @details The Elevation Source Delegate provides elevation data on demand for a given tile.  It returns a MaplyElevationChunk or nil if no data is available.  Your delegate may be called on a random thread, so act accordingly.
    @details This object is probably being used in conjuction with a MaplyTileSource, but is treated separately.  We may reuse the same elevation source for multiple image pyramids and they tend not to be packaged together.
  */
@protocol MaplyElevationSourceDelegate

/** @brief Return the coordinate system we're using.
    @details The coordinate system used by the elevation source delegate should match any associated MaplyTileSource objects.  The toolkit won't check that, so it's up to you.
    @details The coordinate system is probably MaplySphericalMercator.
  */
- (nullable MaplyCoordinateSystem *)getCoordSystem;

/** @brief The minimum zoom level this delegate can provide.
    @details The min zoom level should probably be 0 or at least match your MaplyTileSource objects.
  */
- (int)minZoom;

/** @brief The maximum zoom level this delegate can provide.
    @details The max zoom level is not a requirement.  The elevation pyramid can be sparse, just as the image pyramid can be.
    @details The MaplyQuadImageTilesLayer has an option to not display image tiles that don't have corresponding elevation.  In other words, the image display will cut off when there's no more elevation.
  */
- (int)maxZoom;

/** @brief Return the elevation samples for a given tile.
    @details Your delegate can return elevation data for the given tile or nil if no data was available.  Elevation is in meters and data values are 32 bit floats.
  */
- (nullable MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

/** @brief Return true if the data is local to the device.
    @details Let us know if the data is local or remote.  This is a hint to the pager and will be called before elevForTile:.  If you don't know, just return false.
  */
- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame;

@end

/** @brief A simple test elevation source.
    @details This just generates a bumpy elevation field to cover the whole earth in Spherical Mercator.  Don't be using this for anything real.
  */
@interface MaplyElevationSourceTester : NSObject<MaplyElevationSourceDelegate>

@end
