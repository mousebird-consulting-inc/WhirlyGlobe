/*
 *  MaplyTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/7/13.
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

/// Simple tile IDentifier
typedef struct
{
    int x, y, level;
} MaplyTileID;

/** The protocol for a Maply Tile Source.  Fill these calls in and your can
    pass in your own data tile by tile.
    The tile source should know its coordinate system, which is handed to
    Maply separately.
  */
@protocol MaplyTileSource

/// Minimum zoom level (e.g. 0)
- (int)minZoom;

/// Maximum zoom level (e.g. 17)
- (int)maxZoom;

/// Number of pixels on the side of a single tile (e.g. 128, 256)
- (int)tileSize;

/// Return the image for a given tile
- (NSData *)imageForTile:(MaplyTileID)tileID;

@end
