/*
 *  WhirlyGlobeSimpleViewController.m
 *  WhirlyGlobeSimple
 *
 *  Created by Steve Gifford on 6/1/11.
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

#import "WhirlyGlobeSimpleViewController.h"

using namespace WhirlyKit;

@interface WhirlyGlobeSimpleViewController()
@property (nonatomic,retain) WhirlyKitEAGLView *glView;
@property (nonatomic,retain) WhirlyKitSceneRendererES1 *sceneRenderer;
@property (nonatomic,retain) WhirlyGlobeView *theView;
@property (nonatomic,retain) WhirlyKitTextureGroup *texGroup;
@property (nonatomic,retain) WhirlyKitLayerThread *layerThread;
@property (nonatomic,retain) WhirlyGlobeSphericalEarthLayer *earthLayer;
@property (nonatomic,retain) WhirlyKitVectorLayer *vectorLayer;
@property (nonatomic,retain) WhirlyKitLabelLayer *labelLayer;
@property (nonatomic,retain) WhirlyKitParticleSystemLayer *particleSystemLayer;
@property (nonatomic,retain) WhirlyKitMarkerLayer *markerLayer;
@property (nonatomic,retain) WhirlyGlobePinchDelegate *pinchDelegate;
@property (nonatomic,retain) WhirlyGlobePanDelegate *panDelegate;
@property (nonatomic,retain) WhirlyGlobeTapDelegate *tapDelegate;
@property (nonatomic,retain) WhirlyGlobeLongPressDelegate *longPressDelegate;
@property (nonatomic,retain) WhirlyGlobeRotateDelegate *rotateDelegate;

- (void)addSomeVectors:(id)what;
@end


@implementation WhirlyGlobeSimpleViewController

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
@synthesize pinchDelegate;
@synthesize panDelegate;
@synthesize tapDelegate;
@synthesize longPressDelegate;
@synthesize rotateDelegate;

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
    
    self.pinchDelegate = nil;
    self.panDelegate = nil;
    self.tapDelegate = nil;
    self.longPressDelegate = nil;
    self.rotateDelegate = nil;
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
    self.title = @"Globe";
    [super viewDidLoad];

	// Set up an OpenGL ES view and renderer
	self.glView = [[[WhirlyKitEAGLView alloc] init] autorelease];
	self.sceneRenderer = [[[WhirlyKitSceneRendererES1 alloc] init] autorelease];
	glView.renderer = sceneRenderer;
	glView.frameInterval = 2;  // 60 fps
	[self.view addSubview:glView];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];

	// Create the textures and geometry, but in the right GL context
	[sceneRenderer useContext];
	
	// Set up a texture group for the world texture
	self.texGroup = [[[WhirlyKitTextureGroup alloc] initWithInfo:[[NSBundle mainBundle] pathForResource:@"lowres_wtb_info" ofType:@"plist"]] autorelease];
    
	// Need an empty scene and view
//	theScene = new WhirlyGlobe::GlobeScene(4*texGroup.numX,4*texGroup.numY);
	theScene = new WhirlyGlobe::GlobeScene(&geoCoordSystem,1);
	self.theView = [[[WhirlyGlobeView alloc] init] autorelease];
	
	// Need a layer thread to manage the layers
	self.layerThread = [[[WhirlyKitLayerThread alloc] initWithScene:theScene view:theView renderer:sceneRenderer] autorelease];
	
	// Earth layer on the bottom
	self.earthLayer = [[[WhirlyGlobeSphericalEarthLayer alloc] initWithTexGroup:texGroup cacheName:nil] autorelease];
	[self.layerThread addLayer:earthLayer];
    
	// Set up the vector layer where all our outlines will go
	self.vectorLayer = [[[WhirlyKitVectorLayer alloc] init] autorelease];
	[self.layerThread addLayer:vectorLayer];
    
	// General purpose label layer.
	self.labelLayer = [[[WhirlyKitLabelLayer alloc] init] autorelease];
	[self.layerThread addLayer:labelLayer];
    
    // Particle System layer
    self.particleSystemLayer = [[[WhirlyKitParticleSystemLayer alloc] init] autorelease];
    [self.layerThread addLayer:particleSystemLayer];
    
    // Marker layer
    self.markerLayer = [[[WhirlyKitMarkerLayer alloc] init] autorelease];
    [self.layerThread addLayer:markerLayer];
            
	// Give the renderer what it needs
	sceneRenderer.scene = theScene;
	sceneRenderer.theView = theView;
	
	// Wire up the gesture recognizers
	self.pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:glView globeView:theView];
	self.panDelegate = [WhirlyGlobePanDelegate panDelegateForView:glView globeView:theView];
	self.tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:theView];
    self.longPressDelegate = [WhirlyGlobeLongPressDelegate longPressDelegateForView:glView globeView:theView];
    self.rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:theView];
	
	// Kick off the layer thread
	// This will start loading things
	[self.layerThread start];
    
    // We'll add some random data
    [self performSelector:@selector(addSomeVectors:) withObject:nil afterDelay:0.1];
    [self performSelector:@selector(addParticleSystems:) withObject:nil afterDelay:0.1];
    [self performSelector:@selector(addMarkers:) withObject:nil afterDelay:0.1];
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

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

// Add some random vector data
- (void)addSomeVectors:(id)what
{
    // This describes how our labels will look
    NSDictionary *labelDesc = 
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithBool:YES],@"enable",
     [UIColor clearColor],@"backgroundColor",
     [UIColor whiteColor],@"textColor",
     [UIFont boldSystemFontOfSize:32.0],@"font",
     [NSNumber numberWithInt:4],@"drawOffset",
     [NSNumber numberWithFloat:0.05],@"height",
     [NSNumber numberWithFloat:0.0],@"width",
     nil];
    
    // Build up a list of individual labels
    NSMutableArray *labels = [[[NSMutableArray alloc] init] autorelease];

    WhirlyKitSingleLabel *sfLabel = [[[WhirlyKitSingleLabel alloc] init] autorelease];
    sfLabel.text = @"San Francisco";
    [sfLabel setLoc:GeoCoord::CoordFromDegrees(-122.283,37.7166)];
    [labels addObject:sfLabel];

    WhirlyKitSingleLabel *nyLabel = [[[WhirlyKitSingleLabel alloc] init] autorelease];
    nyLabel.text = @"New York";
    [nyLabel setLoc:GeoCoord::CoordFromDegrees(-74,40.716667)];
    [labels addObject:nyLabel];

    WhirlyKitSingleLabel *romeLabel = [[[WhirlyKitSingleLabel alloc] init] autorelease];
    romeLabel.text = @"Rome";
    [romeLabel setLoc:GeoCoord::CoordFromDegrees(12.5, 41.9)];
    [labels addObject:romeLabel];
    
    // Add all the labels at once
    [self.labelLayer addLabels:labels desc:labelDesc];    
}

// Add a particle system
- (void)addParticleSystems:(id)what
{
    NSDictionary *partDesc =
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithFloat:0.02],@"minLength",
     [NSNumber numberWithFloat:0.03],@"maxLength",
     [NSNumber numberWithInt:500],@"minNumPerSec",
     [NSNumber numberWithInt:600],@"maxNumPerSec",
     [NSNumber numberWithFloat:1.0],@"minLifetime",
     [NSNumber numberWithFloat:5.0],@"maxLifetime",
     nil];
    
    // Add a single particle system
    WhirlyKitParticleSystem *particleSystem = [[[WhirlyKitParticleSystem alloc] init] autorelease];
    GeoCoord washDc = GeoCoord::CoordFromDegrees(-77.036667,38.895111);
    [particleSystem setLoc:washDc];
    [particleSystem setNorm:GeoCoordSystem::LocalToGeocentricish(washDc)];
    
    [self.particleSystemLayer addParticleSystem:particleSystem desc:partDesc];
}

// Add a marker
- (void)addMarkers:(id)what
{
    NSDictionary *markerDesc =
    [NSDictionary dictionaryWithObjectsAndKeys:
     [UIColor redColor],@"color",
     nil];
    
    // Add a stamp at Paris, FR
    GeoCoord paris = GeoCoord::CoordFromDegrees(2.350833, 48.856667);
    WhirlyKitMarker *parisMarker = [[[WhirlyKitMarker alloc] init] autorelease];
    [parisMarker setLoc:paris];
    parisMarker.width = 0.01;    parisMarker.height = 0.01;
    [self.markerLayer addMarker:parisMarker desc:markerDesc];
}

@end
