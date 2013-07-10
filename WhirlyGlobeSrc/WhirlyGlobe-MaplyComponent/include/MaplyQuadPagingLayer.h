/*
 *  MaplyQuadPagingLayer.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/20/13.
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
#import "MaplyViewControllerLayer.h"
#import "MaplyCoordinateSystem.h"
#import "MaplyTileSource.h"
#import "MaplyBaseViewController.h"

@class MaplyQuadPagingLayer;

/** The protocol for the Maply Paging Delegate.  Fill this in to
    provide per tile data for features that you're loading.
  */
@protocol MaplyPagingDelegate

/// Minimum zoom level (e.g. 0)
- (int)minZoom;

/// Maximum zoom level (e.g. 17)
- (int)maxZoom;

/** Start fetching data for the given tile.  This will not be called on the 
    main thread so be prepared for that.  You should immediatley to an
    async call to your own loading logic and then merge in your results.
    If you do your loading calls in line you'll slow down the loading thread.
    After you're done you MUST call tileDidLoad in the layer.
  */
- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer;

@end

/** This is a generic quad earth paging interface.  Hand it your coordinate system,
    bounds, and tile source object and it will page tiles for you.
 */
@interface MaplyQuadPagingLayer : MaplyViewControllerLayer

/// Change the number of fetches allowed at once
@property (nonatomic,assign) int numSimultaneousFetches;

/// The view controller this is part of
@property (nonatomic,readonly) MaplyBaseViewController *viewC;

/// Construct with the coordinate system (which contains bounds) and the tile source
- (id)initWithCoordSystem:(MaplyCoordinateSystem *)coordSys delegate:(NSObject<MaplyPagingDelegate> *)tileSource;

/// Add data for the given tileID.  These should be one or more MaplyComponentObject.
/// Please create them disabled so we enable & disable them as needed.
- (void)addData:(NSArray *)dataObjects forTile:(MaplyTileID)tileID;

/// Let the pager know the tile completely failed to load
- (void)tileFailedToLoad:(MaplyTileID)tileID;

/// Let the paging know the tile is completely loaded
- (void)tileDidLoad:(MaplyTileID)tileID;

/// Utility routine to calculate a bounding box given a particular tile
- (void)geoBoundsforTile:(MaplyTileID)tileID ll:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur ;

@end
