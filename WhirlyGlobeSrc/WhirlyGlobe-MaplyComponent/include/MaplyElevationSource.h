/*
 *  MaplyElevationSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/29/13.
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

#import "MaplyComponentObject.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"

/** The Maply Elevation Chunk encapsulates a grid of elevation data
    at a particular tile.  It overlaps by one cell alone the east and north edges.
  */
@interface MaplyElevationChunk : NSObject

/// Initialize with a buffer full of floats and the extents.
/// Don't forget the overlap alone east and north edges
- (id)initWithData:(NSData *)data numX:(unsigned int)numX numY:(unsigned int)numY;

/// Number of cells in each direction
@property unsigned int numX,numY;

/// Data buffer.  Elevations are floats in meters
@property NSData *data;

@end

/** The Elevation Source Delegate provides elevation data on demand for
    a given tile.  It returns a MaplyElevationChunk or nil if no data
    is available.  Your delegate may be called on a random thread, act accordingly.
  */
@protocol MaplyElevationSourceDelegate

/// Coordinate system we're providing the data in (and extents)
- (MaplyCoordinateSystem *)getCoordSystem;

/// Minimum zoom level (e.g. 0)
- (int)minZoom;

/// Maximum zoom level (e.g. 17)
- (int)maxZoom;

/// Return an elevation chunk (or nil) for a given tile
- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

@end

/** A simple test elevation source.  This just generates a bumpy elevation field.
    to cover the whole earth in Spherical Mercator.
  */
@interface MaplyElevationSourceTester : NSObject<MaplyElevationSourceDelegate>

@end
