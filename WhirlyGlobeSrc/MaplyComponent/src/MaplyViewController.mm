/*
 *  MaplyViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
 *  Copyright 2012 mousebird consulting
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
#import "MaplyViewController.h"
#import "MaplyQuadEarthWithMBTiles_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@interface MaplyViewController ()
@end

@implementation MaplyViewController
{
    WhirlyKitEAGLView *glView;
    WhirlyKitSceneRendererES1 *sceneRenderer;
    
    // Note: Need a custom map scene.
    Maply::MapScene *mapScene;
    // Note: Look at the map view again
    MaplyView *mapView;
    
    WhirlyKitLayerThread *layerThread;
    
    // Standard set of reusable layers
    WhirlyKitMarkerLayer *markerLayer;
    WhirlyKitLabelLayer *labelLayer;
    WhirlyKitVectorLayer *vectorLayer;
    WhirlyKitSelectionLayer *selectLayer;
    
    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;

    // Gesture recognizers
    MaplyPanDelegate *panDelegate;
    MaplyPinchDelegate *pinchDelegate;
}

// Tear down layers and layer thread
- (void) clear
{
    [glView stopAnimation];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (layerThread)
    {
        [layerThread addThingToDelete:mapScene];
        [layerThread addThingToRelease:layerThread];
        [layerThread cancel];
    }
    mapScene = NULL;
    
    glView = nil;
    sceneRenderer = nil;
    
    mapView = nil;
    layerThread = nil;
    
    markerLayer = nil;
    labelLayer = nil;
    vectorLayer = nil;
    selectLayer = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;
    
    panDelegate = nil;
    pinchDelegate = nil;
}

- (void)dealloc
{
    [self clear];
}

// Put together the objects we need to draw
- (void) loadSetup
{
    userLayers = [NSMutableArray array];
    
    // Set up OpenGL ES view and renderer
	glView = [[WhirlyKitEAGLView alloc] init];
	sceneRenderer = [[WhirlyKitSceneRendererES1 alloc] init];
	glView.renderer = sceneRenderer;
	glView.frameInterval = 2;  // 30 fps
    [self.view insertSubview:glView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
    
	// Create the textures and geometry, but in the right GL context
	[sceneRenderer useContext];
    
    // Turn on the model matrix optimization for drawing
    sceneRenderer.useViewChanged = true;
    
    // Need empty scene and view
    mapView = [[MaplyView alloc] init];
    mapScene = new Maply::MapScene(mapView.coordSystem);
    sceneRenderer.theView = mapView;
    
    // Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:mapScene view:mapView renderer:sceneRenderer];
    
    // Selection feedback
    selectLayer = [[WhirlyKitSelectionLayer alloc] initWithView:mapView renderer:sceneRenderer];
    [layerThread addLayer:selectLayer];
    
	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];
    
	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
    labelLayer.selectLayer = selectLayer;
	[layerThread addLayer:labelLayer];
    
    // Marker layer
    markerLayer = [[WhirlyKitMarkerLayer alloc] init];
    markerLayer.selectLayer = selectLayer;
    [layerThread addLayer:markerLayer];

 	// Give the renderer what it needs
	sceneRenderer.scene = mapScene;
	sceneRenderer.theView = mapView;
 
    // Wire up the gesture recognizers
    panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
    pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];

	// Kick off the layer thread
	// This will start loading things
	[layerThread start];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self loadSetup];
}

- (void)viewDidUnload
{
    [self clear];
    
    [super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
	[glView startAnimation];
	
//    [self registerForEvents];
    
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[glView stopAnimation];
    
	// Stop tracking notifications
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
	[super viewWillDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name
{
    MaplyViewControllerLayer *newLayer = [[MaplyQuadEarthWithMBTiles alloc] initWithWithLayerThread:layerThread scene:mapScene renderer:sceneRenderer mbTiles:name];
    if (!newLayer)
        return nil;
    
    [userLayers addObject:newLayer];
    
    return newLayer;
}


@end
