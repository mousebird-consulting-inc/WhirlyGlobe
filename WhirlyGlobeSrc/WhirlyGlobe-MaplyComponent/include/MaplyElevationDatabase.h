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

/** The Maply Elevation Database object interfaces to the weird little
    elevation tile format we made up.
  */
@interface MaplyElevationDatabase : NSObject <MaplyElevationSourceDelegate>

/// Extents in local coordinate system
@property (nonatomic) double minX,minY,maxX,maxY;
/// Tile sizes (in cells, e.g pixels)
@property (nonatomic) unsigned int tileSizeX,tileSizeY;

/// Construct with a name or path
- (id)initWithName:(NSString *)name;

/// Minimum zoom level (e.g. 0)
- (int)minZoom;

/// Maximum zoom level (e.g. 17)
- (int)maxZoom;

/// Return the elevation data for the given tile (if it exists)
- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID;

@end
