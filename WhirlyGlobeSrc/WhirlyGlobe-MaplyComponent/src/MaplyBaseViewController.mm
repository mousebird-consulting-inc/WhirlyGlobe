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
{
    NSDictionary *newScreenLabelDesc,*newLabelDesc,*newScreenMarkerDesc,*newMarkerDesc,
    *newVectorDesc,*newShapeDesc,*newStickerDesc,*newLoftPolyDesc;
}

@synthesize selection;

- (void) clear
{
    if (!scene)
        return;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];

    [glView stopAnimation];
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [sceneRenderer useContext];
    for (MaplyShader *shader in shaders)
        [shader shutdown];
    scene->teardownGL();
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
    sceneRenderer.scene = nil;
    
    if (layerThread)
    {
        [layerThread addThingToDelete:scene];
        [layerThread addThingToRelease:layerThread];
        [layerThread addThingToRelease:visualView];
        [layerThread addThingToRelease:glView];
        [layerThread addThingToRelease:sceneRenderer];
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
    loftLayer = nil;
    activeObjects = nil;
    
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

- (void) dealloc
{
    if (scene)
        [self clear];
}

- (WhirlyKitView *) loadSetup_view
{
    return nil;
}

- (void)loadSetup_glView
{
    glView = [[WhirlyKitEAGLView alloc] init];
}

- (WhirlyKit::Scene *) loadSetup_scene
{
    return NULL;
}

static const char *vertexShaderNoLightTri =
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord;                 \n"
"   v_color = a_color * u_fade;\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderNoLightTri =
"precision mediump float;                            \n"
"\n"
"uniform sampler2D s_baseMap;                        \n"
"uniform bool  u_hasTexture;                         \n"
"\n"
"varying vec2      v_texCoord;                       \n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
"  vec4 baseColor = texture2D(s_baseMap, v_texCoord); \n"
//"  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap, v_texCoord) : vec4(1.0,1.0,1.0,1.0); \n"
//"  if (baseColor.a < 0.1)                            \n"
//"      discard;                                      \n"
"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;

static const char *vertexShaderNoLightLine =
"uniform mat4  u_mvpMatrix;                   \n"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec4 a_color;                     \n"
"uniform float u_fade;                        \n"
"\n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_color = a_color * u_fade;                       \n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderNoLightLine =
"precision mediump float;                            \n"
"\n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
"  gl_FragColor = v_color;  \n"
"}                                                   \n"
;

- (void) loadSetup_lighting
{
    if (![sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
        return;
    
    NSString *lightingType = hints[kWGRendererLightingMode];
    int lightingRegular = true;
    if ([lightingType respondsToSelector:@selector(compare:)])
        lightingRegular = [lightingType compare:@"none"];
    
    // Regular lighting is on by default
    // We need to add a new shader to turn it off
    if (!lightingRegular)
    {
        // Note: Should keep a reference to these around.  I bet we're leaking
        OpenGLES2Program *triShader = new OpenGLES2Program("Default No Lighting Triangle Program",vertexShaderNoLightTri,fragmentShaderNoLightTri);
        OpenGLES2Program *lineShader = new OpenGLES2Program("Default Line Program",vertexShaderNoLightLine,fragmentShaderNoLightLine);
        if (!triShader->isValid() || !lineShader->isValid())
        {
            NSLog(@"MaplyBaseViewController: Shader didn't compile.  Using default.");
            delete triShader;
            delete lineShader;
        } else {
            scene->setDefaultPrograms(triShader,lineShader);
        }
    } else {
        // Add a default light
        MaplyLight *light = [[MaplyLight alloc] init];
        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        light.viewDependent = false;
        [self addLight:light];
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
    userLayers = [NSMutableArray array];
    
    [self loadSetup_glView];

	// Set up the OpenGL ES renderer
    NSNumber *renderNum = hints[kWGRendererOpenGLVersion];
    int whichRenderer = 2;
    if ([renderNum respondsToSelector:@selector(integerValue)])
        whichRenderer = [renderNum integerValue];
    switch (whichRenderer)
    {
        case 1:
            sceneRenderer = [[WhirlyKitSceneRendererES1 alloc] init];
            break;
        case 2:
        default:
            sceneRenderer = [[WhirlyKitSceneRendererES2 alloc] init];
            break;
    }
    sceneRenderer.zBufferMode = zBufferOffUntilLines;
    // Switch to that context for any assets we create
    // Note: Should be switching back at the end
	[sceneRenderer useContext];

    theClearColor = [UIColor blackColor];
    [sceneRenderer setClearColor:theClearColor];

    // Set up the GL View to display it in
	glView = [[WhirlyKitEAGLView alloc] init];
	glView.renderer = sceneRenderer;
    // Note: Should be able to change this
	glView.frameInterval = 1;  // 60 fps
    [self.view insertSubview:glView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
    
    // Turn on the model matrix optimization for drawing
    sceneRenderer.useViewChanged = true;
    
	// Need an empty scene and view
    visualView = [self loadSetup_view];
    scene = [self loadSetup_scene];
    [self loadSetup_lighting];
    
    // Need a layer thread to manage the layers
	layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer];
    
	// Set up the vector layer where all our outlines will go
	vectorLayer = [[WhirlyKitVectorLayer alloc] init];
	[layerThread addLayer:vectorLayer];
    
    // Set up the shape layer.  Manages a set of simple shapes
    shapeLayer = [[WhirlyKitShapeLayer alloc] init];
    [layerThread addLayer:shapeLayer];
    
    // Set up the chunk layer.  Used for stickers.
    chunkLayer = [[WhirlyKitSphericalChunkLayer alloc] init];
    chunkLayer.ignoreEdgeMatching = true;
    [layerThread addLayer:chunkLayer];
    
	// General purpose label layer.
	labelLayer = [[WhirlyKitLabelLayer alloc] init];
	[layerThread addLayer:labelLayer];
    
    // Marker layer
    markerLayer = [[WhirlyKitMarkerLayer alloc] init];
    [layerThread addLayer:markerLayer];
    
    // 2D layout engine layer
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer];
    labelLayer.layoutLayer = layoutLayer;
    markerLayer.layoutLayer = layoutLayer;
    [layerThread addLayer:layoutLayer];
    
    // Lofted polygon layer
    loftLayer = [[WhirlyKitLoftLayer alloc] init];
    [layerThread addLayer:loftLayer];
    
    // Lastly, an interaction layer of our own
    interactLayer = [self loadSetup_interactionLayer];
    interactLayer.vectorLayer = vectorLayer;
    interactLayer.labelLayer = labelLayer;
    interactLayer.markerLayer = markerLayer;
    interactLayer.shapeLayer = shapeLayer;
    interactLayer.chunkLayer = chunkLayer;
    interactLayer.loftLayer = loftLayer;
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
                              nil];
    [self setHints:newHints];
    
    // Set up default descriptions for the various data types
    newScreenLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithFloat:1.0], kMaplyFade,
                                        nil];
    
    newLabelDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSNumber numberWithInteger:kWGLabelDrawOffsetDefault], kWGDrawOffset,
                                  [NSNumber numberWithInteger:kWGLabelDrawPriorityDefault], kWGDrawPriority,
                                  [NSNumber numberWithFloat:1.0], kMaplyFade,
                                  nil];
    
    newScreenMarkerDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                         [NSNumber numberWithFloat:1.0], kMaplyFade,
                                         nil];
    
    newMarkerDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithInteger:kWGMarkerDrawOffsetDefault], kWGDrawOffset,
                                   [NSNumber numberWithInteger:kWGMarkerDrawPriorityDefault], kWGDrawPriority,
                                   [NSNumber numberWithFloat:1.0], kMaplyFade,
                                   nil];
    
    newVectorDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [NSNumber numberWithInteger:kWGVectorDrawOffsetDefault], kWGDrawOffset,
                                   [NSNumber numberWithInteger:kWGVectorDrawPriorityDefault], kWGDrawPriority,
                                   [NSNumber numberWithFloat:1.0], kMaplyFade,
                                   nil];
    
    newShapeDesc = [NSDictionary dictionaryWithObjectsAndKeys:
                                  [NSNumber numberWithFloat:1.0], kMaplyFade,
                                  nil];
    
    newStickerDesc = @{kWGDrawOffset: @(kWGStickerDrawOffsetDefault), kWGDrawPriority: @(kWGStickerDrawPriorityDefault), kWGSampleX: @(15), kWGSampleY: @(15)};
    
    newLoftPolyDesc = @{kWGColor: [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:0.5], kMaplyLoftedPolyHeight: @(0.01)};
    
    selection = true;
    
}

- (void) useGLContext
{
    [sceneRenderer useContext];
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

static const float PerfOutputDelay = 15.0;

- (void)setPerformanceOutput:(bool)performanceOutput
{
    if (perfOutput == performanceOutput)
        return;
    
    perfOutput = performanceOutput;
    if (perfOutput)
    {
        sceneRenderer.perfInterval = 100;
        [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];
    } else {
        sceneRenderer.perfInterval = 0;
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
    return perfOutput;
}

// Build an array of lights and send them down all at once
- (void)updateLights
{
    NSMutableArray *theLights = [NSMutableArray array];
    for (MaplyLight *light in lights)
    {
        WhirlyKitDirectionalLight *theLight = [[WhirlyKitDirectionalLight alloc] init];
        theLight->pos.x() = light.pos.x;  theLight->pos.y() = light.pos.y;  theLight->pos.z() = light.pos.z;
        theLight->ambient = [light.ambient asVec4];
        theLight->diffuse = [light.diffuse asVec4];
        theLight->viewDependent = light.viewDependent;
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

- (void)addShader:(MaplyShader *)shader
{
    if (!shaders)
        shaders = [NSMutableArray array];
    [shaders addObject:shader];
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithMBTiles:(NSString *)name
{
    MaplyQuadEarthWithMBTiles *newLayer = [[MaplyQuadEarthWithMBTiles alloc] initWithMbTiles:name];
    newLayer.handleEdges = (sceneRenderer.zBufferMode != zBufferOff);
    if (!newLayer)
        return nil;
    
    if ([self addLayer:newLayer])
        return newLayer;
    
    return nil;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSString *)baseURL imageExt:(NSString *)ext cache:(NSString *)cacheDir minZoom:(int)minZoom maxZoom:(int)maxZoom;
{
    MaplyQuadEarthWithRemoteTiles *newLayer = [[MaplyQuadEarthWithRemoteTiles alloc] initWithBaseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom];
    newLayer.handleEdges = (sceneRenderer.zBufferMode != zBufferOff);
    newLayer.cacheDir = cacheDir;

    if (!newLayer)
        return nil;

    if ([self addLayer:newLayer])
        return newLayer;
    
    return nil;
}

- (MaplyViewControllerLayer *)addQuadEarthLayerWithRemoteSource:(NSDictionary *)jsonDict cache:(NSString *)cacheDir
{
    MaplyQuadEarthWithRemoteTiles *newLayer = [[MaplyQuadEarthWithRemoteTiles alloc] initWithTilespec:jsonDict];
    
    newLayer.handleEdges = (sceneRenderer.zBufferMode != zBufferOff);
    newLayer.cacheDir = cacheDir;
    
    if (!newLayer)
        return nil;
    
    if ([self addLayer:newLayer])
        return newLayer;
    
    return nil;
}

- (MaplyViewControllerLayer *)addQuadSphericalEarthLayerWithImageSet:(NSString *)imageSet
{
    MaplySphericalQuadEarthWithTexGroup *newLayer = [[MaplySphericalQuadEarthWithTexGroup alloc] initWithWithTexGroup:imageSet];
    
    if (!newLayer)
        return nil;
    
    if ([self addLayer:newLayer])
        return newLayer;
    
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
    sceneRenderer.zBufferMode = (zBuffer ? zBufferOffUntilLines : zBufferOff);
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

- (void)setLoftedPolyDesc:(NSDictionary *)desc
{
    loftDesc = [self mergeAndCheck:loftDesc changeDict:desc];
}

#pragma mark - Geometry related methods

/// Add a group of screen (2D) markers
- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers
{
    return [interactLayer addScreenMarkers:markers desc:screenMarkerDesc];
}

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [interactLayer addScreenMarkers:markers desc:desc];
}

/// Add a group of 3D markers
- (MaplyComponentObject *)addMarkers:(NSArray *)markers
{
    return [interactLayer addMarkers:markers desc:markerDesc];
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [interactLayer addMarkers:markers desc:desc];
}

/// Add a group of screen (2D) labels
- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels
{
    return [interactLayer addScreenLabels:labels desc:screenLabelDesc];
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [interactLayer addScreenLabels:labels desc:desc];    
}

/// Add a group of 3D labels
- (MaplyComponentObject *)addLabels:(NSArray *)labels
{
    return [interactLayer addLabels:labels desc:labelDesc];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [interactLayer addLabels:labels desc:desc];
}

/// Add one or more vectors
- (MaplyComponentObject *)addVectors:(NSArray *)vectors
{
    return [interactLayer addVectors:vectors desc:vectorDesc];
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [interactLayer addVectors:vectors desc:desc];
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

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    return [interactLayer addShapes:shapes desc:desc];
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers
{
    return [interactLayer addStickers:stickers desc:stickerDesc];
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    return [interactLayer addStickers:stickers desc:desc];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb;
{
    return [interactLayer addLoftedPolys:polys desc:loftDesc key:key cache:cacheDb];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc
{
    return [interactLayer addLoftedPolys:polys desc:desc key:key cache:cacheDb];
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
    [sceneRenderer setTriggerDraw];
    
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
        [sceneRenderer setTriggerDraw];
    }
}

- (void)setMaxLayoutObjects:(int)maxLayoutObjects
{
    layoutLayer.maxDisplayObjects = maxLayoutObjects;
}

/// Remove the data associated with an object the user added earlier
- (void)removeObject:(WGComponentObject *)theObj
{
    [interactLayer removeObject:theObj];
}

- (void)removeObjects:(NSArray *)theObjs
{
    [interactLayer removeObjects:[NSArray arrayWithArray:theObjs]];
}

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

- (bool)addLayer:(MaplyViewControllerLayer *)newLayer
{
    if (newLayer && ![userLayers containsObject:newLayer])
    {
        if ([newLayer startLayer:layerThread scene:scene renderer:sceneRenderer viewC:self])
        {
            [userLayers addObject:newLayer];
            return true;
        }
    }
    
    return false;
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

- (void)setDefaultPolyShader:(MaplyShader *)shader
{
    scene->setDefaultPrograms(shader.program, NULL);
}

- (void)setDefaultLineShader:(MaplyShader *)shader
{
    scene->setDefaultPrograms(NULL, shader.program);
}


@end
