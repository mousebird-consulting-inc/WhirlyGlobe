/*
 *  MaplyElevationDatabase.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/7/13.
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

#import <UIKit/UIKit.h>
#import "MaplyElevationSource.h"

/** @brief An elevation source corresponding to a custom sqlite database.
    @details The Maply Elevation Database object interfaces to a custom sqlite
    database that contains elevation tiles.  This is the output of the
    elev_tile_pyramid command line tool.  See that for details.
    Suffice it to say that each tile is separate, contains one extra
    cell on the northern and eastern sides and is made up for shorts (16 bit).
    @see MaplyElevationSourceDelegate
  */
@interface MaplyElevationDatabase : NSObject <MaplyElevationSourceDelegate>

/// @brief Left side of the extents in the local coordinate system
@property (nonatomic) double minX;
/// @brief Bottom side of the extents in the local coordinate system
@property (nonatomic) double minY;
/// @brief Right side of the extents in the local coordinate system
@property (nonatomic) double maxX;
/// @brief Top side of the extents in the local coordinate system
@property (nonatomic) double maxY;
/// @brief Number of samples in the X direction for each tile
@property (nonatomic) unsigned int tileSizeX;
/// @brief Number of samples in the Y direction for each tile
@property (nonatomic) unsigned int tileSizeY;

/** @brief Initialize with the full path or name of the sqlite file.
    @details It looks for the full path first and then will try to find the name in the bundle with an extension of @"sqlite" if that failes.
    @return Returns a valid elevation database object or nil on failure.
  */
- (id)initWithName:(NSString *)name;

/// @brief Minimum zoom level (e.g. 0) as read from the file
- (int)minZoom;

/// @brief Maximum zoom level (e.g. 17) as read from the file
- (int)maxZoom;

/** @brief Returns elevaton data for the given tile.
    @details This will return elevation samples for the given tile in meters.  The size of the tile is determined by the file (and static).  Data will come from the file in 16 bit signed values, but then converted into 32 bit floating point values.
    @return Returns the elevation chunk corresonding to the given tile.
  */
- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

@end
