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
#import "MaplyQuadEarthWithRemoteTiles_private.h"
#import "MaplyInteractionLayer_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@interface MaplyViewController () <MaplyInteractionLayerDelegate>
@end

@implementation MaplyViewController
{
    WhirlyKitEAGLView *glView;
    WhirlyKitSceneRendererES1 *sceneRenderer;
    
    // Coordinate system and display adapter
    SphericalMercatorDisplayAdapter *coordAdapter;

    // Custom map scene
    Maply::MapScene *mapScene;
    // Note: Look at the map view again
    MaplyView *mapView;
    
    WhirlyKitLayerThread *layerThread;
    
    // Standard set of reusable layers
    WhirlyKitMarkerLayer *markerLayer;
    WhirlyKitLabelLayer *labelLayer;
    WhirlyKitVectorLayer *vectorLayer;
    WhirlyKitShapeLayer *shapeLayer;
    WhirlyKitSelectionLayer *selectLayer;
    
    // Our own interaction layer for adding and removing things
    MaplyInteractionLayer *interactLayer;
    
    // Layers (and associated data) created for the user
    NSMutableArray *userLayers;

    // Gesture recognizers
    MaplyTapDelegate *tapDelegate;
    MaplyPanDelegate *panDelegate;
    MaplyPinchDelegate *pinchDelegate;
    
    // List of views we're tracking for location
    NSMutableArray *viewTrackers;
    
    // If set we'll look for selectables
    bool selection;
    
    // General rendering and other display hints
    NSDictionary *hints;
    
    // Default description dictionaries for the various data types
    NSDictionary *screenMarkerDesc,*markerDesc,*screenLabelDesc,*labelDesc,*vectorDesc,*shapeDesc;
    
    // Clear color we're using
    UIColor *theClearColor;
}

@synthesize delegate;
@synthesize selection;

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
    shapeLayer = nil;
    selectLayer = nil;
    
    interactLayer = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;
    
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;
    
    if (coordAdapter)
        delete coordAdapter;
    coordAdapter = NULL;
    
    viewTrackers = nil;
    
    theClearColor = nil;
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
    coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord(-180.0,-90.0), GeoCoord(180.0,90.0));
    mapView = [[MaplyView alloc] initWithCoordAdapater:coordAdapter];
    mapScene = new Maply::MapScene(mapView.coordAdapter);
    sceneRenderer.theView = mapView;
    
    // Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:mapScene view:mapView renderer:sceneRenderer];
    
    // Selection feedback
    selectLayer = [[WhirlyKitSelectionLayer alloc] initWithView:mapView renderer:sceneRenderer];
    [layerThread addLayer:selectLayer];
    
	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];
    
    // Set up the shape layer.  Manages a set of simple shapes
    shapeLayer = [[WhirlyKitShapeLayer alloc] init];
    shapeLayer.selectLayer = selectLayer;
    [layerThread addLayer:shapeLayer];
    
	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
    labelLayer.selectLayer = selectLayer;
	[layerThread addLayer:labelLayer];
    
    // Marker layer
    markerLayer = [[WhirlyKitMarkerLayer alloc] init];
    markerLayer.selectLayer = selectLayer;
    [layerThread addLayer:markerLayer];
    
    // Lastly, an interaction layer of our own
    interactLayer = [[MaplyInteractionLayer alloc] init];
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.labelLayer = labelLayer;
    interactLayer.markerLayer = markerLayer;
    // Note: Fix this when we use shapes in Maply
//    interactLayer.shapeLayer = shapeLayer;
    interactLayer.selectLayer = selectLayer;
    interactLayer.viewController = self;
    interactLayer.glView = glView;
    [layerThread addLayer:interactLayer];

 	// Give the renderer what it needs
	sceneRenderer.scene = mapScene;
	sceneRenderer.theView = mapView;
 
    // Wire up the gesture recognizers
    tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
    panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
    pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
    
    viewTrackers = [NSMutableArray array];

	// Kick off the layer thread
	// This will start loading things
	[layerThread start];
    
    // Set up defaults for the hints
    NSDictionary *newHints = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithBool:NO], kWGRenderHintZBuffer,
                              nil];
    [self setHints:newHints];
    
    // Set up default descriptions for the various data types
    NSDictionary *newScreenLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithFloat:1.0], kWGFade,
                                        nil];
    [self setScreenLabelDesc:newScreenLabelDesc];
    
    NSDictionary *newLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSNumber numberWithInteger:kWGLabelDrawOffsetDefault], kWGDrawOffset,
                                  [NSNumber numberWithInteger:kWGLabelDrawPriorityDefault], kWGDrawPriority,
                                  [NSNumber numberWithFloat:1.0], kWGFade,
                                  nil];
    [self setLabelDesc:newLabelDesc];
    
    NSDictionary *newScreenMarkerDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                         [NSNumber numberWithFloat:1.0], kWGFade,
                                         nil];
    [self setScreenMarkerDesc:newScreenMarkerDesc];
    
    NSDictionary *newMarkerDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithInteger:kWGMarkerDrawOffsetDefault], kWGDrawOffset,
                                   [NSNumber numberWithInteger:kWGMarkerDrawPriorityDefault], kWGDrawPriority,
                                   [NSNumber numberWithFloat:1.0], kWGFade,
                                   nil];
    [self setMarkerDesc:newMarkerDesc];
    
    NSDictionary *newVectorDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithInteger:kWGVectorDrawOffsetDefault], kWGDrawOffset,
                                   [NSNumber numberWithInteger:kWGVectorDrawPriorityDefault], kWGDrawPriority,
                                   [NSNumber numberWithFloat:1.0], kWGFade,
                                   nil];
    [self setVectorDesc:newVectorDesc];
    
    NSDictionary *newShapeDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSNumber numberWithFloat:1.0], kWGFade,
                                  nil];
    [self setShapeDesc:newShapeDesc];
    
    selection = true;
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
	
    [self registerForEvents];
    
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

// Register for interesting tap events and others
// Note: Fill this in
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name
{
    MaplyViewControllerLayer *newLayer = [[MaplyQuadEarthWithMBTiles alloc] initWithWithLayerThread:layerThread scene:mapScene renderer:sceneRenderer mbTiles:name handleEdges:NO];
    if (!newLayer)
        return nil;
    
    [userLayers addObject:newLayer];
    
    return newLayer;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSString *)baseURL imageExt:(NSString *)ext cache:(NSString *)cacheDir minZoom:(int)minZoom maxZoom:(int)maxZoom;
{
    MaplyQuadEarthWithRemoteTiles *newLayer = [[MaplyQuadEarthWithRemoteTiles alloc] initWithLayerThread:layerThread scene:mapScene renderer:sceneRenderer baseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom handleEdges:sceneRenderer.zBuffer];
    if (!newLayer)
        return nil;
    newLayer.cacheDir = cacheDir;
    [userLayers addObject:newLayer];
    
    return nil;
}

#pragma mark - Defaults and descriptions

// Merge the two dictionaries, add taking precidence, and then look for NSNulls
- (NSDictionary *)mergeAndCheck:(NSDictionary *)baseDict changeDict:(NSDictionary *)addDict
{
    if (!addDict)
        return baseDict;
    
    NSMutableDictionary *newDict = [NSMutableDictionary dictionaryWithDictionary:baseDict];
    [newDict addEntriesFromDictionary:addDict];
    
    // Now look for NSNulls, which we use to remove things
    NSArray *keys = [newDict allKeys];
    for (NSString *key in keys)
    {
        NSObject *obj = [newDict objectForKey:key];
        if (obj && [obj isKindOfClass:[NSNull class]])
            [newDict removeObjectForKey:key];
    }
    
    return newDict;
}

// Set new hints and update any related settings
- (void)setHints:(NSDictionary *)changeDict
{
    hints = [self mergeAndCheck:hints changeDict:changeDict];
    
    // Settings we store in the hints
    BOOL zBuffer = [hints boolForKey:kWGRenderHintZBuffer default:true];
    sceneRenderer.zBuffer = zBuffer;
    BOOL culling = [hints boolForKey:kWGRenderHintCulling default:true];
    sceneRenderer.doCulling = culling;
}

// Set the default description for screen markers
- (void)setScreenMarkerDesc:(NSDictionary *)desc
{
    screenMarkerDesc = [self mergeAndCheck:screenMarkerDesc changeDict:desc];
}

// Set the default description for markers
- (void)setMarkerDesc:(NSDictionary *)desc
{
    markerDesc = [self mergeAndCheck:markerDesc changeDict:desc];
}

// Set the default description for screen labels
- (void)setScreenLabelDesc:(NSDictionary *)desc
{
    screenLabelDesc = [self mergeAndCheck:screenLabelDesc changeDict:desc];
}

// Set the default description for labels
- (void)setLabelDesc:(NSDictionary *)desc
{
    labelDesc = [self mergeAndCheck:labelDesc changeDict:desc];
}

- (void)setVectorDesc:(NSDictionary *)desc
{
    vectorDesc = [self mergeAndCheck:vectorDesc changeDict:desc];
}

- (void)setShapeDesc:(NSDictionary *)desc
{
    shapeDesc = [self mergeAndCheck:shapeDesc changeDict:desc];
}

#pragma mark - Geometry related methods

/// Add a group of screen (2D) markers
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers
{
    return [interactLayer addScreenMarkers:markers desc:screenMarkerDesc];
}

/// Add a group of 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers
{
    return [interactLayer addMarkers:markers desc:markerDesc];
}

/// Add a group of screen (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels
{
    return [interactLayer addScreenLabels:labels desc:screenLabelDesc];
}

/// Add a group of 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels
{
    return [interactLayer addLabels:labels desc:labelDesc];
}

/// Add one or more vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors
{
    return [interactLayer addVectors:vectors desc:vectorDesc];
}

/// Add one or more shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes
{
    // Note: Fill this in when we add shapes to Maply
//    return [interactLayer addShapes:shapes desc:shapeDesc];
    return nil;
}

/// Add a view to track to a particular location
- (void)addViewTracker:(MaplyViewTracker *)viewTrack
{
    // Make sure we're not duplicating and add the object
    [self removeViewTrackForView:viewTrack.view];
    [viewTrackers addObject:viewTrack];
    
    // Hook it into the renderer
    ViewPlacementGenerator *vpGen = mapScene->getViewPlacementGenerator();
    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),viewTrack.view,DrawVisibleInvalid,DrawVisibleInvalid);
    
    // And add it to the view hierarchy
    if ([viewTrack.view superview] == nil)
        [glView addSubview:viewTrack.view];
}

/// Remove the view tracker associated with the given UIView
- (void)removeViewTrackForView:(UIView *)view
{
    // Look for the entry
    MaplyViewTracker *theTracker = nil;
    for (MaplyViewTracker *viewTrack in viewTrackers)
        if (viewTrack.view == view)
        {
            theTracker = viewTrack;
            break;
        }
    
    if (theTracker)
    {
        [viewTrackers removeObject:theTracker];
        ViewPlacementGenerator *vpGen = mapScene->getViewPlacementGenerator();
        vpGen->removeView(theTracker.view);
        if ([theTracker.view superview] == glView)
            [theTracker.view removeFromSuperview];
    }
}


/// Remove the data associated with an object the user added earlier
- (void)removeObject:(MaplyComponentObject *)theObj
{
    [interactLayer removeObject:theObj];
}

- (void)removeObjects:(NSArray *)theObjs
{
    [interactLayer removeObjects:theObjs];
}

#pragma mark - Interaction

// Internal animation handler
- (void)animateToPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // note: fill this in
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong
{
    // Note: fill this in
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    // Note: fill this in
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    // Note: fill this in
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    if (selectedObj && selection)
    {
        // The user selected something, so let the delegate know
        if (delegate && [delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
            [delegate maplyViewController:self didSelect:selectedObj];
    } else {
        // The user didn't select anything, let the delegate know.
        if (delegate && [delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
        {
            MaplyCoordinate coord;
            coord.x = msg.whereGeo.lon();
            coord.y = msg.whereGeo.lat();
            [delegate maplyViewController:self didTapAt:coord];
        }
        [self animateToPoint:msg.whereGeo time:1.0];
    }
}


- (void)tapOnMap:(NSNotification *)note
{
    MaplyTapMessage *msg = note.object;
    
    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [interactLayer userDidTap:msg];    
}

@end
