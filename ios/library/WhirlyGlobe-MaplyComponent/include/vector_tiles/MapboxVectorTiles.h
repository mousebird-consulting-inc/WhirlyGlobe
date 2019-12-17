/*
 *  MapboxVectorTiles.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/10/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "vector_styles/MaplyVectorTileStyle.h"
#import "loading/MaplyTileSourceNew.h"
#import "math/MaplyCoordinate.h"
#import "vector_styles/MaplyVectorStyle.h"

typedef NS_ENUM(NSInteger,MapboxGeometryType)
{
    GeomTypeUnknown = 0,
    GeomTypePoint = 1,
    GeomTypeLineString = 2,
    GeomTypePolygon = 3
};


/**
  Container for data parsed out of a Mapbox Vector Tile stream.
 
  This holds the parsed data as well as post-constructed data.  You will likely be handed one of these
    if you see it at all.  There are few cases where you might construct one.
  */
@interface MaplyVectorTileData : NSObject

/// Initialize with tile and bounds, both local coordinates and geographic
- (id)initWithID:(MaplyTileID)tileID bbox:(MaplyBoundingBoxD)bbox geoBBox:(MaplyBoundingBoxD)geoBBox;

/// Tile ID for the tile being built
@property (readonly) MaplyTileID tileID;

/// Bounding box in local coordinates
@property (readonly) MaplyBoundingBoxD bounds;

/// Bounding box in geographic
@property (readonly) MaplyBoundingBoxD geoBounds;

/// Add a single component object for tracking
- (void)addComponentObject:(MaplyComponentObject *)compObj;

/// When a style builds a component object, it needs to add it here
///  for tracking.  This lets us delete it later.
- (void)addComponentObjects:(NSArray *)compObjs;

/// Return all the component objects thus collected
- (NSArray *)componentObjects;

@end
