/*
 *  MayerLayerViewWatcher.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/14/12.
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
#import "GlobeView.h"
#import "LayerViewWatcher.h"
#import "MaplyView.h"

/** View State related to the map view.
  */
@interface MaplyViewState : WhirlyKitViewState

/// Initialize with the maply view and renderer
- (id)initWithView:(MaplyView *)mapView renderer:(WhirlyKitSceneRendererES *)renderer;

@end

/// The Map Layer View Watcher is a subclass of the layer view
///  that handles map specific parameters.
@interface MaplyLayerViewWatcher : WhirlyKitLayerViewWatcher
{
}

/// Initialize with the globe view to watch and the layer thread
- (id)initWithView:(MaplyView *)view thread:(WhirlyKitLayerThread *)inLayerThread;

@end
