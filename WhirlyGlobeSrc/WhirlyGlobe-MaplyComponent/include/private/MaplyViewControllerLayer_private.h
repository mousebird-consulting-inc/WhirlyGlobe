/*
 *  MaplyViewControllerLayer_private.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import <Foundation/Foundation.h>
#import <WhirlyGlobe.h>
#import "MaplyViewControllerLayer.h"
<<<<<<< HEAD
#import "LayerThread_private.h"
=======
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

@class MaplyBaseViewController;

/// Used to keep track of resources for a layer the user has asked to be created.
/// Don't mess with these directly.
@interface MaplyViewControllerLayer()

/// Layer thread this layer belongs to
@property (nonatomic,weak) WhirlyKitLayerThread *layerThread;

/// Subclasses fill this in.  It's called when the Component layer is added to the view controller.
<<<<<<< HEAD
- (bool)startLayer:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKit::SceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC;
=======
- (bool)startLayer:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene renderer:(WhirlyKitSceneRendererES *)renderer viewC:(MaplyBaseViewController *)viewC;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

/// Remove resources associated with this layer
- (void)cleanupLayers:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

@end
