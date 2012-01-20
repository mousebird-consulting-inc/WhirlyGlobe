//
//  MapViewController.m
//  WhirlyMapWorld
//
//  Created by Steve Gifford on 1/9/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import "MapViewController.h"

@interface MapViewController()
@property (nonatomic,retain) WhirlyGlobeEAGLView *glView;
@property (nonatomic,retain) WhirlyGlobeSceneRendererES1 *sceneRenderer;
@property (nonatomic,retain) WhirlyMapView *theView;
@property (nonatomic,retain) WhirlyGlobeLayerThread *layerThread;
@property (nonatomic,retain) WhirlyGlobeVectorLayer *vectorLayer;
@property (nonatomic,retain) WhirlyGlobeLabelLayer *labelLayer;
@property (nonatomic,retain) WhirlyGlobeLoftLayer *loftLayer;
@property (nonatomic,retain) InteractionLayer *interactLayer;
@property (nonatomic,retain) WhirlyMapPinchDelegate *pinchDelegate;
@property (nonatomic,retain) WhirlyMapPanDelegate *panDelegate;
@end

@implementation MapViewController

@synthesize glView;
@synthesize sceneRenderer;
@synthesize theView;
@synthesize layerThread;
@synthesize vectorLayer;
@synthesize labelLayer;
@synthesize loftLayer;
@synthesize interactLayer;
@synthesize pinchDelegate;
@synthesize panDelegate;

+ (MapViewController *)loadFromNib
{
    MapViewController *viewC = [[[MapViewController alloc] initWithNibName:@"MapViewController" bundle:nil] autorelease];
    
    return viewC;
}

- (void)clear
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (self.layerThread)
    {
        [self.layerThread cancel];
        while (!self.layerThread.isFinished)
            [NSThread sleepForTimeInterval:0.001];
    }
    self.layerThread = nil;
    
    self.glView = nil;
    self.sceneRenderer = nil;
    
    if (coordSys)
        delete coordSys;
    coordSys = nil;

    if (theScene)
    {
        delete theScene;
        theScene = NULL;
    }
    self.theView = nil;

    self.vectorLayer = nil;
    self.labelLayer = nil;
    self.loftLayer = nil;
    
    self.interactLayer = nil;
    
    self.pinchDelegate = nil;
    self.panDelegate = nil;
}

- (void)dealloc
{
    [self clear];
    
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title = @"World Map";
    
	// Set up an OpenGL ES view and renderer
	self.glView = [[[WhirlyGlobeEAGLView alloc] init] autorelease];
	self.sceneRenderer = [[[WhirlyGlobeSceneRendererES1 alloc] init] autorelease];
	glView.renderer = sceneRenderer;
	glView.frameInterval = 2;  // 60 fps
    [self.view insertSubview:glView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
    
	// Create the textures and geometry, but in the right GL context
	[sceneRenderer useContext];
    
    // Need a coordinate system to describe the space we're working in
//    coordSys = new WhirlyMap::FlatCoordSystem();
    coordSys = new WhirlyMap::MercatorCoordSystem();

	// Need an empty scene and view
	theScene = new WhirlyGlobe::GlobeScene(6,3,coordSys);
	self.theView = [[[WhirlyMapView alloc] initWithCoordSystem:coordSys] autorelease];
	
	// Need a layer thread to manage the layers
	self.layerThread = [[[WhirlyGlobeLayerThread alloc] initWithScene:theScene] autorelease];

	// Set up the vector layer where all our outlines will go
	self.vectorLayer = [[[WhirlyGlobeVectorLayer alloc] init] autorelease];
	[self.layerThread addLayer:vectorLayer];
    
	// General purpose label layer.
	self.labelLayer = [[[WhirlyGlobeLabelLayer alloc] init] autorelease];
	[self.layerThread addLayer:labelLayer];
    
    // Loft layer, used for triangulated polygons
    self.loftLayer = [[[WhirlyGlobeLoftLayer alloc] init] autorelease];
    loftLayer.useCache = YES;
    [self.layerThread addLayer:loftLayer];
    
    // Our local interaction and data layer
    self.interactLayer = [[[InteractionLayer alloc] initWithMapView:self.theView] autorelease];
    [self.layerThread addLayer:interactLayer];
    interactLayer.labelLayer = labelLayer;
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.loftLayer = loftLayer;

	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.theView = theView;
    
    // Set up the gesture delegates
    self.pinchDelegate = [WhirlyMapPinchDelegate pinchDelegateForView:glView mapView:theView];
    self.panDelegate = [WhirlyMapPanDelegate panDelegateForView:glView mapView:theView];

	// Kick off the layer thread
	// This will start loading things
	[self.layerThread start];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

- (void)viewWillAppear:(BOOL)animated
{
	[self.glView startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    	[self.glView stopAnimation];
	
    // Turn off responses to taps
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
	[super viewWillDisappear:animated];
}

@end
