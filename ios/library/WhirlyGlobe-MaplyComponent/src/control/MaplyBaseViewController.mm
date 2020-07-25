/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 12/14/12.
 *  Copyright 2012-2019 mousebird consulting
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

#import "control/MaplyBaseViewController.h"
#import "MaplyBaseViewController_private.h"
#import "UIKit/NSData+Zlib.h"

#import "MaplyTexture_private.h"
#import "MaplyAnnotation_private.h"
#import "UIKit/NSDictionary+StyleRules.h"
#import "gestures/Maply3dTouchPreviewDelegate.h"
#import "MaplyTexture_private.h"
#import "MaplyRenderTarget_private.h"
#import "FontTextureManager_iOS.h"
#import "UIColor+Stuff.h"
#import "EAGLView.h"
#import "MTLView.h"
#import <sys/utsname.h>

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplySelectedObject
@end

// Target for screen snapshot
@interface SnapshotTarget : NSObject<WhirlyKitSnapshot>
@property (nonatomic,weak) MaplyBaseViewController *viewC;
@property (nonatomic) NSData *data;
@property (nonatomic) SimpleIdentity renderTargetID;
@property (nonatomic) CGRect subsetRect;
@property (nonatomic) NSObject<MaplySnapshotDelegate> *outsideDelegate;
@end

@implementation SnapshotTarget

- (instancetype)initWithViewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    
    _viewC = inViewC;
    _data = nil;
    _renderTargetID = EmptyIdentity;
    _subsetRect = CGRectZero;
    
    return self;
}

- (instancetype)initWithOutsideDelegate:(NSObject<MaplySnapshotDelegate> *)inOutsideDelegate viewC:(MaplyBaseViewController *)inViewC
{
    self = [super init];
    _outsideDelegate = inOutsideDelegate;
    
    return self;
}

- (void)setSubsetRect:(CGRect)subsetRect
{
    _subsetRect = subsetRect;
}

- (CGRect)snapshotRect
{
    if (_outsideDelegate)
        return [_outsideDelegate snapshotRect];
    
    return _subsetRect;
}

- (void)snapshotData:(NSData *)snapshotData {
    if (_outsideDelegate)
        [_outsideDelegate snapshot:snapshotData];
    else
        _data = snapshotData;
}

- (bool)needSnapshot:(NSTimeInterval)now {
    if (_outsideDelegate)
        return [_outsideDelegate needSnapshot:now viewC:_viewC];
    return true;
}

- (SimpleIdentity)renderTargetID
{
    if (_outsideDelegate) {
        MaplyRenderTarget *renderTarget = [_outsideDelegate renderTarget];
        if (renderTarget) {
            return [renderTarget renderTargetID];
        }
        return EmptyIdentity;
    }
    
    return _renderTargetID;
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
        
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(periodicPerfOutput) object:nil];

    [wrapView stopAnimation];
    
//    NSLog(@"BaseViewController: Shutting down layers");
    
    [wrapView teardown];
    
    [renderControl teardown];

    [renderControl->baseLayerThread addThingToRelease:wrapView];
    wrapView = nil;
    renderControl->scene = NULL;
    renderControl->sceneRenderer = NULL;
    
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

- (ViewRef) loadSetup_view
{
    return ViewRef(NULL);
}

- (void)loadSetup_glView
{
    if (_frameInterval <= 0)
        _frameInterval = 1;
    WhirlyKitEAGLView *glView = [[WhirlyKitEAGLView alloc] init];
    glView.frameInterval = _frameInterval;
    wrapView = glView;
}

- (void)loadSetup_mtlView
{
    SceneRendererMTL *renderMTL = (SceneRendererMTL *)renderControl->sceneRenderer.get();
    
    WhirlyKitMTLView *mtlView = [[WhirlyKitMTLView alloc] initWithDevice:((RenderSetupInfoMTL *) renderMTL->getRenderSetupInfo())->mtlDevice];
    wrapView = mtlView;
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    return [[MaplyBaseInteractionLayer alloc] initWithView:renderControl->visualView];
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
    if (!userID) {
        // This means we only send this information once
        // As a result we know nothing about the user, not even if they've run the app again
        userID = [[NSUUID UUID] UUIDString];
        [userDefaults setObject:userID forKey:@"wgmaplyanalyticuser"];

        [self sendAnalytics:@"analytics.mousebirdconsulting.com:8081"];
    }
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
    NSString *wgmaplyVersion = @"3.0";
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
        TimeInterval now = TimeGetCurrent();
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
    
    renderControl->renderType = _useOpenGLES ? SceneRenderer::RenderGLES : SceneRenderer::RenderMetal;
    
    allowRepositionForAnnnotations = true;
        
    [renderControl loadSetup];
    if (renderControl->renderType == SceneRenderer::RenderGLES)
    {
        [self loadSetup_glView];
    } else {
        [self loadSetup_mtlView];
    }
    
    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);
    if (sceneRenderGLES)
        sceneRenderGLES->setLayer((CAEAGLLayer *)wrapView.layer);

    // Set up the GL View to display it in
    [wrapView setRenderer:renderControl->sceneRenderer.get()];
    [self.view insertSubview:wrapView atIndex:0];
    self.view.backgroundColor = [UIColor blackColor];
    self.view.opaque = YES;
	self.view.autoresizesSubviews = YES;
	wrapView.frame = self.view.bounds;
    wrapView.backgroundColor = [UIColor blackColor];
        
    [renderControl loadSetup_view:[self loadSetup_view]];
    [renderControl loadSetup_scene:[self loadSetup_interactionLayer]];
    [self loadSetup_lighting];

    viewTrackers = [NSMutableArray array];
    annotations = [NSMutableArray array];
        
    // View placement manager
    viewPlacementModel = ViewPlacementActiveModelRef(new ViewPlacementActiveModel());
    renderControl->scene->addActiveModel(viewPlacementModel);
    
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

- (void)loadSetup_lighting
{
    [renderControl resetLights];
}

- (id<MTLDevice>)getMetalDevice
{
    if (!renderControl)
        return nil;
    
    return [renderControl getMetalDevice];
}

- (id<MTLLibrary>)getMetalLibrary
{
    if (!renderControl)
        return nil;
    
    return [renderControl getMetalLibrary];
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
    [wrapView startAnimation];
}

- (void)stopAnimation
{
    [wrapView stopAnimation];
}

- (void)teardown
{
    if (renderControl)
        [renderControl->interactLayer lockingShutdown];
    
    if (wrapView)
        [wrapView teardown];
    
    [self clear];
}

- (void)appBackground:(NSNotification *)note
{
    if(!wasAnimating || wrapView.isAnimating)
    {
        wasAnimating = wrapView.isAnimating;
        if (wasAnimating)
            [self stopAnimation];
    }
    
    if (!renderControl)
        return;
    for(WhirlyKitLayerThread *t in renderControl->layerThreads)
    {
        [t pause];
    }
}

- (void)appForeground:(NSNotification *)note
{
    if (!renderControl)
        return;

    for(WhirlyKitLayerThread *t in renderControl->layerThreads)
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

- (void)viewWillLayoutSubviews
{
    if (wrapView) {
        wrapView.frame = self.view.bounds;
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

- (void)setFrameInterval:(int)frameInterval
{
    _frameInterval = frameInterval;
    if ([wrapView isKindOfClass:[WhirlyKitEAGLView class]]) {
        WhirlyKitEAGLView *glView = (WhirlyKitEAGLView *)wrapView;
        glView.frameInterval = frameInterval;
    }
}

static const float PerfOutputDelay = 15.0;

- (void)setPerformanceOutput:(bool)performanceOutput
{
    if (_performanceOutput == performanceOutput)
        return;
    
    _performanceOutput = performanceOutput;
    if (_performanceOutput)
    {
        renderControl->sceneRenderer->setPerfInterval(100);
        [self performSelector:@selector(periodicPerfOutput) withObject:nil afterDelay:PerfOutputDelay];
    } else {
        renderControl->sceneRenderer->setPerfInterval(0);
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
    for (MaplyRemoteTileFetcher *tileFetcher : renderControl->tileFetchers) {
        MaplyRemoteTileFetcherStats *stats = [tileFetcher getStats:false];
        [stats dump];
        [tileFetcher resetStats];
    }
    NSLog(@"Sampling layers: %lu",renderControl->samplingLayers.size());
    
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

- (void)addShaderProgram:(MaplyShader *)shader
{
    [renderControl addShaderProgram:shader];
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
    MaplyComponentObject *compObj = [renderControl addScreenMarkers:markers desc:desc mode:threadMode];
    
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
    
    [renderControl addClusterGenerator:clusterGen];
}


- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addMarkers:markers desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    return [self addMarkers:markers desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addScreenLabels:labels desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addScreenLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addLabels:labels desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    return [self addLabels:labels desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addVectors:vectors desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addVectors:vectors desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)instanceVectors:(MaplyComponentObject *)baseObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl instanceVectors:baseObj desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addWideVectors:vectors desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addWideVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    return [self addWideVectors:vectors desc:desc mode:MaplyThreadAny];
}


- (MaplyComponentObject *)addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addBillboards:billboards desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addParticleSystem:(MaplyParticleSystem *)partSys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    return [renderControl addParticleSystem:partSys desc:desc mode:threadMode];
}

- (void)changeParticleSystem:(MaplyComponentObject *__nonnull)compObj renderTarget:(MaplyRenderTarget *__nullable)target
{
    return [renderControl changeParticleSystem:compObj renderTarget:target];
}

- (void)addParticleBatch:(MaplyParticleBatch *)batch mode:(MaplyThreadMode)threadMode
{
    [renderControl addParticleBatch:batch mode:threadMode];
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
    [renderControl changeVector:compObj desc:desc mode:threadMode];
}

- (void)changeVector:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc
{
    [self changeVector:compObj desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addShapes:shapes desc:desc mode:threadMode];

    return compObj;
}

- (MaplyComponentObject *)addShapes:(NSArray *)shapes desc:(NSDictionary *)desc
{
    return [self addShapes:shapes desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addModelInstances:(NSArray *)modelInstances desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addModelInstances:modelInstances desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addGeometry:(NSArray *)geom desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addGeometry:geom desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addStickers:stickers desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addStickers:(NSArray *)stickers desc:(NSDictionary *)desc
{
    return [self addStickers:stickers desc:desc mode:MaplyThreadAny];
}

- (void)changeSticker:(MaplyComponentObject *)compObj desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    [renderControl changeSticker:compObj desc:desc mode:threadMode];
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addLoftedPolys:polys desc:desc mode:threadMode];
    
    return compObj;
}

- (MaplyComponentObject *)addLoftedPolys:(NSArray *)polys desc:(NSDictionary *)desc
{
    return [self addLoftedPolys:polys desc:desc mode:MaplyThreadAny];
}

- (MaplyComponentObject *)addPoints:(NSArray *)points desc:(NSDictionary *)desc mode:(MaplyThreadMode)threadMode
{
    MaplyComponentObject *compObj = [renderControl addPoints:points desc:desc mode:threadMode];
    
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
    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        vpManage->addView(GeoCoord(viewTrack.loc.x,viewTrack.loc.y),Point2d(viewTrack.offset.x,viewTrack.offset.y),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    }
    renderControl->sceneRenderer->setTriggerDraw();
    
    // And add it to the view hierarchy
    // Can only do this on the main thread anyway
    if ([viewTrack.view superview] == nil)
        [wrapView addSubview:viewTrack.view];
}

- (void)moveViewTracker:(MaplyViewTracker *)viewTrack moveTo:(MaplyCoordinate)newPos
{
    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        vpManage->moveView(GeoCoord(newPos.x,newPos.y),Point2d(0,0),viewTrack.view,viewTrack.minVis,viewTrack.maxVis);
    }
    renderControl->sceneRenderer->setTriggerDraw();
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
            ViewPlacementManager *vpManage = viewPlacementModel->getManager();
            if (vpManage) {
                vpManage->removeView(theTracker.view);
            }
            if ([theTracker.view superview] == wrapView)
                [theTracker.view removeFromSuperview];
            renderControl->sceneRenderer->setTriggerDraw();
        }
    }
}

// Overridden by the subclasses
- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    return CGPointZero;
}

// Overridden by the subclasses
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(TimeInterval)howLong
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
        [annotate.calloutView presentCalloutFromRect:rect inView:wrapView constrainedToView:wrapView animated:YES];
    } else {
        annotate.calloutView.delegate = nil;
        [annotate.calloutView presentCalloutFromRect:rect inView:wrapView constrainedToView:wrapView animated:NO];
    }
    
    // But then we move it back because we're controlling its positioning
    CGRect frame = annotate.calloutView.frame;
    annotate.calloutView.frame = CGRectMake(frame.origin.x-pt.x+offset.x, frame.origin.y-pt.y+offset.y, frame.size.width, frame.size.height);

    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        if (alreadyHere)
        {
            vpManage->moveView(GeoCoord(coord.x,coord.y),Point2d(0,0),annotate.calloutView,annotate.minVis,annotate.maxVis);
        } else
        {
            vpManage->addView(GeoCoord(coord.x,coord.y),Point2d(0,0),annotate.calloutView,annotate.minVis,annotate.maxVis);
        }
    }
    renderControl->sceneRenderer->setTriggerDraw();
}

// Delegate callback for annotation placement
- (TimeInterval)calloutView:(SMCalloutView *)calloutView delayForRepositionWithSize:(CGSize)offset
{
    TimeInterval delay = 0.0;
    
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
    
    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        vpManage->removeView(annotate.calloutView);
    }
    
    [annotations removeObject:annotate];
    
    [annotate.calloutView dismissCalloutAnimated:YES];
}

- (void)freezeAnnotation:(MaplyAnnotation *)annotate
{
    if (!renderControl)
        return;
    
    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        for (MaplyAnnotation *annotation in annotations)
            if (annotate == annotation)
            {
                vpManage->freezeView(annotate.calloutView);
            }
    }
}

- (void)unfreezeAnnotation:(MaplyAnnotation *)annotate
{
    if (!renderControl)
        return;
    
    ViewPlacementManager *vpManage = viewPlacementModel->getManager();
    if (vpManage) {
        for (MaplyAnnotation *annotation in annotations)
            if (annotate == annotation)
            {
                vpManage->unfreezeView(annotate.calloutView);
            }
    }
    renderControl->sceneRenderer->setTriggerDraw();
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
    MaplyTexture *maplyTex = [renderControl addTexture:image desc:desc mode:threadMode];
    
    return maplyTex;
}

- (MaplyTexture *__nullable)addSubTexture:(MaplyTexture *__nonnull)tex xOffset:(int)x yOffset:(int)y width:(int)width height:(int)height mode:(MaplyThreadMode)threadMode
{
    MaplyTexture *maplyTex = [renderControl addSubTexture:tex xOffset:x yOffset:y width:width height:height mode:threadMode];
    
    return maplyTex;
}

- (MaplyTexture *__nullable)createTexture:(NSDictionary * _Nullable)inDesc sizeX:(int)sizeX sizeY:(int)sizeY mode:(MaplyThreadMode)threadMode
{
    MaplyTexture *maplyTex = [renderControl createTexture:inDesc sizeX:sizeX sizeY:sizeY mode:threadMode];
    
    return maplyTex;
}

- (void)removeTexture:(MaplyTexture *)texture mode:(MaplyThreadMode)threadMode
{
    [renderControl removeTextures:@[texture] mode:threadMode];
}

- (void)removeTextures:(NSArray *)textures mode:(MaplyThreadMode)threadMode
{
    [renderControl removeTextures:textures mode:threadMode];
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
    [renderControl addRenderTarget:renderTarget];
}

- (void)changeRenderTarget:(MaplyRenderTarget *)renderTarget tex:(MaplyTexture *)tex
{
    [renderControl changeRenderTarget:renderTarget tex:tex];
}

- (void)clearRenderTarget:(MaplyRenderTarget *)renderTarget mode:(MaplyThreadMode)threadMode
{
    [renderControl clearRenderTarget:renderTarget mode:threadMode];
}

- (void)removeRenderTarget:(MaplyRenderTarget *)renderTarget
{
    [renderControl removeRenderTarget:renderTarget];
}

- (void)setMaxLayoutObjects:(int)maxLayoutObjects
{
    LayoutManager *layoutManager = (LayoutManager *)renderControl->scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setMaxDisplayObjects(maxLayoutObjects);
}

- (void)setLayoutOverrideIDs:(NSArray *)uuids
{
    std::set<std::string> uuidSet;
    for (NSString *uuid in uuids) {
        std::string uuidStr = [uuid cStringUsingEncoding:NSASCIIStringEncoding];
        if (!uuidStr.empty())
            uuidSet.insert(uuidStr);
    }
    
    LayoutManager *layoutManager = (LayoutManager *)renderControl->scene->getManager(kWKLayoutManager);
    if (layoutManager)
        layoutManager->setOverrideUUIDs(uuidSet);
}

- (void)runLayout
{
    [renderControl runLayout];
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

    [renderControl removeObjects:[NSArray arrayWithArray:theObjs] mode:threadMode];
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

    [renderControl disableObjects:theObjs mode:threadMode];
}

- (void)enableObjects:(NSArray *)theObjs mode:(MaplyThreadMode)threadMode
{
    if (!theObjs)
        return;

    [renderControl enableObjects:theObjs mode:threadMode];
}

- (void)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID forObjects:(NSArray<MaplyComponentObject *> *__nonnull)compObjs mode:(MaplyThreadMode)threadMode
{
    if (!compObjs)
        return;

    [renderControl setUniformBlock:uniBlock buffer:bufferID forObjects:compObjs mode:threadMode];
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
        [wrapView setBackgroundColor:[UIColor clearColor]];
    }
}

- (MaplyCoordinate3d)displayPointFromGeo:(MaplyCoordinate)geoCoord
{
    MaplyCoordinate3d displayCoord = {0,0,0};
    if (!renderControl)
        return displayCoord;
    
    Point3f pt = renderControl->visualView->coordAdapter->localToDisplay(renderControl->visualView->coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
    
    displayCoord.x = pt.x();    displayCoord.y = pt.y();    displayCoord.z = pt.z();
    return displayCoord;
}

- (float)currentMapScale
{
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    if (frameSize.x() == 0)
        return MAXFLOAT;
    return (float)renderControl->visualView->currentMapScale(frameSize);
}

- (float)heightForMapScale:(float)scale
{
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    if (frameSize.x() == 0)
        return -1.0;
    return (float)renderControl->visualView->heightForMapScale(scale,frameSize);
}

- (void)addSnapshotDelegate:(NSObject<MaplySnapshotDelegate> *)snapshotDelegate
{
    if (!renderControl)
        return;
    
    SnapshotTarget *newTarget = [[SnapshotTarget alloc] initWithOutsideDelegate:snapshotDelegate viewC:self];
    switch ([self getRenderType])
    {
        case MaplyRenderGLES:
        {
            SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);
            sceneRenderGLES->addSnapshotDelegate(newTarget);
        }
            break;
        case MaplyRenderMetal:
        {
            SceneRendererMTLRef sceneRenderMTL = std::dynamic_pointer_cast<SceneRendererMTL>(renderControl->sceneRenderer);
            sceneRenderMTL->addSnapshotDelegate(newTarget);
        }
            break;
        default:
            break;
    }
}

- (void)removeSnapshotDelegate:(NSObject<MaplySnapshotDelegate> *)snapshotDelegate
{
    if (!renderControl)
        return;
    
    switch ([self getRenderType])
    {
        case MaplyRenderGLES:
        {
            SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);
            for (auto delegate : sceneRenderGLES->snapshotDelegates) {
                if ([delegate isKindOfClass:[SnapshotTarget class]]) {
                    SnapshotTarget *thisTarget = (SnapshotTarget *)delegate;
                    if (thisTarget.outsideDelegate == snapshotDelegate) {
                        sceneRenderGLES->removeSnapshotDelegate(thisTarget);
                        break;
                    }
                }
            }
        }
            break;
        case MaplyRenderMetal:
        {
            SceneRendererMTLRef sceneRenderMTL = std::dynamic_pointer_cast<SceneRendererMTL>(renderControl->sceneRenderer);
            for (auto delegate : sceneRenderMTL->snapshotDelegates) {
                if ([delegate isKindOfClass:[SnapshotTarget class]]) {
                    SnapshotTarget *thisTarget = (SnapshotTarget *)delegate;
                    if (thisTarget.outsideDelegate == snapshotDelegate) {
                        sceneRenderMTL->removeSnapshotDelegate(thisTarget);
                        break;
                    }
                }
            }
        }
            break;
        default:
            break;
    }
}

- (UIImage *)snapshot
{
    if (!renderControl)
        return nil;

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);
    if (!sceneRenderGLES)
        return nil;

    // TODO: Implement this for Metal

    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    sceneRenderGLES->addSnapshotDelegate(target);
    
    sceneRenderGLES->forceDrawNextFrame();
    sceneRenderGLES->render(0.0);
    
    sceneRenderGLES->removeSnapshotDelegate(target);
    
    // Courtesy: https://developer.apple.com/library/ios/qa/qa1704/_index.html
    // Create a CGImage with the pixel data
    // If your OpenGL ES content is opaque, use kCGImageAlphaNoneSkipLast to ignore the alpha channel
    // otherwise, use kCGImageAlphaPremultipliedLast
    CGDataProviderRef ref = CGDataProviderCreateWithData(NULL, [target.data bytes], [target.data length], NULL);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    int framebufferWidth = renderControl->sceneRenderer->framebufferWidth;
    int framebufferHeight = renderControl->sceneRenderer->framebufferHeight;
    CGImageRef iref = CGImageCreate(framebufferWidth, framebufferHeight, 8, 32, framebufferWidth * 4, colorspace, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast,
                                    ref, NULL, true, kCGRenderingIntentDefault);
    
    // OpenGL ES measures data in PIXELS
    // Create a graphics context with the target size measured in POINTS
    NSInteger widthInPoints, heightInPoints;
    {
        // On iOS 4 and later, use UIGraphicsBeginImageContextWithOptions to take the scale into consideration
        // Set the scale parameter to your OpenGL ES view's contentScaleFactor
        // so that you get a high-resolution snapshot when its value is greater than 1.0
        CGFloat scale = sceneRenderGLES->scale;
        widthInPoints = framebufferWidth / scale;
        heightInPoints = framebufferHeight / scale;
        UIGraphicsBeginImageContextWithOptions(CGSizeMake(widthInPoints, heightInPoints), NO, scale);
    }
    
    CGContextRef cgcontext = UIGraphicsGetCurrentContext();
    
    // UIKit coordinate system is upside down to GL/Quartz coordinate system
    // Flip the CGImage by rendering it to the flipped bitmap context
    // The size of the destination area is measured in POINTS
    CGContextSetBlendMode(cgcontext, kCGBlendModeCopy);
    CGContextDrawImage(cgcontext, CGRectMake(0.0, 0.0, widthInPoints, heightInPoints), iref);
    
    // Retrieve the UIImage from the current context
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    // Clean up
    CFRelease(ref);
    CFRelease(colorspace);
    CGImageRelease(iref);

    return image;
}

- (NSData *)shapshotRenderTarget:(MaplyRenderTarget *)renderTarget
{
    if ([NSThread currentThread] != renderControl->mainThread)
        return NULL;

    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    target.renderTargetID = renderTarget.renderTargetID;

    if ([self getRenderType] == MaplyRenderGLES) {
        SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);

        sceneRenderGLES->addSnapshotDelegate(target);
        sceneRenderGLES->forceDrawNextFrame();
        sceneRenderGLES->render(0.0);
        sceneRenderGLES->removeSnapshotDelegate(target);
    } else {
        SceneRendererMTLRef sceneRenderMTL = std::dynamic_pointer_cast<SceneRendererMTL>(renderControl->sceneRenderer);
        
        sceneRenderMTL->addSnapshotDelegate(target);
        sceneRenderMTL->forceDrawNextFrame();
        sceneRenderMTL->render(0.0, nil, nil);
        sceneRenderMTL->removeSnapshotDelegate(target);
    }
    
    return target.data;
}

- (NSData *)shapshotRenderTarget:(MaplyRenderTarget *)renderTarget rect:(CGRect)rect
{
    if ([NSThread currentThread] != renderControl->mainThread)
        return NULL;
    
    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderControl->sceneRenderer);
    if (!sceneRenderGLES)
        return nil;

    SnapshotTarget *target = [[SnapshotTarget alloc] init];
    target.renderTargetID = renderTarget.renderTargetID;
    target.subsetRect = rect;
    sceneRenderGLES->addSnapshotDelegate(target);
    
    sceneRenderGLES->forceDrawNextFrame();
    sceneRenderGLES->render(0.0);
    
    sceneRenderGLES->removeSnapshotDelegate(target);
    
    return target.data;
}


- (float)currentMapZoom:(MaplyCoordinate)coordinate
{
    if (!renderControl)
        return 0.0;
    
    Point2f frameSize(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    if (frameSize.x() == 0)
        return MAXFLOAT;
    return (float)renderControl->visualView->currentMapZoom(frameSize,coordinate.y);
}

- (MaplyCoordinateSystem *)coordSystem
{
    // Note: Hack.  Should wrap the real coordinate system
    MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return coordSys;
}

- (MaplyCoordinate3d)displayCoordFromLocal:(MaplyCoordinate3d)localCoord
{
    Point3d pt = renderControl->visualView->coordAdapter->localToDisplay(Point3d(localCoord.x,localCoord.y,localCoord.z));
    
    MaplyCoordinate3d ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    return ret;
}

- (MaplyCoordinate3d)displayCoord:(MaplyCoordinate3d)localCoord fromSystem:(MaplyCoordinateSystem *)coordSys
{
    Point3d loc3d = CoordSystemConvert3d(coordSys->coordSystem.get(), renderControl->visualView->coordAdapter->getCoordSystem(), Point3d(localCoord.x,localCoord.y,localCoord.z));
    Point3d pt = renderControl->visualView->coordAdapter->localToDisplay(loc3d);
    
    MaplyCoordinate3d ret;
    ret.x = pt.x();  ret.y = pt.y();  ret.z = pt.z();
    return ret;
}

- (MaplyCoordinate3dD)displayCoordD:(MaplyCoordinate3dD)localCoord fromSystem:(MaplyCoordinateSystem *)coordSys
{
    Point3d loc3d = CoordSystemConvert3d(coordSys->coordSystem.get(), renderControl->visualView->coordAdapter->getCoordSystem(), Point3d(localCoord.x,localCoord.y,localCoord.z));
    Point3d pt = renderControl->visualView->coordAdapter->localToDisplay(loc3d);
    
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

- (MaplyLocationTracker *)getLocationTracker
{
    return _locationTracker;
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
    return [NSArray arrayWithArray:renderControl->userLayers];
}

- (MaplyRenderController * __nullable)getRenderControl
{
    return renderControl;
}

- (CGSize)getFramebufferSize
{
    if (!renderControl || !renderControl->sceneRenderer)
        return CGSizeZero;
    
    return CGSizeMake(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
}

- (MaplyRenderType)getRenderType
{
    if (!renderControl || !renderControl->sceneRenderer)
        return MaplyRenderUnknown;
    
    switch (renderControl->sceneRenderer->getType())
    {
        case WhirlyKit::SceneRenderer::RenderGLES:
            return MaplyRenderGLES;
            break;
        case WhirlyKit::SceneRenderer::RenderMetal:
            return MaplyRenderMetal;
            break;
    }
    
    return MaplyRenderUnknown;
}

- (void)addActiveObject:(MaplyActiveObject *__nonnull)theObj
{
    if (!renderControl)
        return;
    
    [renderControl addActiveObject:theObj];
}

- (void)removeActiveObject:(MaplyActiveObject *__nonnull)theObj
{
    if (!renderControl)
        return;

    [renderControl removeActiveObject:theObj];
}

- (void)removeActiveObjects:(NSArray *__nonnull)theObjs
{
    if (!renderControl)
        return;

    [renderControl removeActiveObjects:theObjs];
}

- (bool)addLayer:(MaplyControllerLayer *__nonnull)layer
{
    if (!renderControl)
        return false;

    return [renderControl addLayer:layer];
}

- (void)removeLayer:(MaplyControllerLayer *__nonnull)layer
{
    if (!renderControl)
        return;

    [renderControl removeLayer:layer];
}

- (void)removeLayers:(NSArray *__nonnull)layers
{
    if (!renderControl)
        return;

    [renderControl removeLayers:layers];
}

- (void)removeAllLayers
{
    if (!renderControl)
        return;

    [renderControl removeAllLayers];
}

- (MaplyRemoteTileFetcher * __nonnull)addTileFetcher:(NSString * __nonnull)name
{
    if (!renderControl)
        return nil;

    return [renderControl addTileFetcher:name];
}

@end
