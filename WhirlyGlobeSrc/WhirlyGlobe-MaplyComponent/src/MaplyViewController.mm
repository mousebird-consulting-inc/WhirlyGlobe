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
#import "MaplyViewController_private.h"
#import "MaplyInteractionLayer_private.h"
#import "MaplyCoordinateSystem_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@interface MaplyViewController () <MaplyInteractionLayerDelegate>
@end

@implementation MaplyViewController
{
    // Content scale for scroll view mode
    float scale;
    bool scheduledToDraw;
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    _autoMoveToTap = true;
    
    return self;
}

- (id)initAsFlatMap
{
    self = [super init];
    if (!self)
        return nil;

<<<<<<< HEAD
=======
    // Turn off lighting
    [self setHints:@{kMaplyRendererLightingMode: @"none"}];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    _flatMode = true;
    
    return self;
}

- (id)initAsTetheredFlatMap:(UIScrollView *)inScrollView tetherView:(UIView *)inTetherView
{
    self = [self initAsFlatMap];
    if (!self)
        return nil;
    
    _tetheredMode = true;
    scrollView = inScrollView;
    _tetherView = inTetherView;
//    _tetherView = [[UIView alloc] init];
//    _tetherView.userInteractionEnabled = false;
//    [scrollView addSubview:_tetherView];
    scale = [UIScreen mainScreen].scale;
    
    // Watch changes to the tethered view.  The scroll view is making these, presumably.
    [scrollView addObserver:self forKeyPath:@"contentOffset" options:NSKeyValueObservingOptionNew context:NULL];
    
    return self;
}

- (void)resetTetheredFlatMap:(UIScrollView *)inScrollView tetherView:(UIView *)inTetherView
{
    [scrollView removeObserver:self forKeyPath:@"contentOffset"];

    scrollView = inScrollView;
    _tetherView = inTetherView;
    
    // Watch changes to the tethered view.  The scroll view is making these, presumably.
    [scrollView addObserver:self forKeyPath:@"contentOffset" options:NSKeyValueObservingOptionNew context:NULL];
}

// Tear down layers and layer thread
- (void) clear
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [self stopAnimation];
    
    [baseLayerThread addThingToDelete:coordAdapter];
    if (_coordSys)
        [baseLayerThread addThingToRelease:_coordSys];
    
    if (scrollView)
    {
        [scrollView removeObserver:self forKeyPath:@"contentOffset"];
        scrollView = nil;
        [_tetherView removeFromSuperview];
        _tetherView = nil;
    }

    [super clear];
    
    mapScene = NULL;
    mapView = nil;
    flatView = nil;

    mapInteractLayer = nil;
    
<<<<<<< HEAD
    // Note: Porting
//    tapDelegate = nil;
//    panDelegate = nil;
//    pinchDelegate = nil;
//    rotateDelegate = nil;
=======
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;
    rotateDelegate = nil;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    coordAdapter = NULL;
    _tetherView = NULL;
}

- (void)dealloc
{
    if (mapScene)
        [self clear];
}

// Change the view window and force a draw
- (void)setupFlatView
{
    if (!flatView)
        return;
    
    CGRect newFrame = _tetherView.frame;
//    NSLog(@"newFrame = (%f,%f)->(%f,%f)",newFrame.origin.x,newFrame.origin.y,newFrame.size.width,newFrame.size.height);
    // Change the flat view's window
    CGPoint contentOffset = scrollView.contentOffset;
<<<<<<< HEAD
    flatView->setWindowSize(Point2f(newFrame.size.width*scale,newFrame.size.height*scale),Point2f(contentOffset.x*scale,contentOffset.y*scale));
=======
    [flatView setWindowSize:Point2f(newFrame.size.width*scale,newFrame.size.height*scale) contentOffset:Point2f(contentOffset.x*scale,contentOffset.y*scale)];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    [glView drawView:self];
}

// Called when something changes the tethered view's frame
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
//    NSLog(@"Changed: %@",keyPath);

    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(setupFlatView) object:nil];
    [self performSelector:@selector(setupFlatView) withObject:nil afterDelay:0.0 inModes:[NSArray arrayWithObject:NSRunLoopCommonModes]];
}

- (void) loadSetup_lighting
{
<<<<<<< HEAD
=======
    if (![sceneRenderer isKindOfClass:[WhirlyKitSceneRendererES2 class]])
        return;
   
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    NSString *lightingType = hints[kWGRendererLightingMode];
    int lightingRegular = true;
    if ([lightingType respondsToSelector:@selector(compare:)])
        lightingRegular = [lightingType compare:@"none"];
    
<<<<<<< HEAD
    // Note: Porting
//    // Regular lighting is on by default
//    // We need to add a new shader to turn it off
//    if (!lightingRegular)
//    {
//        SimpleIdentity triNoLighting = scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
//        if (triNoLighting != EmptyIdentity)
//            scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
//        [sceneRenderer replaceLights:nil];
//    } else {
//        // Add a default light
//        MaplyLight *light = [[MaplyLight alloc] init];
//        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
//        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
//        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
//        light.viewDependent = false;
//        [self addLight:light];
//    }
=======
    // Regular lighting is on by default
    // We need to add a new shader to turn it off
    if (!lightingRegular)
    {
        SimpleIdentity triNoLighting = scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
        if (triNoLighting != EmptyIdentity)
            scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
        [sceneRenderer replaceLights:nil];
    } else {
        // Add a default light
        MaplyLight *light = [[MaplyLight alloc] init];
        light.pos = MaplyCoordinate3dMake(0.75, 0.5, -1.0);
        light.ambient = [UIColor colorWithRed:0.6 green:0.6 blue:0.6 alpha:1.0];
        light.diffuse = [UIColor colorWithRed:0.5 green:0.5 blue:0.5 alpha:1.0];
        light.viewDependent = false;
        [self addLight:light];
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    // We don't want the backface culling program for lines
    SimpleIdentity lineNoBackface = scene->getProgramIDByName(kToolkitDefaultLineNoBackfaceProgram);
    if (lineNoBackface)
        scene->setSceneProgram(kSceneDefaultLineShader, lineNoBackface);
}

<<<<<<< HEAD
- (WhirlyKit::View *) loadSetup_view
=======
- (WhirlyKitView *) loadSetup_view
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
{
    if (_coordSys)
    {
        MaplyCoordinate ll,ur;
        [_coordSys getBoundsLL:&ll ur:&ur];
        Point3d ll3d(ll.x,ll.y,0.0),ur3d(ur.x,ur.y,0.0);
        Point3d center3d(_displayCenter.x,_displayCenter.y,_displayCenter.z);
        coordAdapter = new GeneralCoordSystemDisplayAdapter([_coordSys getCoordSystem],ll3d,ur3d,center3d);
    } else
        coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
    
    if (scrollView)
    {
<<<<<<< HEAD
        flatView = new Maply::FlatView(coordAdapter);
        [self setupFlatView];
        mapView = flatView;
    } else {
        mapView = new Maply::MapView(coordAdapter);
        mapView->continuousZoom = true;
=======
        flatView = [[MaplyFlatView alloc] initWithCoordAdapter:coordAdapter];
        [self setupFlatView];
        mapView = flatView;
    } else {
        mapView = [[MaplyView alloc] initWithCoordAdapter:coordAdapter];
        mapView.continuousZoom = true;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    }    

    return mapView;
}

- (Scene *) loadSetup_scene
{
<<<<<<< HEAD
    mapScene = new Maply::MapScene(mapView->coordAdapter);
=======
    mapScene = new Maply::MapScene(mapView.coordAdapter);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    return mapScene;
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    mapInteractLayer = [[MaplyInteractionLayer alloc] initWithMapView:mapView];
    mapInteractLayer.viewController = self;
    return mapInteractLayer;
}

// Put together the objects we need to draw
- (void) loadSetup
{
    [super loadSetup];

    // The gl view won't spontaneously draw
    // Let's just do priority rendering
    if (scrollView)
    {
        glView.reactiveMode = true;
<<<<<<< HEAD
        sceneRenderer->setZBufferMode(zBufferOffDefault);
=======
        sceneRenderer.zBufferMode = zBufferOffDefault;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    }
    
    Point3f ll,ur;
    coordAdapter->getBounds(ll, ur);
    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();
    if (!_tetheredMode)
    {
        // Wire up the gesture recognizers
<<<<<<< HEAD
        // Note: Porting
//        tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
//        panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
//        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
//        pinchDelegate.minZoom = [mapView minHeightAboveSurface];
//        pinchDelegate.maxZoom = [mapView maxHeightAboveSurface];
//        rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:glView mapView:mapView];
    }

    [self setViewExtentsLL:boundLL ur:boundUR];

    // Turn off lighting
    [self setHints:@{kMaplyRendererLightingMode: @"none"}];
=======
        tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
        panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
        pinchDelegate.minZoom = [mapView minHeightAboveSurface];
        pinchDelegate.maxZoom = [mapView maxHeightAboveSurface];
        rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:glView mapView:mapView];
    }

    [self setViewExtentsLL:boundLL ur:boundUR];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
	
    [self registerForEvents];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    // Let's kick off a view update in case the renderer just got set up
<<<<<<< HEAD
    mapView->runViewUpdates();
=======
    [mapView runViewUpdates];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

- (void)viewWillDisappear:(BOOL)animated
{
    // This keeps us from ending the animation in tether mode
    // The display link will still be active, not much will happen
    if (!_tetheredMode)
        [super viewWillDisappear:animated];
    
	// Stop tracking notifications
<<<<<<< HEAD
    // Note: Porting
//    [[NSNotificationCenter defaultCenter] removeObserver:self name:MaplyTapMsg object:nil];
=======
    [[NSNotificationCenter defaultCenter] removeObserver:self name:MaplyTapMsg object:nil];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}


// Register for interesting tap events and others
// Note: Fill this in
- (void)registerForEvents
{
<<<<<<< HEAD
    // Note: Porting
//    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
=======
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

#pragma mark - Interaction

/// Return the view extents.  This is the box the view point is allowed to be within.
- (void)getViewExtentsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    *ll = boundLL;
    *ur = boundUR;
}

- (void)setHeight:(float)height
{
<<<<<<< HEAD
    Point3d loc = mapView->getLoc();
    loc.z() = height;
    mapView->setLoc(loc);
=======
    mapView.loc.z() = height;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur
{
<<<<<<< HEAD
    CoordSystemDisplayAdapter *adapter = mapView->coordAdapter;
=======
    CoordSystemDisplayAdapter *adapter = mapView.coordAdapter;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    CoordSystem *coordSys = adapter->getCoordSystem();
    boundLL = ll;    boundUR = ur;
    
    // Convert the bounds to a rectangle in local coordinates
    Point3f bounds3d[4];
    Point2f bounds[4];
    bounds3d[0] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ll.y)));
    bounds3d[1] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ll.y)));
    bounds3d[2] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ur.y)));
    bounds3d[3] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ur.y)));
    for (unsigned int ii=0;ii<4;ii++)
        bounds[ii] = Point2f(bounds3d[ii].x(),bounds3d[ii].y());
    
    if (flatView)
    {
<<<<<<< HEAD
        flatView->setExtents(Mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y)));
    }
    
    // Note: Porting
//    if (panDelegate)
//        [panDelegate setBounds:bounds];
//    if (pinchDelegate)
//        [pinchDelegate setBounds:bounds];
=======
        [flatView setExtents:Mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y))];
    }
    
    if (panDelegate)
        [panDelegate setBounds:bounds];
    if (pinchDelegate)
        [pinchDelegate setBounds:bounds];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

// Internal animation handler
- (void)animateToPoint:(Point3f)newLoc time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;
    
<<<<<<< HEAD
    mapView->cancelAnimation();

    // Note: Porting
//    MaplyAnimateViewTranslation *animTrans = [[MaplyAnimateViewTranslation alloc] initWithView:mapView translate:newLoc howLong:howLong];
//    curAnimation = animTrans;
//    mapView.delegate = animTrans;
=======
    [mapView cancelAnimation];

    MaplyAnimateViewTranslation *animTrans = [[MaplyAnimateViewTranslation alloc] initWithView:mapView translate:newLoc howLong:howLong];
    curAnimation = animTrans;
    mapView.delegate = animTrans;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;

    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;

<<<<<<< HEAD
    // Note: Porting
//    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
//    loc.z() = mapView.loc.z();
//    [self animateToPoint:loc time:howLong];
}

// Note: This may not work with a tilt
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)screenLoc time:(NSTimeInterval)howLong
=======
    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
    loc.z() = mapView.loc.z();
    [self animateToPoint:loc time:howLong];
}

// Note: This may not work with a tilt
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
{
    if (_tetheredMode)
        return;
    
    // Figure out where the point lands on the map
<<<<<<< HEAD
    Eigen::Matrix4d modelTrans = mapView->calcFullMatrix();
    Point3d whereLoc;
    Point2f frameSize = sceneRenderer->getFramebufferSize();
    Point2f screenLoc2f(screenLoc.x,screenLoc.y);
    if (mapView->pointOnPlaneFromScreen(screenLoc2f,&modelTrans,Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor),&whereLoc,true))
    {
        Point3d curLoc = mapView->getLoc();
        Point3f diffLoc(whereLoc.x()-curLoc.x(),whereLoc.y()-curLoc.y(),0.0);
        Point3f loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
        loc.x() -= diffLoc.x();
        loc.y() -= diffLoc.y();
        loc.z() = loc.z();
=======
    Eigen::Matrix4d modelTrans = [mapView calcFullMatrix];
    Point3d whereLoc;
    if ([mapView pointOnPlaneFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc clip:true])
    {
        Point3f diffLoc(whereLoc.x()-mapView.loc.x(),whereLoc.y()-mapView.loc.y(),0.0);
        Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
        loc.x() -= diffLoc.x();
        loc.y() -= diffLoc.y();
        loc.z() = mapView.loc.z();
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        [self animateToPoint:loc time:howLong];
    }
}

// This version takes a height as well
- (void)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;

<<<<<<< HEAD
    // Note: Porting
//    if (pinchDelegate)
//    {
//        if (newHeight < pinchDelegate.minZoom)
//            newHeight = pinchDelegate.minZoom;
//        if (newHeight > pinchDelegate.maxZoom)
//            newHeight = pinchDelegate.maxZoom;
//    }
=======
    if (pinchDelegate)
    {
        if (newHeight < pinchDelegate.minZoom)
            newHeight = pinchDelegate.minZoom;
        if (newHeight > pinchDelegate.maxZoom)
            newHeight = pinchDelegate.maxZoom;
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;

<<<<<<< HEAD
    Point3f loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
=======
    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    loc.z() = newHeight;
    
    [self animateToPoint:loc time:howLong];
}

// Only used for a flat view
- (void)animateToExtentsWindowSize:(CGSize)windowSize contentOffset:(CGPoint)contentOffset time:(NSTimeInterval)howLong
{
    if (!flatView)
        return;

    Point2f windowSize2f(windowSize.width,windowSize.height);
    Point2f contentOffset2f(contentOffset.x,contentOffset.y);
<<<<<<< HEAD
    // Note: Porting
//    MaplyAnimateFlat *animate = [[MaplyAnimateFlat alloc] initWithView:flatView destWindow:windowSize2f destContentOffset:contentOffset2f howLong:howLong];
//    curAnimation = animate;
//    flatView.delegate = animate;
=======
    MaplyAnimateFlat *animate = [[MaplyAnimateFlat alloc] initWithView:flatView destWindow:windowSize2f destContentOffset:contentOffset2f howLong:howLong];
    curAnimation = animate;
    flatView.delegate = animate;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    if (scrollView)
        return;

<<<<<<< HEAD
    Point3f loc = Vector3dToVector3f(mapView->getLoc());
=======
    Point3f loc = Vector3dToVector3f(mapView.loc);
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    [self setPosition:newPos height:loc.z()];
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    if (scrollView)
        return;

<<<<<<< HEAD
    mapView->cancelAnimation();

    // Note: Porting
//    if (pinchDelegate)
//    {
//        if (height < pinchDelegate.minZoom)
//            height = pinchDelegate.minZoom;
//        if (height > pinchDelegate.maxZoom)
//            height = pinchDelegate.maxZoom;
//    }
//    
=======
    [mapView cancelAnimation];
    
    if (pinchDelegate)
    {
        if (height < pinchDelegate.minZoom)
            height = pinchDelegate.minZoom;
        if (height > pinchDelegate.maxZoom)
            height = pinchDelegate.maxZoom;
    }
    
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;
    
<<<<<<< HEAD
    Point3d loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = height;
    mapView->setLoc(loc);
}

- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height
{
    Point3d loc = mapView->getLoc();
    GeoCoord geoCoord = mapView->coordAdapter->getCoordSystem()->localToGeographic(mapView->coordAdapter->displayToLocal(loc));
=======
    Point3d loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = height;
    mapView.loc = loc;
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    Point3d loc = mapView.loc;
    GeoCoord geoCoord = mapView.coordAdapter->getCoordSystem()->localToGeographic(mapView.coordAdapter->displayToLocal(loc));
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
    *height = loc.z();
}

- (void)setHeading:(float)heading
{
<<<<<<< HEAD
    mapView->setRotAngle(heading);
=======
    mapView.rotAngle = heading;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

- (float)heading
{
<<<<<<< HEAD
    return mapView->getRotAngle();
=======
    return mapView.rotAngle;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

/// Return the min and max heights above the globe for zooming
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight
{
<<<<<<< HEAD
    // Note: Porting
//    if (pinchDelegate)
//    {
//        *minHeight = pinchDelegate.minZoom;
//        *maxHeight = pinchDelegate.maxZoom;
//    }
=======
    if (pinchDelegate)
    {
        *minHeight = pinchDelegate.minZoom;
        *maxHeight = pinchDelegate.maxZoom;
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight
{
<<<<<<< HEAD
    // Note: Porting
//    if (pinchDelegate)
//    {
//        pinchDelegate.minZoom = minHeight;
//        pinchDelegate.maxZoom = maxHeight;
//        Point3d loc = mapView.loc;
//        if (mapView.heightAboveSurface < minHeight)
//            mapView.loc = Point3d(loc.x(),loc.y(),minHeight);
//        if (mapView.heightAboveSurface > maxHeight)
//            mapView.loc = Point3d(loc.x(),loc.y(),maxHeight);
//    }
=======
    if (pinchDelegate)
    {
        pinchDelegate.minZoom = minHeight;
        pinchDelegate.maxZoom = maxHeight;
        Point3d loc = mapView.loc;
        if (mapView.heightAboveSurface < minHeight)
            mapView.loc = Point3d(loc.x(),loc.y(),minHeight);
        if (mapView.heightAboveSurface > maxHeight)
            mapView.loc = Point3d(loc.x(),loc.y(),maxHeight);
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
<<<<<<< HEAD
    Point3d pt = visualView->coordAdapter->localToDisplay(visualView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = visualView->calcFullMatrix();
    Point2f frameSize = sceneRenderer->getFramebufferSize();
    Point2f retPt = mapView->pointOnScreenFromPlane(pt,&modelTrans,Point2f(frameSize.x()/glView.contentScaleFactor,frameSize.y()/glView.contentScaleFactor));
    return CGPointMake(retPt.x(),retPt.y());
}

// See if the given bounding box is all on sreen
- (bool)checkCoverage:(Mbr &)mbr mapView:(Maply::MapView *)theView height:(float)height
{
    Point3d loc = mapView->getLoc();
    Point3d testLoc = Point3d(loc.x(),loc.y(),height);
    mapView->setLoc(testLoc,false);
    
    Point2fVector pts;
=======
    Point3d pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = [visualView calcFullMatrix];
    return [mapView pointOnScreenFromPlane:pt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
}

// See if the given bounding box is all on sreen
- (bool)checkCoverage:(Mbr &)mbr mapView:(MaplyView *)theView height:(float)height
{
    Point3d loc = mapView.loc;
    Point3d testLoc = Point3d(loc.x(),loc.y(),height);
    [mapView setLoc:testLoc runUpdates:false];
    
    std::vector<Point2f> pts;
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    mbr.asPoints(pts);
    CGRect frame = self.view.frame;
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        Point2f pt = pts[ii];
        MaplyCoordinate geoCoord;
        geoCoord.x = pt.x();  geoCoord.y = pt.y();
        CGPoint screenPt = [self screenPointFromGeo:geoCoord];
        if (screenPt.x < 0 || screenPt.y < 0 || screenPt.x > frame.size.width || screenPt.y > frame.size.height)
            return false;
    }
    
    return true;
}

- (float)findHeightToViewBounds:(MaplyBoundingBox *)bbox pos:(MaplyCoordinate)pos
{
    
<<<<<<< HEAD
    Point3d oldLoc = mapView->getLoc();
    Point3d newLoc = Point3d(pos.x,pos.y,oldLoc.z());
    mapView->setLoc(newLoc,false);
=======
    Point3d oldLoc = mapView.loc;
    Point3d newLoc = Point3d(pos.x,pos.y,oldLoc.z());
    [mapView setLoc:newLoc runUpdates:false];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    // Note: Test
//    CGPoint testPt = [self screenPointFromGeo:pos];
    
    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    
<<<<<<< HEAD
    float minHeight = mapView->minHeightAboveSurface();
    float maxHeight = mapView->maxHeightAboveSurface();
    // Note: Porting
//    if (pinchDelegate)
//    {
//        minHeight = std::max(minHeight,pinchDelegate.minZoom);
//        maxHeight = std::min(maxHeight,pinchDelegate.maxZoom);
//    }
=======
    float minHeight = mapView.minHeightAboveSurface;
    float maxHeight = mapView.maxHeightAboveSurface;
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minZoom);
        maxHeight = std::min(maxHeight,pinchDelegate.maxZoom);
    }
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
    
    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr mapView:mapView height:minHeight];
    bool maxOnScreen = [self checkCoverage:mbr mapView:mapView height:maxHeight];
    if (!minOnScreen && !maxOnScreen)
    {
<<<<<<< HEAD
        mapView->setLoc(oldLoc,false);
=======
        [mapView setLoc:oldLoc runUpdates:false];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b
        return oldLoc.z();
    }
    
    // Now for the binary search
    // Note: I'd rather make a copy of the view first
    float minRange = 1e-5;
    do
    {
        float midHeight = (minHeight + maxHeight)/2.0;
        bool midOnScreen = [self checkCoverage:mbr mapView:mapView height:midHeight];
        
        if (!minOnScreen && midOnScreen)
        {
            maxHeight = midHeight;
            maxOnScreen = midOnScreen;
        } else if (!midOnScreen && maxOnScreen)
        {
            minHeight = midHeight;
            minOnScreen = midOnScreen;
        } else {
            // Not expecting this
            break;
        }
        
        if (maxHeight-minHeight < minRange)
            break;
    } while (true);
    
<<<<<<< HEAD
    mapView->setLoc(oldLoc,false);
=======
    [mapView setLoc:oldLoc runUpdates:false];
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

    return maxHeight;
}

<<<<<<< HEAD
// Note: Porting
//// Called back on the main thread after the interaction thread does the selection
//- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj
//{
//    MaplyCoordinate coord;
//    coord.x = msg.whereGeo.lon();
//    coord.y = msg.whereGeo.lat();
//
//    if (selectedObj && self.selection)
//    {
//        // The user selected something, so let the delegate know
//        if (_delegate)
//        {
//            if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:atLoc:onScreen:)])
//                [_delegate maplyViewController:self didSelect:selectedObj atLoc:coord onScreen:msg.touchLoc];
//            else if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
//                [_delegate maplyViewController:self didSelect:selectedObj];
//        }
//    } else {
//        // The user didn't select anything, let the delegate know.
//        if (_delegate)
//        {
//            if ([_delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
//                [_delegate maplyViewController:self didTapAt:coord];
//        }
//        if (_autoMoveToTap)
//            [self animateToPosition:coord time:1.0];
//    }
//}
//
//
//- (void)tapOnMap:(NSNotification *)note
//{
//    MaplyTapMessage *msg = note.object;
//    
//    // Hand this over to the interaction layer to look for a selection
//    // If there is no selection, it will call us back in the main thread
//    [mapInteractLayer userDidTap:msg];
//}
=======
// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    MaplyCoordinate coord;
    coord.x = msg.whereGeo.lon();
    coord.y = msg.whereGeo.lat();

    if (selectedObj && self.selection)
    {
        // The user selected something, so let the delegate know
        if (_delegate)
        {
            if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:atLoc:onScreen:)])
                [_delegate maplyViewController:self didSelect:selectedObj atLoc:coord onScreen:msg.touchLoc];
            else if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
                [_delegate maplyViewController:self didSelect:selectedObj];
        }
    } else {
        // The user didn't select anything, let the delegate know.
        if (_delegate)
        {
            if ([_delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
                [_delegate maplyViewController:self didTapAt:coord];
        }
        if (_autoMoveToTap)
            [self animateToPosition:coord time:1.0];
    }
}


- (void)tapOnMap:(NSNotification *)note
{
    MaplyTapMessage *msg = note.object;
    
    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [mapInteractLayer userDidTap:msg];
}
>>>>>>> 8b82d413fa1eea92c764cf2cc76045872be7384b

@end
