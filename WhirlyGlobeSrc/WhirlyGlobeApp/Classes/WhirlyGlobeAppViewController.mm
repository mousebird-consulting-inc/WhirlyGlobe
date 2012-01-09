/*
 *  WhirlyGlobeAppViewController.m
 *  WhirlyGlobeApp
 *
 *  Created by Stephen Gifford on 1/12/11.
 *  Copyright 2011 mousebird consulting
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
@property (nonatomic,retain) EAGLView *glView;
@property (nonatomic,retain) SceneRendererES1 *sceneRenderer;
@property (nonatomic,retain) UILabel *fpsLabel,*drawLabel;
@property (nonatomic,retain) WhirlyGlobePinchDelegate *pinchDelegate;
@property (nonatomic,retain) WhirlyGlobeSwipeDelegate *swipeDelegate;
@property (nonatomic,retain) WhirlyGlobeRotateDelegate *rotateDelegate;
@property (nonatomic,retain) PanDelegateFixed *panDelegate;
@property (nonatomic,retain) WhirlyGlobeTapDelegate *tapDelegate;
@property (nonatomic,retain) WhirlyGlobeLongPressDelegate *pressDelegate;
@property (nonatomic,retain) WhirlyGlobeView *theView;
@property (nonatomic,retain) TextureGroup *texGroup;
@property (nonatomic,retain) WhirlyGlobeLayerThread *layerThread;
@property (nonatomic,retain) SphericalEarthLayer *earthLayer;
@property (nonatomic,retain) VectorLayer *vectorLayer;
@property (nonatomic,retain) LabelLayer *labelLayer;
@property (nonatomic,retain) GridLayer *gridLayer;
@property (nonatomic,retain) InteractionLayer *interactLayer;

- (void)labelUpdate:(NSObject *)sender;
@end

@implementation WhirlyGlobeAppViewController

@synthesize glView;
@synthesize sceneRenderer;
@synthesize fpsLabel,drawLabel;
@synthesize pinchDelegate;
@synthesize swipeDelegate;
@synthesize rotateDelegate;
@synthesize panDelegate;
@synthesize tapDelegate;
@synthesize pressDelegate;
@synthesize theView;
@synthesize texGroup;
@synthesize layerThread;
@synthesize earthLayer;
@synthesize vectorLayer;
@synthesize labelLayer;
@synthesize gridLayer;
@synthesize interactLayer;

- (void)clear
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

    if (self.layerThread)
    {
        [self.layerThread cancel];
        while (!self.layerThread.isFinished)
            [NSThread sleepForTimeInterval:0.001];
    }

    self.glView = nil;
    self.sceneRenderer = nil;
    self.fpsLabel = nil;
    self.drawLabel = nil;
    self.pinchDelegate = nil;
    self.swipeDelegate = nil;
    self.rotateDelegate = nil;
    self.panDelegate = nil;
    self.tapDelegate = nil;
    self.pressDelegate = nil;
    
    if (theScene)
    {
        delete theScene;
        theScene = NULL;
    }
    self.theView = nil;
    self.texGroup = nil;
    
    self.layerThread = nil;
    self.earthLayer = nil;
    self.vectorLayer = nil;
    self.labelLayer = nil;
    self.gridLayer = nil;
    self.interactLayer = nil;
}

- (void)dealloc 
{
    [self clear];
    
    [super dealloc];
}

// Get the structures together for a 
- (void)viewDidLoad 
{
    [super viewDidLoad];
	
	// Set up an OpenGL ES view and renderer
	self.glView = [[[EAGLView alloc] init] autorelease];
	self.sceneRenderer = [[[SceneRendererES1 alloc] init] autorelease];
	glView.renderer = sceneRenderer;
	glView.frameInterval = 2;  // 60 fps
	[self.view addSubview:glView];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
	
	// Stick a FPS label in the upper left
	self.fpsLabel = [[[UILabel alloc] initWithFrame:CGRectMake(0,0,100,20)] autorelease];
	self.fpsLabel.backgroundColor = [UIColor clearColor];
	self.fpsLabel.textColor = [UIColor whiteColor];
    // Note: Turned off for now
//	[self.view addSubview:self.fpsLabel];
	[self labelUpdate:self];
	
	// And a drawable label right below that
	self.drawLabel = [[[UILabel alloc] initWithFrame:CGRectMake(0,20,100,20)] autorelease];
	self.drawLabel.backgroundColor = [UIColor clearColor];
	self.drawLabel.textColor = [UIColor whiteColor];
    // Note: Also turned off
//	[self.view addSubview:self.drawLabel];

	// Create the textures and geometry, but in the right GL context
	[sceneRenderer useContext];
	
	// Set up a texture group for the world texture
	self.texGroup = [[[TextureGroup alloc] initWithInfo:[[NSBundle mainBundle] pathForResource:@"big_wtb_info" ofType:@"plist"]] autorelease];

	// Need an empty scene and view
	theScene = new WhirlyGlobe::GlobeScene(4*texGroup.numX,4*texGroup.numY);
	self.theView = [[[WhirlyGlobeView alloc] init] autorelease];
	
	// Need a layer thread to manage the layers
	self.layerThread = [[[WhirlyGlobeLayerThread alloc] initWithScene:theScene] autorelease];
	
	// Earth layer on the bottom
    NSString *globeCache = @"GlobeCache";
    if (RenderCacheExists(globeCache))
    {
        self.earthLayer = [[[SphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:globeCache] autorelease];
    } else {
        self.earthLayer = [[[SphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:nil] autorelease];
        [self.earthLayer saveToCacheName:globeCache];
    }
	[self.layerThread addLayer:earthLayer];
    
    // Toss up an optional grid layer
    if (UseGridLayer)
    {
        self.gridLayer = [[[GridLayer alloc] initWithX:10 Y:5] autorelease];
        [self.layerThread addLayer:gridLayer];
    }

	// Set up the vector layer where all our outlines will go
	self.vectorLayer = [[[VectorLayer alloc] init] autorelease];
	[self.layerThread addLayer:vectorLayer];

	// General purpose label layer.
	self.labelLayer = [[[LabelLayer alloc] init] autorelease];
	[self.layerThread addLayer:labelLayer];

	// The interaction layer will handle label and geometry creation when something is tapped
    // Data is divided by countries, oceans, and regions (e.g. states/provinces)
	self.interactLayer = [[[InteractionLayer alloc] initWithVectorLayer:self.vectorLayer labelLayer:labelLayer globeView:self.theView
                                                           countryShape:[[NSBundle mainBundle] pathForResource:@"10m_admin_0_map_subunits" ofType:@"shp"]
                                                             oceanShape:[[NSBundle mainBundle] pathForResource:@"10m_geography_marine_polys" ofType:@"shp"]
                                                            regionShape:[[NSBundle mainBundle] pathForResource:@"10m_admin_1_states_provinces_shp" ofType:@"shp"]]
                          autorelease]; 
    self.interactLayer.maxEdgeLen = [self.earthLayer smallestTesselation]/10.0;
	[self.layerThread addLayer:interactLayer];
			
	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.view = theView;
	
	// Wire up the gesture recognizers
	self.pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:glView globeView:theView];
//	self.swipeDelegate = [WhirlyGlobeSwipeDelegate swipeDelegateForView:glView globeView:theView];
	self.panDelegate = [PanDelegateFixed panDelegateForView:glView globeView:theView];
	self.tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:theView];
    self.pressDelegate = [WhirlyGlobeLongPressDelegate longPressDelegateForView:glView globeView:theView];
    self.rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:theView];
	
	// Kick off the layer thread
	// This will start loading things
	[self.layerThread start];
}

- (void)viewDidUnload
{	
	[self clear];
	
	[super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
	[self.glView startAnimation];
	
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[self.glView stopAnimation];
	
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
	self.fpsLabel.text = [NSString stringWithFormat:@"%.2f fps",sceneRenderer.framesPerSec];
	self.drawLabel.text = [NSString stringWithFormat:@"%d draws",sceneRenderer.numDrawables];
	[self performSelector:@selector(labelUpdate:) withObject:nil afterDelay:FPSUpdateInterval];
}

@end
