/*
 *  WhirlyGlobeAppViewController.m
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 1/12/11.
 *  Copyright 2011-2012 mousebird consulting
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

#import "WhirlyGlobeAppViewController.h"
#import "PanDelegateFixed.h"

using namespace WhirlyGlobe;

@interface WhirlyGlobeAppViewController()

- (void)labelUpdate:(NSObject *)sender;
@end

@implementation WhirlyGlobeAppViewController

- (void)clear
{
    [glView stopAnimation];

	[[NSNotificationCenter defaultCenter] removeObserver:self];

    if (layerThread)
    {
        [layerThread addThingToDelete:theScene];
        [layerThread addThingToRelease:layerThread];
        [layerThread cancel];
    }
    theScene = NULL;

    glView = nil;
    sceneRenderer = nil;
    fpsLabel = nil;
    drawLabel = nil;
    pinchDelegate = nil;
    swipeDelegate = nil;
    rotateDelegate = nil;
    panDelegate = nil;
    tapDelegate = nil;
    pressDelegate = nil;
    
    theView = nil;
    texGroup = nil;
    
    layerThread = nil;
    earthLayer = nil;
    vectorLayer = nil;
    labelLayer = nil;
    gridLayer = nil;
    interactLayer = nil;
}

- (void)dealloc 
{
    [self clear];
}

// Get the structures together for a 
- (void)viewDidLoad 
{
    [super viewDidLoad];
    
	// Set up an OpenGL ES view and renderer
	glView = [[WhirlyKitEAGLView alloc] init];
	sceneRenderer = [[WhirlyKitSceneRendererES1 alloc] init];
	glView.renderer = sceneRenderer;
	glView.frameInterval = 2;  // 60 fps
	[self.view addSubview:glView];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
	
	// Stick a FPS label in the upper left
//	fpsLabel = [[UILabel alloc] initWithFrame:CGRectMake(0,0,100,20)];
//	fpsLabel.backgroundColor = [UIColor clearColor];
//	fpsLabel.textColor = [UIColor whiteColor];
    // Note: Turned off for now
//	[self.view addSubview:self.fpsLabel];
//	[self labelUpdate:self];
	
	// And a drawable label right below that
//	drawLabel = [[UILabel alloc] initWithFrame:CGRectMake(0,20,100,20)];
//	drawLabel.backgroundColor = [UIColor clearColor];
//	drawLabel.textColor = [UIColor whiteColor];
    // Note: Also turned off
//	[self.view addSubview:self.drawLabel];

	// Create the textures and geometry, but in the right GL context
	[sceneRenderer useContext];
	
	// Set up a texture group for the world texture
	texGroup = [[WhirlyKitTextureGroup alloc] initWithInfo:[[NSBundle mainBundle] pathForResource:@"big_wtb_info" ofType:@"plist"]];

	// Need an empty scene and view
	theScene = new WhirlyGlobe::GlobeScene(1);
	theView = [[WhirlyGlobeView alloc] init];
	
	// Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:theScene view:theView renderer:sceneRenderer];
	
	// Earth layer on the bottom
    earthLayer = [[WhirlyGlobeSphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:nil];
	[layerThread addLayer:earthLayer];
    
    // Toss up an optional grid layer
    if (UseGridLayer)
    {
        gridLayer = [[WhirlyKitGridLayer alloc] initWithX:10 Y:5];
        [layerThread addLayer:gridLayer];
    }

	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];

	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
	[layerThread addLayer:labelLayer];

	// The interaction layer will handle label and geometry creation when something is tapped
    // Data is divided by countries, oceans, and regions (e.g. states/provinces)
	interactLayer = [[InteractionLayer alloc] initWithVectorLayer:vectorLayer labelLayer:labelLayer globeView:theView
                                                           countryShape:[[NSBundle mainBundle] pathForResource:@"10m_admin_0_map_subunits" ofType:@"shp"]
                                                             oceanShape:[[NSBundle mainBundle] pathForResource:@"10m_geography_marine_polys" ofType:@"shp"]
                                                            regionShape:[[NSBundle mainBundle] pathForResource:@"10m_admin_1_states_provinces_shp" ofType:@"shp"]]; 
    interactLayer.maxEdgeLen = [earthLayer smallestTesselation]/10.0;
	[layerThread addLayer:interactLayer];
			
	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.theView = theView;
	
	// Wire up the gesture recognizers
	pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:glView globeView:theView];
	panDelegate = [PanDelegateFixed panDelegateForView:glView globeView:theView];
	tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:theView];
    pressDelegate = [WhirlyGlobeLongPressDelegate longPressDelegateForView:glView globeView:theView];
    rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:theView];
	
	// Kick off the layer thread
	// This will start loading things
	[layerThread start];
}

- (void)viewDidUnload
{	
	[self clear];
	
	[super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
	[glView startAnimation];
	
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[glView stopAnimation];
	
	[super viewWillDisappear:animated];
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
//	return (interfaceOrientation == UIInterfaceOrientationPortrait);
    return true;
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning];
	
	// Note: Not clear what we can really do here
}

- (void)labelUpdate:(NSObject *)sender
{
	fpsLabel.text = [NSString stringWithFormat:@"%.2f fps",sceneRenderer.framesPerSec];
	drawLabel.text = [NSString stringWithFormat:@"%d draws",sceneRenderer.numDrawables];
	[self performSelector:@selector(labelUpdate:) withObject:nil afterDelay:FPSUpdateInterval];
}

@end
