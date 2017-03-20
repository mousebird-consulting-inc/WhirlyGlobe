/*
 *  GlobeViewController.mm
 *  WhirlyGlobeTester
 *
 *  Created by Steve Gifford on 10/26/11.
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

#import "GlobeViewController.h"
#import "OptionsViewController.h"
#import "OverlayViewController.h"

using namespace WhirlyGlobe;

@interface GlobeViewController()
{
    AnimateViewRotation *animateRotation;
}
- (void)updateLabels:(id)sender;
- (void)registerForTaps;
@end


@implementation GlobeViewController

+ (GlobeViewController *)loadFromNib
{
    GlobeViewController *viewC = [[GlobeViewController alloc] initWithNibName:@"GlobeViewController" bundle:nil];
    
    return viewC;
}

- (void)clear
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
        
    [layerThread addThingToDelete:theScene];
    [layerThread cancel];
    
    statsView = nil;
    fpsLabel = nil;
    drawLabel = nil;
    selectLabel = nil;
    
    glView = nil;
    sceneRenderer = nil;

    theScene = NULL;
    
    theView = nil;
    texGroup = nil;
    tileLoader = nil;
    mbTiles = nil;
    netTiles = nil;
    
    earthLayer = nil;
    quadLayer = nil;
    vectorLayer = nil;
    labelLayer = nil;
    particleSystemLayer = nil;
    markerLayer = nil;
    selectionLayer = nil;
    loftLayer = nil;
    interactLayer = nil;
    
    pinchDelegate = nil;
    panDelegate = nil;
    tapDelegate = nil;
    longPressDelegate = nil;
    rotateDelegate = nil;
    
    popoverController = nil;
    optionsViewC = nil;

    layerThread = nil;
}

- (void)dealloc
{
    [self clear];
    
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title = @"Globe";
    
	// Set up an OpenGL ES view and renderer
	glView = [[WhirlyKitEAGLView alloc] init];
	sceneRenderer = [[WhirlyKitSceneRendererES1 alloc] init];
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
    
    // Turn off the model matrix optimization for drawing.  We have too many animations.
    sceneRenderer.useViewChanged = false;
	
	// Set up a texture group for the world texture
    if (UseMBTiles)
        mbTiles = [[WhirlyKitMBTileQuadSource alloc] initWithPath:[[NSBundle mainBundle] pathForResource:@"blue-marble-topo-jan" ofType:@"mbtiles"]];
    
    // Set up a network tile set
    if (UseStamenTiles)
    {
        NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

//        netTiles = [[WhirlyGlobeNetworkTileQuadSource alloc] initWithBaseURL:@"http://a.tile.openstreetmap.org/" ext:@"png"];
//        netTiles.minZoom = 0;
//        netTiles.maxZoom = 14;
//        netTiles.numSimultaneous = 8;
//        netTiles.cacheDir = [NSString stringWithFormat:@"%@/osm_tiles/",cacheDir];
        netTiles = [[WhirlyKitNetworkTileQuadSource alloc] initWithBaseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/" ext:@"png"];
        netTiles.minZoom = 0;
        netTiles.maxZoom = 12;
        netTiles.numSimultaneous = 8;
        netTiles.cacheDir = [NSString stringWithFormat:@"%@/osm2_tiles/",cacheDir];        
//        netTiles = [[WhirlyGlobeNetworkTileQuadSource alloc] initWithBaseURL:@"http://tile.stamen.com/watercolor/" ext:@"jpg"];
//        netTiles.minZoom = 0;
//        netTiles.maxZoom = 10;
//        netTiles.numSimultaneous = 8;
//        netTiles.cacheDir = [NSString stringWithFormat:@"%@/stamen_tiles/",cacheDir];
        NSError *error = nil;
        [[NSFileManager defaultManager] createDirectoryAtPath:netTiles.cacheDir withIntermediateDirectories:YES attributes:nil error:&error];
    }

    // Load in a texture group if all else failes
    if (!mbTiles && !netTiles)
        texGroup = [[WhirlyKitTextureGroup alloc] initWithInfo:[[NSBundle mainBundle] pathForResource:@"big_wtb_info" ofType:@"plist"]];
    
	// Need an empty scene and view    
	theView = [[WhirlyGlobeView alloc] init];
    theScene = new WhirlyGlobe::GlobeScene(4);
    sceneRenderer.theView = theView;
	
	// Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:theScene view:theView renderer:sceneRenderer];
	
	// Earth layer on the bottom
    if (UseMBTiles || UseStamenTiles)
    {
        NSObject<WhirlyKitQuadDataStructure,WhirlyKitQuadTileImageDataSource> *dataSource = (UseMBTiles ? mbTiles : netTiles);
        tileLoader = [[WhirlyKitQuadTileLoader alloc] initWithDataSource:dataSource];
        quadLayer = [[WhirlyKitQuadDisplayLayer alloc] initWithDataSource:dataSource loader:tileLoader renderer:sceneRenderer];
//        quadLayer.lineMode = true;
//        quadLayer.debugMode = true;
        [layerThread addLayer:quadLayer];
    } else {
        earthLayer = [[WhirlyGlobeSphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:nil];
        earthLayer.fade = 1.0;
        [layerThread addLayer:earthLayer];
    }
    
    // Selection feedback
    selectionLayer = [[WhirlyKitSelectionLayer alloc] initWithView:theView renderer:sceneRenderer];
    [layerThread addLayer:selectionLayer];

	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];
    
	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
    labelLayer.selectLayer = selectionLayer;
	[layerThread addLayer:labelLayer];
    
    // Particle System layer
    particleSystemLayer = [[WhirlyKitParticleSystemLayer alloc] init];
    [layerThread addLayer:particleSystemLayer];
    
    // Marker layer
    markerLayer = [[WhirlyKitMarkerLayer alloc] init];
    markerLayer.selectLayer = selectionLayer;
    [layerThread addLayer:markerLayer];
    
    // Lofted poly layer
    loftLayer = [[WhirlyGlobeLoftLayer alloc] init];
    [layerThread addLayer:loftLayer];
    
    // Lastly, an interaction layer of our own
    interactLayer = [[InteractionLayer alloc] initWithGlobeView:theView];
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.labelLayer = labelLayer;
    interactLayer.particleSystemLayer = particleSystemLayer;
    interactLayer.markerLayer = markerLayer;
    interactLayer.loftLayer = loftLayer;
    interactLayer.selectionLayer = selectionLayer;
    [layerThread addLayer:interactLayer];
        
	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.theView = theView;
	
	// Wire up the gesture recognizers
	pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:glView globeView:theView];
	panDelegate = [PanDelegateFixed panDelegateForView:glView globeView:theView];
	tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:theView];
    longPressDelegate = [WhirlyGlobeLongPressDelegate longPressDelegateForView:glView globeView:theView];
    rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:theView];
	
	// Kick off the layer thread
	// This will start loading things
	[layerThread start];
        
    statsView.hidden = YES;
    
    // Bring up an overlay if the user taps
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Overlay" style:UIBarButtonItemStylePlain target:self action:@selector(pushOverlay:)];
}

- (void)viewDidUnload
{	
	[self clear];
	
	[super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
	[glView startAnimation];
	
    [self registerForTaps];

	[super viewWillAppear:animated];

	[self updateLabels:self];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[glView stopAnimation];
	
    // Turn off responses to taps
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];

	[super viewWillDisappear:animated];
}

// Register for interesting tap events
- (void)registerForTaps
{
    // If the user taps outside the globe, we'll bring up the options
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOutsideSelector:) name:WhirlyGlobeTapOutsideMsg object:nil];
    
    // If the user taps, the globe we'll rotate there
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnGlobe:) name:WhirlyGlobeTapMsg object:nil];
    
    // Keep track of setting changes
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(optionsChange:) name:kWGControlChange object:nil];
    
    // Update the display based on notification
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectionChange:) name:kWGSelectionNotification object:nil];    
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

// Called when the user taps the overlay button
- (void) pushOverlay:(id)sender
{
    OverlayViewController *viewC = [[OverlayViewController alloc] init];
    [self.navigationController pushViewController:viewC animated:YES];
}

// Called when the user taps on the globe.  We'll rotate to that position
- (void) tapOnGlobe:(NSNotification *)note
{
    WhirlyGlobeTapMessage *msg = note.object;
    
    // If we were rotating from one point to another, stop
    [theView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaternionf newRotQuat = [theView makeRotationToGeoCoord:msg.whereGeo keepNorthUp:YES];
    
    // Rotate to the given position over 1s
    animateRotation = [[AnimateViewRotation alloc] initWithView:theView rot:newRotQuat howLong:1.0];
    theView.delegate = animateRotation;    
}

// Called when the user taps outside the globe
- (void) tapOutsideSelector:(NSNotification *)note
{
    optionsViewC = [OptionsViewController loadFromNib];
    optionsViewC.delegate = self;
    popoverController = [[UIPopoverController alloc] initWithContentViewController:optionsViewC];
    popoverController.popoverContentSize = CGSizeMake(400, 600);
    popoverController.delegate = self;
    [popoverController presentPopoverFromRect:CGRectMake(0, 0, 10, 10) inView:self.view permittedArrowDirections: UIPopoverArrowDirectionAny animated:YES];
}

// Called when the options change
- (void)optionsChange:(NSNotification *)note
{
    NSDictionary *options = note.object;
    bool statsOn = [[options objectForKey:kWGStatsControl] boolValue];
    if (statsOn)
    {
        statsView.hidden = NO;
    } else {
        statsView.hidden = YES;
    }
}

// User tapped something
- (void)selectionChange:(NSNotification *)note
{
    NSString *what = note.object;
    selectLabel.text = what;
}

// Called every so often to update the labels
- (void)updateLabels:(id)sender
{
	fpsLabel.text = [NSString stringWithFormat:@"%.2f fps",sceneRenderer.framesPerSec];
	drawLabel.text = [NSString stringWithFormat:@"%d draws",sceneRenderer.numDrawables];
	[self performSelector:@selector(updateLabels:) withObject:nil afterDelay:FPSUpdateInterval];    
}

#pragma mark -
#pragma mark Popover Delegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)inPopoverController
{
    popoverController = nil;
    optionsViewC = nil;
}

@end
