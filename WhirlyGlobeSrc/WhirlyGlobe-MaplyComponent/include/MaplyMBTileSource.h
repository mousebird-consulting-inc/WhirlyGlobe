/*
 *  MaplyMBTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
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

#import "MaplyTileSource.h"
#import "MaplyCoordinateSystem.h"

/** @brief MapBox Tiles format tile source object.
    @details This is the MBTiles format tile source.
    It'll read local MBTiles archives (https://github.com/mapbox/mbtiles-spec?source=c) which are basically just collections of image
    tiles in a sqlite database.
    @details The sqlite file must be local, that is contained
     within the bundle or copied into it at some point.  To
     access remote tile sources look at the MaplyRemoteTileSource.
    @details MBtiles archives tell us how big they are, including
     min and max zoom levels.  You can also limit the max zoom by setting
     it directly.
    @see MaplyRemoteTileSource
    @see MaplyQuadImageTilesLayer
  */
@interface MaplyMBTileSource : NSObject<MaplyTileSource>

/** @brief Initialize with the name or path to the MBTiles file.
    @details This initializer will look for the the MBTiles
     file as if it was given a full path. If that fails it will
     look for it in the bundle as if it was given just the first
     part of the name.  I'll tack on the sqlite extension.
    @return Returns a new MaplyMBTileSource object on success, nil on failure.
  */
- (id)initWithMBTiles:(NSString *)fileName;

/** @brief The maximum zoom level in the MBTiles archive.
    @details This is initially the max zoom level read out
     of the MBTiles archive.  However, you can override it
     to limit the zoom level, if that's what you need.
  */
@property (nonatomic) int maxZoom;

/** @brief The minimum zoom level in the MBTiles archive.
 @details This is initially the min zoom level read out
 of the MBTiles archive.  However, you can override it
 to limit the zoom level, if that's what you need.
 */
@property (nonatomic) int minZoom;

/** @brief The coordinate system for the MBTiles file.
    @details The coordinate system for an MBTiles file
     is always web mercator with the web extents.  This is
     known as web mercator and it makes cartographers cry.
  */
@property (nonatomic,readonly) MaplyCoordinateSystem *coordSys;

@end
