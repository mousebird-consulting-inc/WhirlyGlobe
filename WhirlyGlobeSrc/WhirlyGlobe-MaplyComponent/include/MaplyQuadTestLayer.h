/*
 *  MaplyQuadTestLayer.h
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

/** The quad test layer dislays a quad tree of empty tiles with only color
    and the ID of each tile.  Very useful for debugging.
  */
@interface MaplyQuadTestLayer : MaplyViewControllerLayer

/// Construct with just the maximum zoom level
- (id)initWithMaxZoom:(int)maxZoom;

/// Set the depth (must be done before the layer is started)
@property unsigned int depth;

/// Set the current image we're displaying.  You can call this from any thread.
@property (nonatomic, assign) unsigned int currentImage;

/// If set non-zero we'll switch through all the images over the given period.
/// This must be called on the main thread
@property (nonatomic, assign) float period;

@end
