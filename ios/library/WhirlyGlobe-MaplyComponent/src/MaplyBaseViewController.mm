/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2017 mousebird consulting
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
#import "Maply3dTouchPreviewDelegate.h"
#import "MaplyTexture_private.h"
#import "MaplyRenderTarget_private.h"
#import <sys/utsname.h>

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplySelectedObject
@end

// Target for screen snapshot
@interface SnapshotTarget : NSObject<WhirlyKitSnapshot>
@property (nonatomic) UIImage *image;
@property (nonatomic) NSData *data;
@property (nonatomic) SimpleIdentity renderTargetID;
@end

@implementation SnapshotTarget

- (instancetype)init
{
    self = [super init];
    
    _image = nil;
    _data = nil;
    _renderTargetID = EmptyIdentity;
    
    return self;
}

- (void)snapshotData:(NSData *)snapshotData {
    _data = snapshotData;
}

- (void)snapshotImage:(UIImage *)snapshotImage {
    _image = snapshotImage;
}

@end

@implementation MaplyBaseViewController
{
    MaplyLocationTracker *_locationTracker;
}

- (void) clear
{
    if (!renderControl)
        return;
    
    if (!renderControl->scene)
        return;
    
    for (auto tileFetcher : tileFetchers)
        [tileFetcher shutdown];
    tileFetchers.clear();
    
    defaultClusterGenerator = nil;
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];

    [glView stopAnimation];
    [glView teardown];
    
    [renderControl teardown];
    
    if (baseLayerThread)
    {
        // Kill off all the other layers first
        for (unsigned int ii=1;ii<[layerThreads count];ii++)
        {
            WhirlyKitLayerThread *layerThread = [layerThreads objectAtIndex:ii];
            [baseLayerThread addThreadToShutdown:layerThread];
        }

        [baseLayerThread addThingToDelete:renderControl->scene];
        [baseLayerThread addThingToRelease:baseLayerThread];
        [baseLayerThread addThingToRelease:visualView];
        [baseLayerThread addThingToRelease:glView];
        [baseLayerThread addThingToRelease:renderControl->sceneRenderer];
        [baseLayerThread cancel];
    }
    layerThreads = nil;
    visualView = nil;

    glView = nil;
    renderControl->scene = NULL;
    renderControl->sceneRenderer = nil;
    baseLayerThread = nil;
    layoutLayer = nil;
    partSysLayer = nil;
    
    activeObjects = nil;
    
    while ([userLayers count] > 0)
    {
        MaplyViewControllerLayer *layer = [userLayers objectAtIndex:0];
        [userLayers removeObject:layer];
    }
    userLayers = nil;

    viewTrackers = nil;
    annotations = nil;
    
    [renderControl clear];
    renderControl = nil;
}

- (void) dealloc
{
    if (renderControl && renderControl->scene)
        [self teardown];
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
    [renderControl resetLights];
    if (![renderControl->sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
        return;
    
    [self resetLights];
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    return nil;
}

- (void)setScreenObjectDrawPriorityOffset:(int)screenObjectDrawPriorityOffset
{
    renderControl.screenObjectDrawPriorityOffset = screenObjectDrawPriorityOffset;
}

- (int)screenObjectDrawPriorityOffset
{
    return renderControl.screenObjectDrawPriorityOffset;
}

// Kick off the analytics logic.  First we need the server name.
- (void)startAnalytics
{
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    // This is completely random.  We can't track it in any useful way
    NSString *userID = [userDefaults stringForKey:@"wgmaplyanalyticuser"];
    NSTimeInterval lastSent = 0.0;
    if (!userID) {
        userID = [[NSUUID UUID] UUIDString];
        [userDefaults setObject:userID forKey:@"wgmaplyanalyticuser"];
    } else {
        lastSent = [userDefaults doubleForKey:@"wgmaplyanalytictime"];
    }
    
    // Sent once a month at most
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    if (now - lastSent < 30*24*60*60.0)
        return;

    [self sendAnalytics:@"analytics.mousebirdconsulting.com:8081"];
}

// Send the actual analytics data
// There's nothing unique in here to identify the user
// The user ID is completely made up and we don't get it more than once per week
- (void)sendAnalytics:(NSString *)serverName
{
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    NSString *userID = [userDefaults stringForKey:@"wgmaplyanalyticuser"];

    // Model number
    struct utsname systemInfo;
    uname(&systemInfo);
    NSString *model = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    NSDictionary *infoDict = [NSBundle mainBundle].infoDictionary;
    // Bundle ID, version and build
    NSString *bundleID = infoDict[@"CFBundleIdentifier"];
    NSString *bundleName = infoDict[@"CFBundleName"];
    NSString *build = infoDict[@"CFBundleVersion"];
    NSString *bundleVersion = infoDict[@"CFBundleShortVersionString"];
    // WGMaply version
    NSString *wgmaplyVersion = @"2.6.2";
    // OS version
    NSOperatingSystemVersion osversionID = [[NSProcessInfo processInfo] operatingSystemVersion];
    NSString *osversion = [NSString stringWithFormat:@"%d.%d.%d",(int)osversionID.majorVersion,(int)osversionID.minorVersion,(int) osversionID.patchVersion];

    // We're not recording anything that can identify the user, just the app
    // create table register( userid VARCHAR(50), bundleid VARCHAR(100), bundlename VARCHAR(100), bundlebuild VARCHAR(100), bundleversion VARCHAR(100), osversion VARCHAR(20), model VARCHAR(100), wgmaplyversion VARCHAR(20));
    NSString *postArgs = [NSString stringWithFormat:@"{ \"userid\":\"%@\", \"bundleid\":\"%@\", \"bundlename\":\"%@\", \"bundlebuild\":\"%@\", \"bundleversion\":\"%@\", \"osversion\":\"%@\", \"model\":\"%@\", \"wgmaplyversion\":\"%@\" }",
                          userID,bundleID,bundleName,build,bundleVersion,osversion,model,wgmaplyVersion];
    NSMutableURLRequest *req = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"http://%@/register", serverName]]];
    [req setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
    [req setHTTPMethod:@"POST"];
    [req setHTTPBody:[postArgs dataUsingEncoding:NSASCIIStringEncoding]];
    
    NSURLSession *session = [NSURLSession sharedSession];
    NSURLSessionDataTask *dataTask = [session dataTaskWithRequest:req completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        NSTimeInterval now = CFAbsoluteTimeGetCurrent();
        NSHTTPURLResponse *resp = (NSHTTPURLResponse *)response;
        if (resp.statusCode == 200) {
            [userDefaults setDouble:now forKey:@"wgmaplyanalytictime"];
        }
    }];
    [dataTask resume];
}

// Create the Maply or Globe view.
// For specific parts we'll call our subclasses
- (void) loadSetup
{
#if !TARGET_OS_SIMULATOR
    [self startAnalytics];
#endif
    
    if (!renderControl)
        renderControl = [[MaplyRenderController alloc] init];
    
    tileFetcherConnections = 16;
    allowRepositionForAnnnotations = true;
    
    userLayers = [NSMutableArray array];
    _threadPerLayer = true;
    
    [self loadSetup_glView];
    
    [renderControl loadSetup];

    // Set up the GL View to display it in
	glView.renderer = renderControl->sceneRenderer;
    [self.view insertSubview:glView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	glView.frame = self.view.bounds;
    glView.backgroundColor = [UIColor blackColor];
    
	// Need an empty scene and view
    visualView = [self loadSetup_view];
    renderControl->scene = [self loadSetup_scene];
    renderControl->sceneRenderer.scene = renderControl->scene;
    [self loadSetup_lighting];
    
    layerThreads = [NSMutableArray array];
    
    // Need a layer thread to manage the layers
	baseLayerThread = [[WhirlyKitLayerThread alloc] initWithScene:renderControl->scene view:visualView renderer:renderControl->sceneRenderer mainLayerThread:true];
    [layerThreads addObject:baseLayerThread];
    
    // Layout still needs a layer to kick it off
    layoutLayer = [[WhirlyKitLayoutLayer alloc] initWithRenderer:renderControl->sceneRenderer];
    [baseLayerThread addLayer:layoutLayer];
    
    // Particle systems need a layer for cleanup
    partSysLayer = [[WhirlyKitParticleSystemLayer alloc] init];
    [baseLayerThread addLayer:partSysLayer];
    
    // Lastly, an interaction layer of our own
    renderControl->interactLayer = [self loadSetup_interactionLayer];
    renderControl->interactLayer.screenObjectDrawPriorityOffset = renderControl.screenObjectDrawPriorityOffset;
    renderControl->interactLayer.glView = glView;
    renderControl->interactLayer->layerThreads = layerThreads;
    [baseLayerThread addLayer:renderControl->interactLayer];
    
	// Give the renderer what it needs
	renderControl->sceneRenderer.theView = visualView;
	    
    viewTrackers = [NSMutableArray array];
    annotations = [NSMutableArray array];
	
	// Kick off the layer thread
	// This will start loading things
	[baseLayerThread start];
    
    // Default cluster generator
    defaultClusterGenerator = [[MaplyBasicClusterGenerator alloc] initWithColors:@[[UIColor orangeColor]] clusterNumber:0 size:CGSizeMake(32,32) viewC:self];
    [self addClusterGenerator:defaultClusterGenerator];
    
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

- (void) useGLContext
{
    [renderControl useGLContext];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self loadSetup];
}

- (void)startAnimation
{
    [glView startAnimation];
}

- (void)stopAnimation
{
    [glView stopAnimation];
}

- (void)teardown
{
    if (renderControl)
        [renderControl->interactLayer lockingShutdown];
    
    if (glView)
        [glView teardown];
    
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

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    if (renderControl && renderControl->scene)
    {
        WhirlyKit::OpenGLMemManager *memManager = renderControl->scene->getMemManager();
        // We may retain a bit of memory here.  Clear it up.
        if (memManager && renderControl->sceneRenderer)
        {
            EAGLContext *oldContext = [EAGLContext currentContext];
            [renderControl->sceneRenderer useContext];

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
        renderControl->sceneRenderer.perfInterval = 100;
        [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];
    } else {
        renderControl->sceneRenderer.perfInterval = 0;
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];
    }
}

// Run every so often to dump out stats
- (void)periodicPerfOutput
{
    if (!renderControl || !renderControl->scene)
        return;
    
    renderControl->scene->dumpStats();
    [renderControl->interactLayer dumpStats];
    for (MaplyRemoteTileFetcher *tileFetcher : tileFetchers) {
        MaplyRemoteTileFetcherStats *stats = [tileFetcher getStats:false];
        [stats dump];
        [tileFetcher resetStats];
    }
    NSLog(@"Sampling layers: %lu",samplingLayers.size());
    
    [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];    
}

- (bool)performanceOutput
{
    return _performanceOutput;
}

// Build an array of lights and send them down all at once
- (void)updateLights
{
    [renderControl updateLights];
}

- (void)clearLights
{
    [renderControl clearLights];
}

- (void)resetLights
{
    [renderControl resetLights];
}

- (void)addLight:(MaplyLight *)light
{
    [renderControl addLight:light];
}

- (void)removeLight:(MaplyLight *)light
{
    [renderControl removeLight:light];
}

- (void)addShaderProgram:(MaplyShader *)shader sceneName:(NSString *)sceneName
{
    [renderControl addShaderProgram:shader sceneName:sceneName];
}

- (MaplyShader *)getShaderByName:(NSString *)name
{
    return [renderControl getShaderByName:name];
}

- (void)removeShaderProgram:(MaplyShader *__nonnull)shader
{
    [renderControl removeShaderProgram:shader];
}

#pragma mark - Defaults and descriptions

// Set new hints and update any related settings
- (void)setHints:(NSDictionary *)changeDict
{
    [renderControl setHints:changeDict];
}

- (bool) startOfWork
{
    return [renderControl startOfWork];
}

/// Called internally to end a block of work being done
- (void) endOfWork
{
    [renderControl endOfWork];
}

#pragma mark - Geometry related methods

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode;
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addScreenMarkers:markers desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addScreenMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (void)addClusterGenerator:(NSObject <MaplyClusterGenerator> *)clusterGen
{
    if (!renderControl)
        return;

    [renderControl->interactLayer addClusterGenerator:clusterGen];
}


- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addMarkers:markers desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addScreenLabels:labels desc:desc mode:threadMode];

    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addScreenLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addLabels:labels desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addVectors:vectors desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addVectors:vectors desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl instanceVectors:baseObj desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addWideVectors:vectors desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addWideVectors:vectors desc:desc mode:MaplyThreadAny];
}


- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addBillboards:billboards desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl->interactLayer addParticleSystem:partSys desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return compObj;
}

- (void)changeParticleSystem:(MaplyComponentObject *__nonnull)compObj renderTarget:(MaplyRenderTarget *__nullable)target
{
    if ([NSThread currentThread] != [NSThread mainThread]) {
        NSLog(@"MaplyBaseViewController: changeParticleSystem:renderTarget: must be called on main thread");
        return;
    }
    
    if (![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer changeParticleSystem:compObj renderTarget:target];

    [renderControl endOfWork];
}

- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode
{
    if (![batch isValid])
        return;
    if (!renderControl)
        return;
    
    if (![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer addParticleBatch:batch mode:threadMode];

    [renderControl endOfWork];
}

- (MaplyComponentObject *)addSelectionVectors:(NSArray *)vectors
{
    if (!renderControl)
        return nil;
    
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl->interactLayer addSelectionVectors:vectors desc:nil];
    [renderControl endOfWork];
    
    return compObj;
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return;
    
    [renderControl changeVector:compObj desc:desc mode:threadMode];
    
    [renderControl endOfWork];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc
{
    [self changeVector:compObj desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addShapes:shapes desc:desc mode:threadMode];

    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    return [self addShapes:shapes desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addModelInstances:modelInstances desc:desc mode:threadMode];

    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addGeometry:(NSArray *)geom desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addGeometry:geom desc:desc mode:threadMode];

    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addStickers:stickers desc:desc mode:threadMode];

    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    return [self addStickers:stickers desc:desc mode:MaplyThreadAny];
}

- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return;
    
    [renderControl changeSticker:compObj desc:desc mode:threadMode];

    [renderControl endOfWork];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;
    
    MaplyComponentObject *compObj = [renderControl addLoftedPolys:polys key:key cache:cacheDb desc:desc mode:threadMode];
    [renderControl endOfWork];
    
    return compObj;
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys key:(NSString *)key cache:(MaplyVectorDatabase *)cacheDb desc:(NSDictionary *)desc
{
    return [self addLoftedPolys:polys key:key cache:cacheDb desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addPoints:(NSArray *)points desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;

    MaplyComponentObject *compObj = [renderControl addPoints:points desc:desc mode:threadMode];
    [renderControl endOfWork];
    
    return compObj;
}

/// Add a view to track to a particular location
- (void)addViewTracker:(WGViewTracker *)viewTrack
{
    if (!renderControl)
        return;
    
    // Make sure we're not duplicating and add the object
    [self removeViewTrackForView:viewTrack.view];

    @synchronized(self)
    {
        [viewTrackers addObject:viewTrack];
    }
    
    // Hook it into the renderer
    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
    vpGen->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),Point2d(viewTrack.offset.x,viewTrack.offset.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    renderControl->sceneRenderer.triggerDraw = true;
    
    // And add it to the view hierarchy
    // Can only do this on the main thread anyway
    if ([viewTrack.view superview] == nil)
        [glView addSubview:viewTrack.view];
}

- (void)moveViewTracker:(MaplyViewTracker *)viewTrack moveTo:(MaplyCoordinate)newPos
{
    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();

    vpGen->moveView(GeoCoord(newPos.x,newPos.y),Point2d(0,0),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    renderControl->sceneRenderer.triggerDraw = true;
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
            ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
            vpGen->removeView(theTracker.view);
            if ([theTracker.view superview] == glView)
                [theTracker.view removeFromSuperview];
            renderControl->sceneRenderer.triggerDraw = true;
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
    if (!renderControl)
        return;
    
    // See if we're already representing the annotation
    bool alreadyHere = [annotations containsObject:annotate];
    
    // Let's put it in the right place so the callout can do its layout logic
    CGPoint pt = [self screenPointFromGeo:coord];

    // Fix for bad screen point return (courtesy highlander)
    if (isnan(pt.x) || isnan(pt.y))
        pt = CGPointMake(-2000.0,-2000.0);
    
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

    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
    if (alreadyHere)
    {
        vpGen->moveView(GeoCoord(coord.x,coord.y),Point2d(0,0),annotate.calloutView,annotate.minVis,annotate.maxVis);
    } else
    {
        vpGen->addView(GeoCoord(coord.x,coord.y),Point2d(0,0),annotate.calloutView,annotate.minVis,annotate.maxVis);
    }
    renderControl->sceneRenderer.triggerDraw = true;
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
    if (!renderControl)
        return;
    
    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
    vpGen->removeView(annotate.calloutView);
    
    [annotations removeObject:annotate];
    
    [annotate.calloutView dismissCalloutAnimated:YES];
}

- (void)freezeAnnotation:(MaplyAnnotation *)annotate
{
    if (!renderControl)
        return;
    
    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
    for (MaplyAnnotation *annotation in annotations)
        if (annotate == annotation)
        {
            vpGen->freezeView(annotate.calloutView);
        }
}

- (void)unfreezeAnnotation:(MaplyAnnotation *)annotate
{
    if (!renderControl)
        return;
    
    ViewPlacementGenerator *vpGen = renderControl->scene->getViewPlacementGenerator();
    for (MaplyAnnotation *annotation in annotations)
        if (annotate == annotation)
        {
            vpGen->unfreezeView(annotate.calloutView);
        }
    renderControl->sceneRenderer.triggerDraw = true;
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
    if (!renderControl || ![renderControl startOfWork])
        return nil;
    
    MaplyTexture *maplyTex = [renderControl->interactLayer addTexture:image desc:desc mode:threadMode];
    
    [renderControl endOfWork];
    
    return maplyTex;
}

- (MaplyTexture *__nullable)createTexture:(NSDictionary * _Nullable)inDesc sizeX:(int)sizeX sizeY:(int)sizeY mode:(MaplyThreadMode)threadMode
{
    if (![renderControl startOfWork])
        return nil;

    MaplyTexture *maplyTex = [renderControl createTexture:inDesc sizeX:sizeX sizeY:sizeY mode:threadMode];
    
    [renderControl endOfWork];
    
    return maplyTex;
}

- (void)removeTexture:(MaplyTexture *)texture mode:(MaplyThreadMode)threadMode
{
    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer removeTextures:@[texture] mode:threadMode];

    [renderControl endOfWork];
}

- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode
{
    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer removeTextures:textures mode:threadMode];

    [renderControl endOfWork];
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

- (void)addRenderTarget:(MaplyRenderTarget *)renderTarget
{
    if (!renderControl || ![renderControl startOfWork])
        return;

    [renderControl->interactLayer addRenderTarget:renderTarget];
    
    [renderControl endOfWork];
}

- (void)changeRenderTarget:(MaplyRenderTarget *)renderTarget tex:(MaplyTexture *)tex
{
    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer changeRenderTarget:renderTarget tex:tex];
    
    [renderControl endOfWork];
}

- (void)clearRenderTarget:(MaplyRenderTarget *)renderTarget mode:(MaplyThreadMode)threadMode
{
    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer clearRenderTarget:renderTarget mode:threadMode];

    [renderControl endOfWork];
}

- (void)removeRenderTarget:(MaplyRenderTarget *)renderTarget
{
    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer removeRenderTarget:renderTarget];
    
    [renderControl endOfWork];
}

- (void)setMaxLayoutObjects:(int)maxLayoutObjects
{
    LayoutManager *layoutManager = (LayoutManager *)renderControl->scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setMaxDisplayObjects(maxLayoutObjects);
}

- (void)runLayout
{
    [layoutLayer scheduleUpdateNow];
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

    // All objects must be MaplyComponentObject.  Yes, this happens.
    for (id obj in theObjs)
        if (![obj isKindOfClass:[MaplyComponentObject class]]) {
            NSLog(@"User passed an invalid objects into removeOjbects:mode:  All objects must be MaplyComponentObject.  Ignoring.");
            return;
        }

    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];

    [renderControl endOfWork];
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

    if (!renderControl || ![renderControl startOfWork])
        return;
    
    [renderControl->interactLayer disableObjects:theObjs mode:threadMode];

    [renderControl endOfWork];
}

- (void)enableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    if (!theObjs)
        return;

    if (!renderControl || ![renderControl startOfWork])
        return;

    [renderControl->interactLayer enableObjects:theObjs mode:threadMode];

    [renderControl endOfWork];
}

- (void)startChanges
{
    if (!renderControl || ![renderControl startOfWork])
        return;

    [renderControl->interactLayer startChanges];

    [renderControl endOfWork];
}

- (void)endChanges
{
    if (!renderControl || ![renderControl startOfWork])
        return;

    [renderControl->interactLayer endChanges];

    [renderControl endOfWork];
}

- (void)addActiveObject:(MaplyActiveObject *)theObj
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Must call addActiveObject: on the main thread.");
        return;
    }
    
    if (!renderControl)
        return;
    
    if (!activeObjects)
        activeObjects = [NSMutableArray array];

    if (![activeObjects containsObject:theObj])
    {
        renderControl->scene->addActiveModel(theObj);
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
    
    if (!renderControl)
        return;
    
    if ([activeObjects containsObject:theObj])
    {
        renderControl->scene->removeActiveModel(theObj);
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
    if (!renderControl)
        return false;
    
    if (newLayer && ![userLayers containsObject:newLayer])
    {
        WhirlyKitLayerThread *layerThread = baseLayerThread;
        // Only supporting quad image tiles layer for the thread per layer
        if (_threadPerLayer && ([newLayer isKindOfClass:[MaplyQuadImageTilesLayer class]] || [newLayer isKindOfClass:[MaplyQuadSamplingLayer class]]))
        {
            layerThread = [[WhirlyKitLayerThread alloc] initWithScene:renderControl->scene view:visualView renderer:renderControl->sceneRenderer mainLayerThread:false];
            [layerThreads addObject:layerThread];
            [layerThread start];
        }
        
        if ([newLayer startLayer:layerThread scene:renderControl->scene renderer:renderControl->sceneRenderer viewC:self])
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

- (void)removeLayer:(MaplyViewControllerLayer *)layer wait:(bool)wait
{
    if (!renderControl)
        return;
    
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
    [layer cleanupLayers:layerThread scene:renderControl->scene];
    [userLayers removeObject:layer];
    
    // Need to shut down the layer thread too
    if (layerThread != baseLayerThread)
    {
        if ([layerThreads containsObject:layerThread])
        {
            [layerThreads removeObject:layerThread];
            [layerThread addThingToRelease:theLayer];
            [layerThread cancel];

            if (wait) {
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
}

- (void)removeLayer:(MaplyViewControllerLayer *)layer
{
    [self removeLayer:layer wait:false];
}

- (void)removeLayers:(NSArray *)layers
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        [self performSelector:@selector(removeLayers:) withObject:layers];
        return;
    }

    for (MaplyViewControllerLayer *layer in layers)
        [self removeLayer:layer];
}

- (void)removeAllLayers
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        [self performSelector:@selector(removeAllLayers) withObject:nil];
        return;
    }

    NSArray *allLayers = [NSArray arrayWithArray:userLayers];
    
    for (MaplyViewControllerLayer *theLayer in allLayers)
        [self removeLayer:theLayer];
}

- (MaplyQuadSamplingLayer *)findSamplingLayer:(MaplySamplingParams *)params forUser:(NSObject<WhirlyKitQuadTileBuilderDelegate> *)userObj
{
    if (!renderControl)
        return nil;
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Caller called findSamplerLayer:forUser: off of main thread.");
        return nil;
    }

    // Look for a matching sampler
    for (auto layer : samplingLayers) {
        if ([layer.params isEqualTo:params]) {
            [layer addBuilderDelegate:userObj];
            return layer;
        }
    }
    
    // Create a new sampler
    MaplyQuadSamplingLayer *layer = [[MaplyQuadSamplingLayer alloc] initWithParams:params];
    [layer addBuilderDelegate:userObj];
    [self addLayer:layer];
    samplingLayers.push_back(layer);
    
    return layer;
}

- (void)releaseSamplingLayer:(MaplyQuadSamplingLayer *)layer forUser:(NSObject *)userObj
{
    if (!renderControl)
        return;
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Caller called findSamplerLayer:forUser: off of main thread.");
        return;
    }

    [layer removeBuilderDelegate:userObj];
    
    if (layer.numClients == 0) {
        [self removeLayer:layer wait:false];
        auto it = std::find(samplingLayers.begin(),samplingLayers.end(),layer);
        if (it != samplingLayers.end())
            samplingLayers.erase(it);
    }
}

- (MaplyRemoteTileFetcher *)addTileFetcher:(NSString *)name
{
    for (auto tileFetcher : tileFetchers)
        if ([tileFetcher.name isEqualToString:name])
            return tileFetcher;
    
    MaplyRemoteTileFetcher *tileFetcher = [[MaplyRemoteTileFetcher alloc] initWithName:name connections:tileFetcherConnections];
    tileFetchers.push_back(tileFetcher);
    
    return tileFetcher;
}

- (MaplyRemoteTileFetcher *)getTileFetcher:(NSString *)name
{
    for (auto tileFetcher : tileFetchers)
        if ([tileFetcher.name isEqualToString:name])
            return tileFetcher;
    
    return nil;
}

-(NSArray*)objectsAtCoord:(MaplyCoordinate)coord
{
    if (!renderControl)
        return nil;
    
    return [renderControl->interactLayer findVectorsInPoint:Point2f(coord.x,coord.y)];
}

#pragma mark - Properties

- (UIColor *)clearColor
{
    if (!renderControl)
        return nil;
    
    return renderControl->theClearColor;
}

- (void)setClearColor:(UIColor *)clearColor
{
    [renderControl setClearColor:clearColor];
    
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
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer.framebufferWidth,renderControl->sceneRenderer.framebufferHeight);
    if (frameSize.x() == 0)
        return MAXFLOAT;
    return (float)[visualView currentMapScale:frameSize];
}

- (float)heightForMapScale:(float)scale
{
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer.framebufferWidth,renderControl->sceneRenderer.framebufferHeight);
    if (frameSize.x() == 0)
        return -1.0;
    return (float)[visualView heightForMapScale:scale frame:frameSize];
}

- (UIImage *)snapshot
{
    if (!renderControl)
        return nil;
    
    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    renderControl->sceneRenderer.snapshotDelegate = target;
    
    [renderControl->sceneRenderer forceDrawNextFrame];
    [renderControl->sceneRenderer render:0.0];
    
    return target.image;
}

- (NSData *)shapshotRenderTarget:(MaplyRenderTarget *)renderTarget
{
    if ([NSThread currentThread] != [NSThread mainThread])
        return NULL;

    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    target.renderTargetID = renderTarget.renderTargetID;
    renderControl->sceneRenderer.snapshotDelegate = target;
    
    [renderControl->sceneRenderer forceDrawNextFrame];
    [renderControl->sceneRenderer render:0.0];
    
    return target.data;
}

- (float)currentMapZoom:(MaplyCoordinate)coordinate
{
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer.framebufferWidth,renderControl->sceneRenderer.framebufferHeight);
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

- (MaplyCoordinate3dD)displayCoordD:(MaplyCoordinate3dD)localCoord fromSystem:(MaplyCoordinateSystem *)coordSys
{
    Point3d loc3d = CoordSystemConvert3d(coordSys->coordSystem, visualView.coordAdapter->getCoordSystem(), Point3d(localCoord.x,localCoord.y,localCoord.z));
    Point3d pt = visualView.coordAdapter->localToDisplay(loc3d);
    
    MaplyCoordinate3dD ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    return ret;
}

- (BOOL)enable3dTouchSelection:(NSObject<Maply3dTouchPreviewDatasource>*)previewDataSource
{
    if (!renderControl)
        return false;
    
    if(previewingContext)
    {
        [self disable3dTouchSelection];
    }
    
    if([self respondsToSelector:@selector(traitCollection)] &&
       [self.traitCollection respondsToSelector:@selector(forceTouchCapability)] &&
       self.traitCollection.forceTouchCapability == UIForceTouchCapabilityAvailable)
    {
        previewTouchDelegate = [Maply3dTouchPreviewDelegate touchDelegate:self
                                                            interactLayer:renderControl->interactLayer
                                                               datasource:previewDataSource];
        previewingContext = [self registerForPreviewingWithDelegate:previewTouchDelegate
                                                         sourceView:self.view];
        return YES;
    }
    return NO;
}

- (void)disable3dTouchSelection {
    if(previewingContext)
    {
        [self unregisterForPreviewingWithContext:previewingContext];
        previewingContext = nil;
    }
}

- (void)requirePanGestureRecognizerToFailForGesture:(UIGestureRecognizer *__nullable)other {
    // Implement in derived class.
}


- (void)startLocationTrackingWithDelegate:(NSObject<MaplyLocationTrackerDelegate> *)delegate useHeading:(bool)useHeading useCourse:(bool)useCourse simulate:(bool)simulate {
    if (_locationTracker)
        [self stopLocationTracking];
    _locationTracker = [[MaplyLocationTracker alloc] initWithViewC:self delegate:delegate useHeading:useHeading useCourse:useCourse simulate:simulate];
}

- (void)changeLocationTrackingLockType:(MaplyLocationLockType)lockType {
    [self changeLocationTrackingLockType:lockType forwardTrackOffset:0];
}

- (void)changeLocationTrackingLockType:(MaplyLocationLockType)lockType forwardTrackOffset:(int)forwardTrackOffset {
    if (!_locationTracker)
        return;
    [_locationTracker changeLockType:lockType forwardTrackOffset:forwardTrackOffset];
}

- (void)stopLocationTracking {
    if (!_locationTracker)
        return;
    [_locationTracker teardown];
    _locationTracker = nil;
}

- (MaplyCoordinate)getDeviceLocation {
    if (!_locationTracker)
        return kMaplyNullCoordinate;
    return [_locationTracker getLocation];
}

- (CLLocationManager *)getTrackingLocationManager {
    if (!_locationTracker)
        return nil;
    return _locationTracker.locationManager;
}

- (float) getMaxLineWidth
{
    GLfloat width[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, &width[0]);
    
    return width[1];
}

-(NSArray *)loadedLayers
{
    return [NSArray arrayWithArray:userLayers];
}

- (MaplyRenderController * __nullable)getRenderControl
{
    return renderControl;
}

- (CGSize)getFramebufferSize
{
    if (!renderControl || !renderControl->sceneRenderer)
        return CGSizeZero;
    
    return CGSizeMake(renderControl->sceneRenderer.framebufferWidth,renderControl->sceneRenderer.framebufferHeight);
}

@end
