/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
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

#import "MaplyBaseViewController_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyBaseViewController

@synthesize selection;

- (void) clear
{
    [glView stopAnimation];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    if (layerThread)
    {
        [layerThread addThingToDelete:scene];
        [layerThread addThingToRelease:layerThread];
        [layerThread cancel];
    }
    
    scene = NULL;
    visualView = nil;

    glView = nil;
    sceneRenderer = nil;

    layerThread = nil;
    
    markerLayer = nil;
    labelLayer = nil;
    vectorLayer = nil;
    shapeLayer = nil;
    chunkLayer = nil;
    layoutLayer = nil;
    selectLayer = nil;
    
    interactLayer = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;

    viewTrackers = nil;
    
    theClearColor = nil;
}

- (WhirlyKitView *) loadSetup_view
{
    return nil;
}

- (WhirlyKit::Scene *) loadSetup_scene
{
    return NULL;
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    return nil;
}

// Create the Maply or Globe view.
// For specific parts we'll call our subclasses
- (void) loadSetup
{
    userLayers = [NSMutableArray array];
    
	// Set up an OpenGL ES view and renderer
	glView = [[WhirlyKitEAGLView alloc] init];
	sceneRenderer = [[WhirlyKitSceneRendererES1 alloc] init];
    //	sceneRenderer = [[WhirlyKitSceneRendererES2 alloc] init];
    theClearColor = [UIColor blackColor];
    [sceneRenderer setClearColor:theClearColor];
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
    
	// Need an empty scene and view
    visualView = [self loadSetup_view];
    scene = [self loadSetup_scene];
    
    // Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer];
    
    // Selection feedback
    selectLayer = [[WhirlyKitSelectionLayer alloc] initWithView:visualView renderer:sceneRenderer];
    [layerThread addLayer:selectLayer];
    
	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];
    
    // Set up the shape layer.  Manages a set of simple shapes
    shapeLayer = [[WhirlyKitShapeLayer alloc] init];
    shapeLayer.selectLayer = selectLayer;
    [layerThread addLayer:shapeLayer];
    
    // Set up the chunk layer.  Used for stickers.
    chunkLayer = [[WhirlyKitSphericalChunkLayer alloc] init];
    chunkLayer.ignoreEdgeMatching = true;
    [layerThread addLayer:chunkLayer];
    
	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
    labelLayer.selectLayer = selectLayer;
	[layerThread addLayer:labelLayer];
    
    // Marker layer
    markerLayer = [[WhirlyKitMarkerLayer alloc] init];
    markerLayer.selectLayer = selectLayer;
    [layerThread addLayer:markerLayer];
    
    // 2D layout engine layer
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer];
    labelLayer.layoutLayer = layoutLayer;
    [layerThread addLayer:layoutLayer];
    
    // Lastly, an interaction layer of our own
    interactLayer = [self loadSetup_interactionLayer];
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.labelLayer = labelLayer;
    interactLayer.markerLayer = markerLayer;
    interactLayer.shapeLayer = shapeLayer;
    interactLayer.chunkLayer = chunkLayer;
    interactLayer.selectLayer = selectLayer;
    interactLayer.glView = glView;
    [layerThread addLayer:interactLayer];
    
	// Give the renderer what it needs
	sceneRenderer.scene = scene;
	sceneRenderer.theView = visualView;
	    
    viewTrackers = [NSMutableArray array];
	
	// Kick off the layer thread
	// This will start loading things
	[layerThread start];
    
    // Set up defaults for the hints
    NSDictionary *newHints = [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithBool:YES], kWGRenderHintZBuffer,
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
    
    [self setStickerDesc:@{}];
    
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

- (void)startAnimation
{
    [glView startAnimation];
}

- (void)stopAnimation
{
    [glView stopAnimation];
}

- (void)viewWillAppear:(BOOL)animated
{
	[glView startAnimation];
	
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];

	[glView stopAnimation];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name
{
    MaplyViewControllerLayer *newLayer = (MaplyViewControllerLayer *)[[MaplyQuadEarthWithMBTiles alloc] initWithWithLayerThread:layerThread scene:scene renderer:sceneRenderer mbTiles:name handleEdges:(sceneRenderer.zBufferMode == zBufferOn)];
    if (!newLayer)
        return nil;
    
    [userLayers addObject:newLayer];
    
    return newLayer;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSString *)baseURL imageExt:(NSString *)ext cache:(NSString *)cacheDir minZoom:(int)minZoom maxZoom:(int)maxZoom;
{
    MaplyQuadEarthWithRemoteTiles *newLayer = [[MaplyQuadEarthWithRemoteTiles alloc] initWithLayerThread:layerThread scene:scene renderer:sceneRenderer baseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom handleEdges:(sceneRenderer.zBufferMode == zBufferOn)];
    if (!newLayer)
        return nil;
    newLayer.cacheDir = cacheDir;
    [userLayers addObject:newLayer];
    
    return newLayer;
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
    sceneRenderer.zBufferMode = (zBuffer ? zBufferOn : zBufferOff);
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

- (void)setStickerDesc:(NSDictionary *)desc
{
    stickerDesc = [self mergeAndCheck:stickerDesc changeDict:desc];
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

- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors
{
    return [interactLayer addSelectionVectors:vectors desc:vectorDesc];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc
{
    [interactLayer changeVectors:compObj desc:desc];
}

/// Add one or more shapes
- (MaplyComponentObject *)addShapes:(NSArray *)shapes
{
    return [interactLayer addShapes:shapes desc:shapeDesc];
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers
{
    return [interactLayer addStickers:stickers desc:stickerDesc];
}

/// Add a view to track to a particular location
- (void)addViewTracker:(WGViewTracker *)viewTrack
{
    // Make sure we're not duplicating and add the object
    [self removeViewTrackForView:viewTrack.view];
    [viewTrackers addObject:viewTrack];
    
    // Hook it into the renderer
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),viewTrack.view,DrawVisibleInvalid,DrawVisibleInvalid);
    
    // And add it to the view hierarchy
    if ([viewTrack.view superview] == nil)
        [glView addSubview:viewTrack.view];
}

/// Remove the view tracker associated with the given UIView
- (void)removeViewTrackForView:(UIView *)view
{
    // Look for the entry
    WGViewTracker *theTracker = nil;
    for (WGViewTracker *viewTrack in viewTrackers)
        if (viewTrack.view == view)
        {
            theTracker = viewTrack;
            break;
        }
    
    if (theTracker)
    {
        [viewTrackers removeObject:theTracker];
        ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
        vpGen->removeView(theTracker.view);
        if ([theTracker.view superview] == glView)
            [theTracker.view removeFromSuperview];
    }
}


/// Remove the data associated with an object the user added earlier
- (void)removeObject:(WGComponentObject *)theObj
{
    [interactLayer removeObject:theObj];
}

- (void)removeObjects:(NSArray *)theObjs
{
    [interactLayer removeObjects:theObjs];
}

- (void)removeLayer:(MaplyViewControllerLayer *)layer
{
    bool found = false;
    for (MaplyViewControllerLayer *theLayer in userLayers)
    {
        if (theLayer == layer)
        {
            found = true;
            break;
        }
    }
    if (!found)
        return;
    
    [layer cleanupLayers:layerThread scene:scene];
    [userLayers removeObject:layer];
}

- (void)removeAllLayers
{
    for (MaplyViewControllerLayer *theLayer in userLayers)
    {
        [theLayer cleanupLayers:layerThread scene:scene];
    }
    [userLayers removeAllObjects];
}

#pragma mark - Properties

- (UIColor *)clearColor
{
    return theClearColor;
}

- (void)setClearColor:(UIColor *)clearColor
{
    theClearColor = clearColor;
    [sceneRenderer setClearColor:clearColor];
}

- (MaplyCoordinate3d)displayPointFromGeo:(MaplyCoordinate)geoCoord
{
    MaplyCoordinate3d displayCoord;
    Point3f pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
    
    displayCoord.x = pt.x();    displayCoord.y = pt.y();    displayCoord.z = pt.z();
    return displayCoord;
}

@end
