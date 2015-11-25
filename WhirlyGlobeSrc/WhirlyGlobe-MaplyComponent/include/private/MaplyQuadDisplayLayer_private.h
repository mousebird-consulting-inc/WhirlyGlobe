/*
 *  MaplyQuadDisplayLayer_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/20/14.
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

#import <WhirlyGlobe.h>
#import "DataLayer_private.h"

/** The Quad Display Layer is an Obj-C wrapper around the C++ objects that set up and maintain
    an interactive quad tree display layer.
  */
@interface WhirlyKitQuadDisplayLayer : NSObject<WhirlyKitLayer>

/// Layer thread we're attached to
@property (nonatomic,weak,readonly) WhirlyKitLayerThread *layerThread;
/// The Quad Display Controller does all the work.  Set the settings with it, but right after creation
@property (nonatomic) WhirlyKit::QuadDisplayController *displayControl;

/// Construct with a renderer and data source for the tiles
- (id)initWithDataSource:(WhirlyKit::QuadDataStructure *)dataSource loader:(WhirlyKit::QuadLoader *)loader renderer:(WhirlyKit::SceneRendererES *)renderer;

/// Call this to force a reload for all existing tiles
- (void)refresh;

/// Call this to nudge the quad display layer awake.
- (void)wakeUp;

@end
