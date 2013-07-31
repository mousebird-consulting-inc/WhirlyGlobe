/*
 *  MaplyQuadEarthWithMBTiles.h
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

/** The Quad Earth with MBTiles layer implements a layer that pages from a local MBTiles
    archive.  Create one of these and then add it to a view controller.
  */
@interface MaplyQuadEarthWithMBTiles : MaplyViewControllerLayer

/// Create with the name of the local MBTiles archive.
- (id)initWithMbTiles:(NSString *)mbTilesName;

/// Off by default, if set the layer will create skirts to handle edge gaps in tiles
@property (nonatomic,assign) bool handleEdges;

/// Override the minzoom
@property (nonatomic,assign) int minZoom;

/// Override the maxzoom
@property (nonatomic,assign) int maxZoom;

/// Whether or not we'll do the poles
@property (nonatomic,assign) bool coverPoles;

@end
