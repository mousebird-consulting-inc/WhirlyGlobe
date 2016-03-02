/*
 *  MBTileQuadSource.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/23/12.
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

#import "QuadDisplayLayer.h"
#import "SphericalMercator.h"
#import "TileQuadLoader.h"

/** MabBox Tile Quad Data source.
    This implements the data source protocol for MapBox Tiles.
    Initialize with an archive, hand it a quad display layer and it'll
    page.
 */
@interface WhirlyKitMBTileQuadSource : NSObject<WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource>

/// The SQLite database we're looking at
@property (nonatomic,assign) sqlite3 *sqlDb;
/// Spherical Mercator coordinate system, for the tiles
@property (nonatomic,assign) WhirlyKit::SphericalMercatorCoordSystem *coordSys;
/// Bounds in Spherical Mercator
@property (nonatomic,assign) WhirlyKit::Mbr &mbr;
/// Bounds in geographic
@property (nonatomic,assign) WhirlyKit::GeoMbr &geoMbr;
/// Size of a tile in pixels square.  256 is the usual.
@property (nonatomic,assign) int pixelsPerTile;
/// If set, we call this delegate to get some elevation
@property (nonatomic,weak) NSObject<WhirlyKitElevationHelper> *elevDelegate;

/// Initialize the data source with the full path to the SQLite DB
- (id)initWithPath:(NSString *)path;

/// Minimum available zoom level.  Can be read from mb tiles db or assigned
@property (nonatomic,assign) int minZoom;

/// Maximum available zoom level.  Can be read from mb tiles db or assigned
@property (nonatomic,assign) int maxZoom;

/// Called by the layer to shut things down
- (void)shutdown;

@end

