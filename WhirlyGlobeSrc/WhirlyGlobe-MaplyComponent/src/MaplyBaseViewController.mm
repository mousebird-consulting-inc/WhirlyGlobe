/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2015 mousebird consulting
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
#import "MaplyTexture_private.h"
#import "MaplyAnnotation_private.h"
#import "NSDictionary+StyleRules.h"
#import "DDXMLElementAdditions.h"
#import "NSString+DDXML.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplySelectedObject
@end

// Target for screen snapshot
@interface SnapshotTarget : NSObject<WhirlyKitSnapshot>
@property (nonatomic) UIImage *image;
@end

@implementation SnapshotTarget

- (void)snapshot:(UIImage *)image
{
    _image = image;
}

@end

@implementation MaplyBaseViewController
{
}

- (void) clear
{
    if (!scene)
        return;
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];

    [glView stopAnimation];
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [sceneRenderer useContext];
    for (MaplyShader *shader in shaders)
        [shader shutdown];
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
    sceneRenderer.scene = nil;
    
    if (baseLayerThread)
    {
        // Kill off all the other layers first
        for (unsigned int ii=1;ii<[layerThreads count];ii++)
        {
            WhirlyKitLayerThread *layerThread = [layerThreads objectAtIndex:ii];
            [baseLayerThread addThreadToShutdown:layerThread];
        }

        [baseLayerThread addThingToDelete:scene];
        [baseLayerThread addThingToRelease:baseLayerThread];
        [baseLayerThread addThingToRelease:visualView];
        [baseLayerThread addThingToRelease:glView];
        [baseLayerThread addThingToRelease:sceneRenderer];
        [baseLayerThread cancel];
    }
    layerThreads = nil;
    
    scene = NULL;
    visualView = nil;

    glView = nil;
    sceneRenderer = nil;
    baseLayerThread = nil;
    layoutLayer = nil;
    partSysLayer = nil;
    
    activeObjects = nil;
    
    interactLayer = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;

    viewTrackers = nil;
    annotations = nil;
    
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
    if (![sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
        return;
    
    [self resetLights];
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    return nil;
}

// Create the Maply or Globe view.
// For specific parts we'll call our subclasses
- (void) loadSetup
{
    allowRepositionForAnnnotations = true;
    
    _screenObjectDrawPriorityOffset = 1000000;
    
    // Need this logic here to pull in the categories
    static bool dummyInit = false;
    if (!dummyInit)
    {
        NSDataDummyFunc();
        NSDictionaryStyleDummyFunc();
        DDXMLElementDummyFunc();
        DDXMLDummyFunc();
        
        dummyInit = true;
    }
    
    userLayers = [NSMutableArray array];
    _threadPerLayer = true;
    
    [self loadSetup_glView];

	// Set up the OpenGL ES renderer
    sceneRenderer = [[WhirlyKitSceneRendererES3 alloc] init];
    if (!sceneRenderer)
        sceneRenderer = [[WhirlyKitSceneRendererES2 alloc] init];
    sceneRenderer.zBufferMode = zBufferOffDefault;
    // Switch to that context for any assets we create
    // Note: Should be switching back at the end
	[sceneRenderer useContext];

    theClearColor = [UIColor blackColor];
    [sceneRenderer setClearColor:theClearColor];

    // Set up the GL View to display it in
	glView.renderer = sceneRenderer;
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
    sceneRenderer.scene = scene;
    [self loadSetup_lighting];
    
    layerThreads = [NSMutableArray array];
    
    // Need a layer thread to manage the layers
	baseLayerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer mainLayerThread:true];
    [layerThreads addObject:baseLayerThread];
    
    // Layout still needs a layer to kick it off
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:sceneRenderer];
    [baseLayerThread addLayer:layoutLayer];
    
    // Particle systems need a layer for cleanup
    partSysLayer = [[WhirlyKitParticleSystemLayer alloc] init];
    [baseLayerThread addLayer:partSysLayer];
    
    // Lastly, an interaction layer of our own
    interactLayer = [self loadSetup_interactionLayer];
    interactLayer.screenObjectDrawPriorityOffset = _screenObjectDrawPriorityOffset;
    interactLayer.glView = glView;
    [baseLayerThread addLayer:interactLayer];
    
	// Give the renderer what it needs
	sceneRenderer.theView = visualView;
	    
    viewTrackers = [NSMutableArray array];
    annotations = [NSMutableArray array];
	
	// Kick off the layer thread
	// This will start loading things
	[baseLayerThread start];
    
    // Set up defaults for the hints
    NSDictionary *newHints = [NSDictionary dictionary];
    [self setHints:newHints];
        
    _selection = true;

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appBackground:)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appForeground:)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
}

- (void)setScreenObjectDrawPriorityOffset:(int)drawPriorityOffset
{
    _screenObjectDrawPriorityOffset = drawPriorityOffset;
    if (interactLayer)
        interactLayer.screenObjectDrawPriorityOffset = _screenObjectDrawPriorityOffset;
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

- (void)shutdown
{
    [interactLayer lockingShutdown];
    
    if (glView)
        [glView shutdown];
    
    [self clear];
}

- (void)appBackground:(NSNotification *)note
{
    if(!wasAnimating || glView.animating)
    {
        wasAnimating = glView.animating;
        if (wasAnimating)
            [self stopAnimation];
    }
    for(WhirlyKitLayerThread *t in layerThreads)
    {
        [t pause];
    }
}

- (void)appForeground:(NSNotification *)note
{
    for(WhirlyKitLayerThread *t in layerThreads)
    {
        [t unpause];
    }
    if (wasAnimating)
    {
        [self startAnimation];
        wasAnimating = false;
    }
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
    
    if (scene)
    {
        WhirlyKit::OpenGLMemManager *memManager = scene->getMemManager();
        // We may retain a bit of memory here.  Clear it up.
        if (memManager && sceneRenderer)
        {
            EAGLContext *oldContext = [EAGLContext currentContext];
            [sceneRenderer useContext];

            memManager->clearBufferIDs();
            memManager->clearTextureIDs();
            
            if (oldContext)
                [EAGLContext setCurrentContext:oldContext];
        }
    }
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
    return _performanceOutput;
}

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

- (void)resetLights
{
    [self clearLights];

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
        // Add a default light
        MaplyLight *light = [[MaplyLight alloc] init];
        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        light.viewDependent = false;
        [self addLight:light];
    }
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

- (void)addShaderProgram:(MaplyShader *)shader sceneName:(NSString *)sceneName
{
    if (!shader)
        return;
    
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
    sceneRenderer.zBufferMode = (zBuffer ? zBufferOn : zBufferOffDefault);
    BOOL culling = [hints boolForKey:kWGRenderHintCulling default:false];
    sceneRenderer.doCulling = culling;
}

#pragma mark - Geometry related methods

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addScreenMarkers:markers desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addScreenMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addMarkers:markers desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addScreenLabels:labels desc:desc mode:threadMode];

    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addScreenLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addLabels:labels desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addVectors:vectors desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addVectors:vectors desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer instanceVectors:baseObj desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addWideVectors:vectors desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addWideVectors:vectors desc:desc mode:MaplyThreadAny];
}


- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addBillboards:billboards desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addParticleSystem:partSys desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return compObj;
}

- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode
{
    if (![batch isValid])
        return;
    
    if (![interactLayer startOfWork])
        return;
    
    [interactLayer addParticleBatch:batch mode:threadMode];

    [interactLayer endOfWork];
}

- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addSelectionVectors:vectors desc:nil];
    [interactLayer endOfWork];
    
    return compObj;
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return;
    
    [interactLayer changeVectors:compObj desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc
{
    [self changeVector:compObj desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addShapes:shapes desc:desc mode:threadMode];

    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    return [self addShapes:shapes desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addModelInstances:modelInstances desc:desc mode:threadMode];

    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addGeometry:(NSArray *)geom desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addGeometry:geom desc:desc mode:threadMode];

    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addStickers:stickers desc:desc mode:threadMode];

    [interactLayer endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    return [self addStickers:stickers desc:desc mode:MaplyThreadAny];
}

- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return;
    
    [interactLayer changeSticker:compObj desc:desc mode:threadMode];

    [interactLayer endOfWork];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [interactLayer addLoftedPolys:polys desc:desc key:key cache:cacheDb mode:threadMode];
    [interactLayer endOfWork];
    
    return compObj;
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

    @synchronized(self)
    {
        [viewTrackers addObject:viewTrack];
    }
    
    // Hook it into the renderer
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    sceneRenderer.triggerDraw = true;
    
    // And add it to the view hierarchy
    // Can only do this on the main thread anyway
    if ([viewTrack.view superview] == nil)
        [glView addSubview:viewTrack.view];
}

- (void)moveViewTracker:(MaplyViewTracker *)viewTrack moveTo:(MaplyCoordinate)newPos
{
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();

    vpGen->moveView(GeoCoord(newPos.x,newPos.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    sceneRenderer.triggerDraw = true;
}

/// Remove the view tracker associated with the given UIView
- (void)removeViewTrackForView:(UIView *)view
{
    @synchronized(self)
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
}

// Overridden by the subclasses
- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    return CGPointZero;
}

// Overridden by the subclasses
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    return false;
}

- (void)addAnnotation:(MaplyAnnotation *)annotate forPoint:(MaplyCoordinate)coord offset:(CGPoint)offset
{
    // See if we're already representing the annotation
    bool alreadyHere = [annotations containsObject:annotate];
    
    // Let's put it in the right place so the callout can do its layout logic
    CGPoint pt = [self screenPointFromGeo:coord];
    CGRect rect = CGRectMake(pt.x+offset.x, pt.y+offset.y, 0.0, 0.0);
    annotate.loc = coord;
    if (!alreadyHere)
    {
        annotate.calloutView.delegate = self;
        [annotations addObject:annotate];
        [annotate.calloutView presentCalloutFromRect:rect inView:glView constrainedToView:glView animated:YES];
    } else {
        annotate.calloutView.delegate = nil;
        [annotate.calloutView presentCalloutFromRect:rect inView:glView constrainedToView:glView animated:NO];
    }
    
    // But then we move it back because we're controlling its positioning
    CGRect frame = annotate.calloutView.frame;
    annotate.calloutView.frame = CGRectMake(frame.origin.x-pt.x+offset.x, frame.origin.y-pt.y+offset.y, frame.size.width, frame.size.height);

    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    if (alreadyHere)
    {
        vpGen->moveView(GeoCoord(coord.x,coord.y),annotate.calloutView,annotate.minVis,annotate.maxVis);
    } else
    {
        vpGen->addView(GeoCoord(coord.x,coord.y),annotate.calloutView,annotate.minVis,annotate.maxVis);
    }
    sceneRenderer.triggerDraw = true;
}

// Delegate callback for annotation placement
// Note: Not doing anything with this yet
- (NSTimeInterval)calloutView:(SMCalloutView *)calloutView delayForRepositionWithSize:(CGSize)offset
{
    NSTimeInterval delay = 0.0;
    
    // Need to find the annotation this belongs to
    for (MaplyAnnotation *annotation in annotations)
    {
        if (annotation.calloutView == calloutView && annotation.repositionForVisibility && allowRepositionForAnnnotations)
        {
            CGPoint pt = [self screenPointFromGeo:annotation.loc];
            CGPoint newPt = CGPointMake(pt.x+offset.width, pt.y+offset.height);
            delay = 0.25;
            if (![self animateToPosition:annotation.loc onScreen:newPt time:delay])
                delay = 0.0;
            break;
        }
    }
    
    return 0.0;
}

- (void)removeAnnotation:(MaplyAnnotation *)annotate
{
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    vpGen->removeView(annotate.calloutView);
    
    [annotations removeObject:annotate];
    
    [annotate.calloutView dismissCalloutAnimated:YES];
}

- (void)freezeAnnotation:(MaplyAnnotation *)annotate
{
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    for (MaplyAnnotation *annotation in annotations)
        if (annotate == annotation)
        {
            vpGen->freezeView(annotate.calloutView);
        }
}

- (void)unfreezeAnnotation:(MaplyAnnotation *)annotate
{
    ViewPlacementGenerator *vpGen = scene->getViewPlacementGenerator();
    for (MaplyAnnotation *annotation in annotations)
        if (annotate == annotation)
        {
            vpGen->unfreezeView(annotate.calloutView);
        }
    sceneRenderer.triggerDraw = true;
}

- (NSArray *)annotations
{
    return annotations;
}

- (void)clearAnnotations
{
    NSArray *allAnnotations = [NSArray arrayWithArray:annotations];
    for (MaplyAnnotation *annotation in allAnnotations)
        [self removeAnnotation:annotation];
}

- (MaplyTexture *)addTexture:(UIImage *)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags mode:(MaplyThreadMode)threadMode
{
    return [self addTexture:image desc:@{kMaplyTexFormat: @(imageFormat),
                                         kMaplyTexWrapX: @(wrapFlags & MaplyImageWrapX),
                                         kMaplyTexWrapY: @(wrapFlags & MaplyImageWrapY)}
                                         mode:threadMode];
}

- (MaplyTexture *)addTexture:(UIImage *)image desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return nil;
    
    MaplyTexture *maplyTex = [interactLayer addTexture:image desc:desc mode:threadMode];
    
    [interactLayer endOfWork];
    
    return maplyTex;
}

- (void)removeTexture:(MaplyTexture *)texture mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return;
    
    [interactLayer removeTextures:@[texture] mode:threadMode];

    [interactLayer endOfWork];
}

- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode
{
    if (![interactLayer startOfWork])
        return;
    
    [interactLayer removeTextures:textures mode:threadMode];

    [interactLayer endOfWork];
}

- (MaplyTexture *)addTextureToAtlas:(UIImage *)image mode:(MaplyThreadMode)threadMode
{
    MaplyTexture *maplyTex = [self addTextureToAtlas:image imageFormat:MaplyImageIntRGBA wrapFlags:0 mode:threadMode];
    
    return maplyTex;
}

- (MaplyTexture *)addTextureToAtlas:(UIImage *)image imageFormat:(MaplyQuadImageFormat)imageFormat wrapFlags:(int)wrapFlags mode:(MaplyThreadMode)threadMode
{
    return [self addTexture:image desc:@{kMaplyTexFormat: @(imageFormat),
                                         kMaplyTexWrapX: @(wrapFlags & MaplyImageWrapX),
                                         kMaplyTexWrapY: @(wrapFlags & MaplyImageWrapY),
                                         kMaplyTexAtlas: @(YES)} mode:threadMode];
}

- (void)setMaxLayoutObjects:(int)maxLayoutObjects
{
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setMaxDisplayObjects(maxLayoutObjects);
}

- (void)removeObject:(MaplyComponentObject *)theObj
{
    if (!theObj)
        return;

    [self removeObjects:@[theObj] mode:MaplyThreadAny];
}

- (void)removeObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    if (!theObjs)
        return;

    if (![interactLayer startOfWork])
        return;
    
    [interactLayer removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];

    [interactLayer endOfWork];
}

- (void)removeObjects:(NSArray *)theObjs
{
    if (!theObjs)
        return;
    
    [self removeObjects:theObjs mode:MaplyThreadAny];
}

- (void)disableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    if (!theObjs)
        return;

    if (![interactLayer startOfWork])
        return;
    
    [interactLayer disableObjects:theObjs mode:threadMode];

    [interactLayer endOfWork];
}

- (void)enableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    if (!theObjs)
        return;

    if (![interactLayer startOfWork])
        return;

    [interactLayer enableObjects:theObjs mode:threadMode];

    [interactLayer endOfWork];
}

- (void)startChanges
{
    if (![interactLayer startOfWork])
        return;

    [interactLayer startChanges];

    [interactLayer endOfWork];
}

- (void)endChanges
{
    if (![interactLayer startOfWork])
        return;

    [interactLayer endChanges];

    [interactLayer endOfWork];
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
        WhirlyKitLayerThread *layerThread = baseLayerThread;
        // Only supporting quad image tiles layer for the thread per layer
        if (_threadPerLayer && [newLayer isKindOfClass:[MaplyQuadImageTilesLayer class]])
        {
            layerThread = [[WhirlyKitLayerThread alloc] initWithScene:scene view:visualView renderer:sceneRenderer mainLayerThread:false];
            [layerThreads addObject:layerThread];
            [layerThread start];
        }
        
        if ([newLayer startLayer:layerThread scene:scene renderer:sceneRenderer viewC:self])
        {
            if (!newLayer.drawPriorityWasSet)
            {
                newLayer.drawPriority = 100*(layerDrawPriority++) + kMaplyImageLayerDrawPriorityDefault;
            }
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

            // We also have to make sure it actually does finish
            bool finished = true;
            do {
                finished = [layerThread isFinished];
                if (!finished)
                    [NSThread sleepForTimeInterval:0.0001];
            } while (!finished);
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
    [sceneRenderer setClearColor:clearColor];
    
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
    Point3f pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
    
    displayCoord.x = pt.x();    displayCoord.y = pt.y();    displayCoord.z = pt.z();
    return displayCoord;
}

- (float)currentMapScale
{
    Point2f frameSize(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    if (frameSize.x() == 0)
        return MAXFLOAT;
    return (float)[visualView currentMapScale:frameSize];
}

- (float)heightForMapScale:(float)scale
{
    Point2f frameSize(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    if (frameSize.x() == 0)
        return -1.0;
    return (float)[visualView heightForMapScale:scale frame:frameSize];
}

- (UIImage *)snapshot
{
    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    sceneRenderer.snapshotDelegate = target;
    
    [sceneRenderer forceDrawNextFrame];
    [sceneRenderer render:0.0];
    
    return target.image;
}

- (float)currentMapZoom:(MaplyCoordinate)coordinate
{
    Point2f frameSize(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    if (frameSize.x() == 0)
        return MAXFLOAT;
    return (float)[visualView currentMapZoom:frameSize latitude:coordinate.y];
}

- (MaplyCoordinateSystem *)coordSystem
{
    // Note: Hack.  Should wrap the real coordinate system
    MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return coordSys;
}

- (MaplyCoordinate3d)displayCoordFromLocal:(MaplyCoordinate3d)localCoord
{
    Point3d pt = visualView.coordAdapter->localToDisplay(Point3d(localCoord.x,localCoord.y,localCoord.z));
    
    MaplyCoordinate3d ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    return ret;
}

- (MaplyCoordinate3d)displayCoord:(MaplyCoordinate3d)localCoord fromSystem:(MaplyCoordinateSystem *)coordSys
{
    Point3d loc3d = CoordSystemConvert3d(coordSys->coordSystem, visualView.coordAdapter->getCoordSystem(), Point3d(localCoord.x,localCoord.y,localCoord.z));
    Point3d pt = visualView.coordAdapter->localToDisplay(loc3d);
    
    MaplyCoordinate3d ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    return ret;
}

@end
