/*
 *  LayoutLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 12/4/12.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "LayoutManager.h"

/** The layout layer is a 2D text and marker layout engine.  You feed it objects
    you want it to draw and it will route them accordingly and control their
    visibility as new objects are added or old ones removed.
  */
@interface WhirlyKitLayoutLayer : NSObject<WhirlyKitLayer>

/// If set to a value greater than zero, that will the max number of objects displayed.
/// Zero by default, meaning all visible objects will be displayed (that can fit).
@property (nonatomic,assign) int maxDisplayObjects;

/// Initialize with the renderer (for screen size)
- (id)initWithRenderer:(WhirlyKitSceneRendererES *)renderer;

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a whole bunch of objects to track with the layout engine.
- (void)addLayoutObjects:(const std::vector<WhirlyKit::LayoutObject> &)layoutObjects;

/// Stop tracking a bunch of objects.  We assume they're removed elsewhere.
- (void)removeLayoutObjects:(const WhirlyKit::SimpleIDSet &)objectIDs;

@end
