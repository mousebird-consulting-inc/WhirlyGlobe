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

#import "MaplyBaseViewController.h"
#import "MaplyBaseViewController_private.h"
#import "NSData+Zlib.h"
<<<<<<< HEAD
#import "UIColor+Stuff.h"
#import "NSDictionary+Stuff.h"
=======
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
#import "MaplyTexture_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyBaseViewController
{
}

- (void) clear
{
    if (!scene)
        return;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];

    [glView stopAnimation];
    
    EAGLContext *oldContext = [EAGLContext currentContext];
<<<<<<< HEAD
    sceneRenderer->useContext();
=======
    [sceneRenderer useContext];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    for (MaplyShader *shader in shaders)
        [shader shutdown];
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
<<<<<<< HEAD
=======
    sceneRenderer.scene = nil;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    if (baseLayerThread)
    {
        // Kill off all the other layers first
        for (unsigned int ii=1;ii<[layerThreads count];ii++)
        {
            WhirlyKitLayerThread *layerThread = [layerThreads objectAtIndex:ii];
            [baseLayerThread addThreadToShutdown:layerThread];
        }

        [baseLayerThread addThingToDelete:scene];
<<<<<<< HEAD
        [baseLayerThread addThingToDelete:visualView];
        [baseLayerThread addThingToDelete:sceneRenderer];
        [baseLayerThread addThingToRelease:baseLayerThread];
        [baseLayerThread addThingToRelease:glView];
=======
        [baseLayerThread addThingToRelease:baseLayerThread];
        [baseLayerThread addThingToRelease:visualView];
        [baseLayerThread addThingToRelease:glView];
        [baseLayerThread addThingToRelease:sceneRenderer];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        [baseLayerThread cancel];
    }
    layerThreads = nil;
    
    scene = NULL;
    visualView = nil;

    glView = nil;
    sceneRenderer = nil;
    baseLayerThread = nil;
<<<<<<< HEAD
    // Note: Porting
//    layoutLayer = nil;
=======
    layoutLayer = nil;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    activeObjects = nil;
    
    interactLayer = nil;
    
<<<<<<< HEAD
    // Note: Porting
//    while ([userLayers count] > 0)
//    {
//        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
//        [userLayers removeObject:layer];
//    }
=======
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    userLayers = nil;

    viewTrackers = nil;
    
    theClearColor = nil;
}

- (void) dealloc
{
    if (scene)
        [self clear];
}

<<<<<<< HEAD
- (WhirlyKit::View *) loadSetup_view
=======
- (WhirlyKitView *) loadSetup_view
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
{
    return nil;
}

- (void)loadSetup_glView
{
    if (_frameInterval <= 0)
        _frameInterval = 1;
    glView = [[WhirlyKitEAGLView alloc] init];
    glView.frameInterval = _frameInterval;
}

- (WhirlyKit::Scene *) loadSetup_scene
{
    return NULL;
}

- (void) loadSetup_lighting
{
    NSString *lightingType = hints[kWGRendererLightingMode];
    int lightingRegular = true;
    if ([lightingType respondsToSelector:@selector(compare:)])
        lightingRegular = [lightingType compare:@"none"];
    
    // Regular lighting is on by default
    if (!lightingRegular)
    {
        SimpleIdentity triNoLighting = scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
        if (triNoLighting != EmptyIdentity)
            scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
    } else {
<<<<<<< HEAD
        // Note: Porting
//        // Add a default light
//        MaplyLight *light = [[MaplyLight alloc] init];
//        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
//        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
//        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
//        light.viewDependent = false;
//        [self addLight:light];
=======
        // Add a default light
        MaplyLight *light = [[MaplyLight alloc] init];
        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        light.viewDependent = false;
        [self addLight:light];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    }
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    return nil;
}

// Create the Maply or Globe view.
// For specific parts we'll call our subclasses
- (void) loadSetup
{
    // Need this logic here to pull in the categories
    static bool dummyInit = false;
    if (!dummyInit)
    {
        NSDataDummyFunc();
        dummyInit = true;
    }
    
    userLayers = [NSMutableArray array];
    _threadPerLayer = true;
    
    [self loadSetup_glView];

	// Set up the OpenGL ES renderer
<<<<<<< HEAD
    sceneRenderer = new MaplySceneRendererES2();
=======
    sceneRenderer = new SceneRendererES2();
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    sceneRenderer->setZBufferMode(zBufferOffDefault);
    // Switch to that context for any assets we create
    // Note: Should be switching back at the end
    sceneRenderer->useContext();

    theClearColor = [UIColor blackColor];
<<<<<<< HEAD
    sceneRenderer->setClearColor([theClearColor asRGBAColor]);
=======
    sceneRenderer->setClearColor(theClearColor);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    // Set up the GL View to display it in
	glView = [[WhirlyKitEAGLView alloc] init];
	glView.renderer = sceneRenderer;
	glView.frameInterval = _frameInterval;
    [self.view insertSubview:glView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
    
    // Turn on the model matrix optimization for drawing
<<<<<<< HEAD
    sceneRenderer->setUseViewChanged(true);
=======
    sceneRenderer.useViewChanged = true;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
	// Need an empty scene and view
    visualView = [self loadSetup_view];
    scene = [self loadSetup_scene];
<<<<<<< HEAD
    sceneRenderer->setScene(scene);
=======
    sceneRenderer.scene = scene;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    [self loadSetup_lighting];
    
    layerThreads = [NSMutableArray array];
    
    // Need a layer thread to manage the layers
	baseLayerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer mainLayerThread:true];
    [layerThreads addObject:baseLayerThread];
    
<<<<<<< HEAD
    // Note: Porting
    // Layout still needs a layer to kick it off
//    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer];
//    [baseLayerThread addLayer:layoutLayer];
=======
    // Layout still needs a layer to kick it off
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer];
    [baseLayerThread addLayer:layoutLayer];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    // Lastly, an interaction layer of our own
    interactLayer = [self loadSetup_interactionLayer];
    interactLayer.glView = glView;
    [baseLayerThread addLayer:interactLayer];
    
	// Give the renderer what it needs
<<<<<<< HEAD
	sceneRenderer->setView(visualView);
=======
	sceneRenderer.theView = visualView;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
	    
    viewTrackers = [NSMutableArray array];
	
	// Kick off the layer thread
	// This will start loading things
	[baseLayerThread start];
    
    // Set up defaults for the hints
    NSDictionary *newHints = [NSDictionary dictionaryWithObjectsAndKeys:
                              nil];
    [self setHints:newHints];
        
    _selection = true;
}

- (void) useGLContext
{
<<<<<<< HEAD
    sceneRenderer->useContext();
=======
    [sceneRenderer useContext];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
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
	[self startAnimation];
	
	[super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];

	[self stopAnimation];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    // We may retain a bit of memory here.  Clear it up.
    scene->getMemManager()->clearBufferIDs();
    scene->getMemManager()->clearTextureIDs();
}

- (void)setFrameInterval:(int)frameInterval
{
    _frameInterval = frameInterval;
    glView.frameInterval = frameInterval;
}

static const float PerfOutputDelay = 15.0;

- (void)setPerformanceOutput:(bool)performanceOutput
{
    if (_performanceOutput == performanceOutput)
        return;
    
    _performanceOutput = performanceOutput;
    if (_performanceOutput)
    {
<<<<<<< HEAD
        sceneRenderer->setPerfInterval(100);
        [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];
    } else {
        sceneRenderer->setPerfInterval(0);
=======
        sceneRenderer.perfInterval = 100;
        [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];
    } else {
        sceneRenderer.perfInterval = 0;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];
    }
}

// Run every so often to dump out stats
- (void)periodicPerfOutput
{
    if (!scene)
        return;
    
    scene->dumpStats();
    
    [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];    
}

- (bool)performanceOutput
{
    return _performanceOutput;
}

<<<<<<< HEAD
// Note: Porting
//// Build an array of lights and send them down all at once
//- (void)updateLights
//{
//    NSMutableArray *theLights = [NSMutableArray array];
//    for (MaplyLight *light in lights)
//    {
//        WhirlyKitDirectionalLight *theLight = [[WhirlyKitDirectionalLight alloc] init];
//        theLight.pos = Vector3f(light.pos.x,light.pos.y,light.pos.z);
//        theLight.ambient = [light.ambient asVec4];
//        theLight.diffuse = [light.diffuse asVec4];
//        theLight.viewDependent = light.viewDependent;
//        [theLights addObject:theLight];
//    }
//    if ([theLights count] == 0)
//        theLights = nil;
//    if ([sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
//    {
//        WhirlyKitSceneRendererES2 *rendererES2 = (WhirlyKitSceneRendererES2 *)sceneRenderer;
//        [rendererES2 replaceLights:theLights];
//    }
//}
//
//- (void)clearLights
//{
//    lights = nil;
//    [self updateLights];
//}

// Note: Porting
//- (void)addLight:(MaplyLight *)light
//{
//    if (!lights)
//        lights = [NSMutableArray array];
//    [lights addObject:light];
//    [self updateLights];
//}
//
//- (void)removeLight:(MaplyLight *)light
//{
//    [lights removeObject:light];
//    [self updateLights];
//}
=======
// Build an array of lights and send them down all at once
- (void)updateLights
{
    NSMutableArray *theLights = [NSMutableArray array];
    for (MaplyLight *light in lights)
    {
        WhirlyKitDirectionalLight *theLight = [[WhirlyKitDirectionalLight alloc] init];
        theLight.pos = Vector3f(light.pos.x,light.pos.y,light.pos.z);
        theLight.ambient = [light.ambient asVec4];
        theLight.diffuse = [light.diffuse asVec4];
        theLight.viewDependent = light.viewDependent;
        [theLights addObject:theLight];
    }
    if ([theLights count] == 0)
        theLights = nil;
    if ([sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
    {
        WhirlyKitSceneRendererES2 *rendererES2 = (WhirlyKitSceneRendererES2 *)sceneRenderer;
        [rendererES2 replaceLights:theLights];
    }
}

- (void)clearLights
{
    lights = nil;
    [self updateLights];
}

- (void)addLight:(MaplyLight *)light
{
    if (!lights)
        lights = [NSMutableArray array];
    [lights addObject:light];
    [self updateLights];
}

- (void)removeLight:(MaplyLight *)light
{
    [lights removeObject:light];
    [self updateLights];
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (void)addShaderProgram:(MaplyShader *)shader sceneName:(NSString *)sceneName
{
    if (!shaders)
        shaders = [NSMutableArray array];

    if (![shaders containsObject:shader])
        [shaders addObject:shader];
    
    std::string theSceneName = [sceneName cStringUsingEncoding:NSASCIIStringEncoding];
    scene->addProgram(theSceneName, shader.program);
}

- (MaplyShader *)getShaderByName:(NSString *)name
{
    for (MaplyShader *shader in shaders)
        if (![shader.name compare:name])
            return shader;
    
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
    BOOL zBuffer = [hints boolForKey:kWGRenderHintZBuffer default:false];
<<<<<<< HEAD
    sceneRenderer->setZBufferMode((zBuffer ? zBufferOn : zBufferOffDefault));
    BOOL culling = [hints boolForKey:kWGRenderHintCulling default:true];
    sceneRenderer->setDoCulling(culling);
=======
    sceneRenderer.zBufferMode = (zBuffer ? zBufferOn : zBufferOffDefault);
    BOOL culling = [hints boolForKey:kWGRenderHintCulling default:true];
    sceneRenderer.doCulling = culling;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

#pragma mark - Geometry related methods

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;
{
    return [interactLayer addScreenMarkers:markers desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addScreenMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addMarkers:markers desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addMarkers:markers desc:desc mode:MaplyThreadAny];
}

<<<<<<< HEAD
// Note: Porting
//- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addScreenLabels:labels desc:desc mode:threadMode];
//}
//
//- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
//{
//    return [self addScreenLabels:labels desc:desc mode:MaplyThreadAny];
//}
//
//- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addLabels:labels desc:desc mode:threadMode];
//}
//
//- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
//{
//    return [self addLabels:labels desc:desc mode:MaplyThreadAny];
//}
=======
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addScreenLabels:labels desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addScreenLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addLabels:labels desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addLabels:labels desc:desc mode:MaplyThreadAny];
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addVectors:vectors desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addVectors:vectors desc:desc mode:MaplyThreadAny];
}

<<<<<<< HEAD
// Note: Porting
//- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addBillboards:billboards desc:desc mode:threadMode];
//}
=======
- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addBillboards:billboards desc:desc mode:threadMode];
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors
{
    return [interactLayer addSelectionVectors:vectors desc:nil];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    [interactLayer changeVectors:compObj desc:desc mode:MaplyThreadAny];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc
{
    [self changeVector:compObj desc:desc mode:MaplyThreadAny];
}

<<<<<<< HEAD
// Note: Porting
//- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addShapes:shapes desc:desc mode:threadMode];
//}
//
//- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
//{
//    return [self addShapes:shapes desc:desc mode:MaplyThreadAny];
//}
//
//- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addStickers:stickers desc:desc mode:threadMode];
//}
//
//- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
//{
//    return [self addStickers:stickers desc:desc mode:MaplyThreadAny];
//}
//
//- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer changeSticker:compObj desc:desc mode:threadMode];
//}

// Note: Porting
//- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
//{
//    return [interactLayer addLoftedPolys:polys desc:desc key:key cache:cacheDb mode:threadMode];
//}
//
//- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc
//{
//    return [self addLoftedPolys:polys key:key cache:cacheDb desc:desc mode:MaplyThreadAny];
//}

// Note: Porting
///// Add a view to track to a particular location
//- (void)addViewTracker:(MaplyViewTracker *)viewTrack
//{
//    // Make sure we're not duplicating and add the object
//    [self removeViewTrackForView:viewTrack.view];
//    [viewTrackers addObject:viewTrack];
//    
//    // Hook it into the renderer
//    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
//    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
//    sceneRenderer.triggerDraw = true;
//    
//    // And add it to the view hierarchy
//    if ([viewTrack.view superview] == nil)
//        [glView addSubview:viewTrack.view];
//}
//
///// Remove the view tracker associated with the given UIView
//- (void)removeViewTrackForView:(UIView *)view
//{
//    // Look for the entry
//    WGViewTracker *theTracker = nil;
//    for (WGViewTracker *viewTrack in viewTrackers)
//        if (viewTrack.view == view)
//        {
//            theTracker = viewTrack;
//            break;
//        }
//    
//    if (theTracker)
//    {
//        [viewTrackers removeObject:theTracker];
//        ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
//        vpGen->removeView(theTracker.view);
//        if ([theTracker.view superview] == glView)
//            [theTracker.view removeFromSuperview];
//        sceneRenderer.triggerDraw = true;
//    }
//}
=======
- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addShapes:shapes desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    return [self addShapes:shapes desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addStickers:stickers desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    return [self addStickers:stickers desc:desc mode:MaplyThreadAny];
}

- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer changeSticker:compObj desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [interactLayer addLoftedPolys:polys desc:desc key:key cache:cacheDb mode:threadMode];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc
{
    return [self addLoftedPolys:polys key:key cache:cacheDb desc:desc mode:MaplyThreadAny];
}

/// Add a view to track to a particular location
- (void)addViewTracker:(WGViewTracker *)viewTrack
{
    // Make sure we're not duplicating and add the object
    [self removeViewTrackForView:viewTrack.view];
    [viewTrackers addObject:viewTrack];
    
    // Hook it into the renderer
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    sceneRenderer.triggerDraw = true;
    
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
        sceneRenderer.triggerDraw = true;
    }
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (MaplyTexture *)addTexture:(UIImage *)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags mode:(MaplyThreadMode)threadMode
{
    MaplyTexture *maplyTex = [interactLayer addTexture:image imageFormat:imageFormat wrapFlags:wrapFlags mode:threadMode];
    maplyTex.viewC = self;
    
    return maplyTex;
}

- (void)removeTexture:(MaplyTexture *)texture mode:(MaplyThreadMode)threadMode
{
    [interactLayer removeTexture:texture];
}

<<<<<<< HEAD
// Note: Porting
//- (void)setMaxLayoutObjects:(int)maxLayoutObjects
//{
//    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
//    if (layoutManager)
//        layoutManager->setMaxDisplayObjects(maxLayoutObjects);
//}
=======
- (void)setMaxLayoutObjects:(int)maxLayoutObjects
{
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setMaxDisplayObjects(maxLayoutObjects);
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (void)removeObject:(MaplyComponentObject *)theObj
{
    [self removeObjects:@[theObj] mode:MaplyThreadAny];
}

- (void)removeObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];
}

- (void)removeObjects:(NSArray *)theObjs
{
    [self removeObjects:theObjs mode:MaplyThreadAny];
}

- (void)disableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer disableObjects:theObjs mode:threadMode];
}

- (void)enableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    [interactLayer enableObjects:theObjs mode:threadMode];
}

- (void)startChanges
{
    [interactLayer startChanges];
}

- (void)endChanges
{
    [interactLayer endChanges];
}

<<<<<<< HEAD
// Note: Porting
//- (void)addActiveObject:(MaplyActiveObject *)theObj
//{
//    if ([NSThread currentThread] != [NSThread mainThread])
//    {
//        NSLog(@"Must call addActiveObject: on the main thread.");
//        return;
//    }
//    
//    if (!activeObjects)
//        activeObjects = [NSMutableArray array];
//
//    if (![activeObjects containsObject:theObj])
//    {
//        scene->addActiveModel(theObj);
//        [activeObjects addObject:theObj];
//    }
//}
//
//- (void)removeActiveObject:(MaplyActiveObject *)theObj
//{
//    if ([NSThread currentThread] != [NSThread mainThread])
//    {
//        NSLog(@"Must call removeActiveObject: on the main thread.");
//        return;
//    }
//    
//    if ([activeObjects containsObject:theObj])
//    {
//        scene->removeActiveModel(theObj);
//        [activeObjects removeObject:theObj];
//    }
//}
//
//- (void)removeActiveObjects:(NSArray *)theObjs
//{
//    if ([NSThread currentThread] != [NSThread mainThread])
//    {
//        NSLog(@"Must call removeActiveObject: on the main thread.");
//        return;
//    }
//
//    for (MaplyActiveObject *theObj in theObjs)
//        [self removeActiveObject:theObj];
//}
=======
- (void)addActiveObject:(MaplyActiveObject *)theObj
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Must call addActiveObject: on the main thread.");
        return;
    }
    
    if (!activeObjects)
        activeObjects = [NSMutableArray array];

    if (![activeObjects containsObject:theObj])
    {
        scene->addActiveModel(theObj);
        [activeObjects addObject:theObj];
    }
}

- (void)removeActiveObject:(MaplyActiveObject *)theObj
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Must call removeActiveObject: on the main thread.");
        return;
    }
    
    if ([activeObjects containsObject:theObj])
    {
        scene->removeActiveModel(theObj);
        [activeObjects removeObject:theObj];
    }
}

- (void)removeActiveObjects:(NSArray *)theObjs
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Must call removeActiveObject: on the main thread.");
        return;
    }

    for (MaplyActiveObject *theObj in theObjs)
        [self removeActiveObject:theObj];
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

- (bool)addLayer:(MaplyViewControllerLayer *)newLayer
{
    if (newLayer && ![userLayers containsObject:newLayer])
    {
        WhirlyKitLayerThread *layerThread = baseLayerThread;
<<<<<<< HEAD
        // Note: Porting
        // Only supporting quad image tiles layer for the thread per layer
//        if (_threadPerLayer && [newLayer isKindOfClass:[MaplyQuadImageTilesLayer class]])
//        {
//            layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer mainLayerThread:false];
//            [layerThreads addObject:layerThread];
//            [layerThread start];
//        }
=======
        // Only supporting quad image tiles layer for the thread per layer
        if (_threadPerLayer && [newLayer isKindOfClass:[MaplyQuadImageTilesLayer class]])
        {
            layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer mainLayerThread:false];
            [layerThreads addObject:layerThread];
            [layerThread start];
        }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        
        if ([newLayer startLayer:layerThread scene:scene renderer:sceneRenderer viewC:self])
        {
            newLayer.drawPriority = layerDrawPriority++ + kMaplyImageLayerDrawPriorityDefault;
            [userLayers addObject:newLayer];
            return true;
        }
    }
    
    return false;
}

- (void)removeLayer:(MaplyViewControllerLayer *)layer
{
    bool found = false;
    MaplyViewControllerLayer *theLayer = nil;
    for (theLayer in userLayers)
    {
        if (theLayer == layer)
        {
            found = true;
            break;
        }
    }
    if (!found)
        return;
    
    WhirlyKitLayerThread *layerThread = layer.layerThread;
    [layer cleanupLayers:layerThread scene:scene];
    [userLayers removeObject:layer];
    
    // Need to shut down the layer thread too
    if (layerThread != baseLayerThread)
    {
        if ([layerThreads containsObject:layerThread])
        {
            [layerThreads removeObject:layerThread];
            [layerThread addThingToRelease:theLayer];
            [layerThread cancel];
        }
    }
}

- (void)removeLayers:(NSArray *)layers
{
    for (MaplyViewControllerLayer *layer in layers)
        [self removeLayer:layer];
}

- (void)removeAllLayers
{
    NSArray *allLayers = [NSArray arrayWithArray:userLayers];
    
    for (MaplyViewControllerLayer *theLayer in allLayers)
        [self removeLayer:theLayer];
}

#pragma mark - Properties

- (UIColor *)clearColor
{
    return theClearColor;
}

- (void)setClearColor:(UIColor *)clearColor
{
    theClearColor = clearColor;
<<<<<<< HEAD
    sceneRenderer->setClearColor([clearColor asRGBAColor]);
=======
    [sceneRenderer setClearColor:clearColor];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    // This is a hack for clear color
    RGBAColor theColor = [clearColor asRGBAColor];
    if (theColor.a < 255)
    {
        [self.view setBackgroundColor:[UIColor clearColor]];
        [glView setBackgroundColor:[UIColor clearColor]];
    }
}

- (MaplyCoordinate3d)displayPointFromGeo:(MaplyCoordinate)geoCoord
{
    MaplyCoordinate3d displayCoord;
<<<<<<< HEAD
    Point3f pt = visualView->coordAdapter->localToDisplay(visualView->coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
=======
    Point3f pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    displayCoord.x = pt.x();    displayCoord.y = pt.y();    displayCoord.z = pt.z();
    return displayCoord;
}

@end
