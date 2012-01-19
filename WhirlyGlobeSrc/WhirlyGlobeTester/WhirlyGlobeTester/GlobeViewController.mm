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
@property (nonatomic,retain) UIView *statsView;
@property (nonatomic,retain) UILabel *fpsLabel;
@property (nonatomic,retain) UILabel *drawLabel;
@property (nonatomic,retain) UILabel *selectLabel;
@property (nonatomic,retain) WhirlyGlobeEAGLView *glView;
@property (nonatomic,retain) WhirlyGlobeSceneRendererES1 *sceneRenderer;
@property (nonatomic,retain) WhirlyGlobeView *theView;
@property (nonatomic,retain) WhirlyGlobeTextureGroup *texGroup;
@property (nonatomic,retain) WhirlyGlobeLayerThread *layerThread;
@property (nonatomic,retain) WhirlyGlobeSphericalEarthLayer *earthLayer;
@property (nonatomic,retain) WhirlyGlobeVectorLayer *vectorLayer;
@property (nonatomic,retain) WhirlyGlobeLabelLayer *labelLayer;
@property (nonatomic,retain) WhirlyGlobeParticleSystemLayer *particleSystemLayer;
@property (nonatomic,retain) WhirlyGlobeMarkerLayer *markerLayer;
@property (nonatomic,retain) WhirlyGlobeSelectionLayer *selectionLayer;
@property (nonatomic,retain) WhirlyGlobeLoftLayer *loftLayer;
@property (nonatomic,retain) InteractionLayer *interactLayer;
@property (nonatomic,retain) WhirlyGlobePinchDelegate *pinchDelegate;
@property (nonatomic,retain) PanDelegateFixed *panDelegate;
@property (nonatomic,retain) WhirlyGlobeTapDelegate *tapDelegate;
@property (nonatomic,retain) WhirlyGlobeLongPressDelegate *longPressDelegate;
@property (nonatomic,retain) WhirlyGlobeRotateDelegate *rotateDelegate;
@property (nonatomic,retain) UIPopoverController *popoverController;
@property (nonatomic,retain) OptionsViewController *optionsViewC;

- (void)updateLabels:(id)sender;
- (void)registerForTaps;
@end


@implementation GlobeViewController

@synthesize statsView;
@synthesize fpsLabel;
@synthesize drawLabel;
@synthesize selectLabel;
@synthesize glView;
@synthesize sceneRenderer;
@synthesize theView;
@synthesize texGroup;
@synthesize layerThread;
@synthesize earthLayer;
@synthesize vectorLayer;
@synthesize labelLayer;
@synthesize particleSystemLayer;
@synthesize markerLayer;
@synthesize loftLayer;
@synthesize selectionLayer;
@synthesize interactLayer;
@synthesize pinchDelegate;
@synthesize panDelegate;
@synthesize tapDelegate;
@synthesize longPressDelegate;
@synthesize rotateDelegate;
@synthesize popoverController;
@synthesize optionsViewC;

+ (GlobeViewController *)loadFromNib
{
    GlobeViewController *viewC = [[[GlobeViewController alloc] initWithNibName:@"GlobeViewController" bundle:nil] autorelease];
    
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
 
    self.statsView = nil;
    self.fpsLabel = nil;
    self.drawLabel = nil;
    self.selectLabel = nil;
    self.glView = nil;
    self.sceneRenderer = nil;
    
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
    self.particleSystemLayer = nil;
    self.markerLayer = nil;
    self.loftLayer = nil;
    self.selectionLayer = nil;
    self.interactLayer = nil;
    
    self.pinchDelegate = nil;
    self.panDelegate = nil;
    self.tapDelegate = nil;
    self.longPressDelegate = nil;
    self.rotateDelegate = nil;
    
    self.popoverController = nil;
    self.optionsViewC = nil;
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

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title = @"Globe";
    
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
	
	// Set up a texture group for the world texture
	self.texGroup = [[[WhirlyGlobeTextureGroup alloc] initWithInfo:[[NSBundle mainBundle] pathForResource:@"big_wtb_info" ofType:@"plist"]] autorelease];
    
	// Need an empty scene and view    
	self.theView = [[[WhirlyGlobeView alloc] init] autorelease];
	theScene = new WhirlyGlobe::GlobeScene(4*texGroup.numX,4*texGroup.numY,theView.coordSystem);
    sceneRenderer.theView = theView;
	
	// Need a layer thread to manage the layers
	self.layerThread = [[[WhirlyGlobeLayerThread alloc] initWithScene:theScene] autorelease];
	
	// Earth layer on the bottom
	self.earthLayer = [[[WhirlyGlobeSphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:nil] autorelease];
    self.earthLayer.fade = 1.0;
	[self.layerThread addLayer:earthLayer];
    
    // Selection feedback
    self.selectionLayer = [[[WhirlyGlobeSelectionLayer alloc] initWithView:self.theView renderer:self.sceneRenderer] autorelease];
    [self.layerThread addLayer:selectionLayer];

	// Set up the vector layer where all our outlines will go
	self.vectorLayer = [[[WhirlyGlobeVectorLayer alloc] init] autorelease];
	[self.layerThread addLayer:vectorLayer];
    
	// General purpose label layer.
	self.labelLayer = [[[WhirlyGlobeLabelLayer alloc] init] autorelease];
    self.labelLayer.selectLayer = self.selectionLayer;
	[self.layerThread addLayer:labelLayer];
    
    // Particle System layer
    self.particleSystemLayer = [[[WhirlyGlobeParticleSystemLayer alloc] init] autorelease];
    [self.layerThread addLayer:particleSystemLayer];
    
    // Marker layer
    self.markerLayer = [[[WhirlyGlobeMarkerLayer alloc] init] autorelease];
    self.markerLayer.selectLayer = self.selectionLayer;
    [self.layerThread addLayer:markerLayer];
    
    // Lofted poly layer
    self.loftLayer = [[[WhirlyGlobeLoftLayer alloc] init] autorelease];
    [self.layerThread addLayer:loftLayer];
    
    // Lastly, an interaction layer of our own
    self.interactLayer = [[[InteractionLayer alloc] initWithGlobeView:theView] autorelease];
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.labelLayer = labelLayer;
    interactLayer.particleSystemLayer = particleSystemLayer;
    interactLayer.markerLayer = markerLayer;
    interactLayer.loftLayer = loftLayer;
    interactLayer.selectionLayer = selectionLayer;
    [self.layerThread addLayer:interactLayer];
        
	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.theView = theView;
	
	// Wire up the gesture recognizers
	self.pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:glView globeView:theView];
	self.panDelegate = [PanDelegateFixed panDelegateForView:glView globeView:theView];
	self.tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:theView];
    self.longPressDelegate = [WhirlyGlobeLongPressDelegate longPressDelegateForView:glView globeView:theView];
    self.rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:theView];
	
	// Kick off the layer thread
	// This will start loading things
	[self.layerThread start];
        
    statsView.hidden = YES;
    
    // Bring up an overlay if the user taps
    self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc] initWithTitle:@"Overlay" style:UIBarButtonItemStylePlain target:self action:@selector(pushOverlay:)] autorelease];
}

- (void)viewDidUnload
{	
	[self clear];
	
	[super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
	[self.glView startAnimation];
	
    [self registerForTaps];

	[super viewWillAppear:animated];

	[self updateLabels:self];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[self.glView stopAnimation];
	
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
    OverlayViewController *viewC = [[[OverlayViewController alloc] init] autorelease];
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
    theView.delegate = [[[AnimateViewRotation alloc] initWithView:theView rot:newRotQuat howLong:1.0] autorelease];    
}

// Called when the user taps outside the globe
- (void) tapOutsideSelector:(NSNotification *)note
{
    self.optionsViewC = [OptionsViewController loadFromNib];
    optionsViewC.delegate = self;
    self.popoverController = [[[UIPopoverController alloc] initWithContentViewController:optionsViewC] autorelease];
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
    self.selectLabel.text = what;
}

// Called every so often to update the labels
- (void)updateLabels:(id)sender
{
	self.fpsLabel.text = [NSString stringWithFormat:@"%.2f fps",sceneRenderer.framesPerSec];
	self.drawLabel.text = [NSString stringWithFormat:@"%d draws",sceneRenderer.numDrawables];
	[self performSelector:@selector(updateLabels:) withObject:nil afterDelay:FPSUpdateInterval];    
}

#pragma mark -
#pragma mark Popover Delegate

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
    self.popoverController = nil;
    self.optionsViewC = nil;
}

@end
