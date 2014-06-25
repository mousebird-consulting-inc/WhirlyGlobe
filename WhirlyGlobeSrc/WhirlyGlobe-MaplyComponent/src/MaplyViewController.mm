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
#import "MaplyAnnotation_private.h"

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
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;

    return self;
}

- (id)initAsFlatMap
{
    self = [super init];
    if (!self)
        return nil;

    // Turn off lighting
    [self setHints:@{kMaplyRendererLightingMode: @"none"}];
    _flatMode = true;
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;

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
    
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;
    rotateDelegate = nil;
    doubleTapDelegate = nil;
    twoFingerTapDelegate = nil;
    doubleTapDragDelegate = nil;
    
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
    [flatView setWindowSize:Point2f(newFrame.size.width*scale,newFrame.size.height*scale) contentOffset:Point2f(contentOffset.x*scale,contentOffset.y*scale)];
    
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

    // We don't want the backface culling program for lines
    SimpleIdentity lineNoBackface = scene->getProgramIDByName(kToolkitDefaultLineNoBackfaceProgram);
    if (lineNoBackface)
        scene->setSceneProgram(kSceneDefaultLineShader, lineNoBackface);
}

- (WhirlyKitView *) loadSetup_view
{
    if (_coordSys)
    {
        MaplyCoordinate ll,ur;
        [_coordSys getBoundsLL:&ll ur:&ur];
        Point3d ll3d(ll.x,ll.y,0.0),ur3d(ur.x,ur.y,0.0);
        Point3d center3d(_displayCenter.x,_displayCenter.y,_displayCenter.z);
        coordAdapter = new GeneralCoordSystemDisplayAdapter([_coordSys getCoordSystem],ll3d,ur3d,center3d);
    } else {
        coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
    }
    
    if (scrollView)
    {
        flatView = [[MaplyFlatView alloc] initWithCoordAdapter:coordAdapter];
        [self setupFlatView];
        mapView = flatView;
    } else {
        mapView = [[MaplyView alloc] initWithCoordAdapter:coordAdapter];
        mapView.continuousZoom = true;
        mapView.wrap = _viewWrap;
    }

    return mapView;
}

- (Scene *) loadSetup_scene
{
    mapScene = new Maply::MapScene(mapView.coordAdapter);

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
    
    allowRepositionForAnnnotations = false;

    // The gl view won't spontaneously draw
    // Let's just do priority rendering
    if (scrollView)
    {
        glView.reactiveMode = true;
        sceneRenderer.zBufferMode = zBufferOffDefault;
    }
    
    Point3f ll,ur;
    coordAdapter->getBounds(ll, ur);
    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();

    // Let them move E/W infinitely
    if (_viewWrap)
    {
        boundLL.x = -MAXFLOAT;
        boundUR.x = MAXFLOAT;
    }
    
    if (!_tetheredMode)
    {
        // Wire up the gesture recognizers
        tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
        panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
        pinchDelegate.minZoom = [mapView minHeightAboveSurface];
        pinchDelegate.maxZoom = [mapView maxHeightAboveSurface];
        if(_rotateGesture)
            rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:glView mapView:mapView];
        if(_doubleTapZoomGesture)
        {
            doubleTapDelegate = [MaplyDoubleTapDelegate doubleTapDelegateForView:glView mapView:mapView];
            doubleTapDelegate.minZoom = [mapView minHeightAboveSurface];
            doubleTapDelegate.maxZoom = [mapView maxHeightAboveSurface];
        }
        if(_twoFingerTapGesture)
        {
            twoFingerTapDelegate = [MaplyTwoFingerTapDelegate twoFingerTapDelegateForView:glView mapView:mapView];
            twoFingerTapDelegate.minZoom = [mapView minHeightAboveSurface];
            twoFingerTapDelegate.maxZoom = [mapView maxHeightAboveSurface];
            [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        }
        if (_doubleTapDragGesture)
        {
            doubleTapDragDelegate = [MaplyDoubleTapDragDelegate doubleTapDragDelegateForView:glView mapView:mapView];
            doubleTapDragDelegate.minZoom = [mapView minHeightAboveSurface];
            doubleTapDragDelegate.maxZoom = [mapView maxHeightAboveSurface];
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
            [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
    }

    [self setViewExtentsLL:boundLL ur:boundUR];
}

- (void)setViewWrap:(bool)viewWrap
{
    _viewWrap = viewWrap;
    
    if (!coordAdapter)
        return;

    Point3f ll,ur;
    coordAdapter->getBounds(ll, ur);
    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();
    
    // Let them move E/W infinitely
    if (_viewWrap)
    {
        boundLL.x = -MAXFLOAT;
        boundUR.x = MAXFLOAT;
    }
    [self setViewExtentsLL:boundLL ur:boundUR];
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
    [mapView runViewUpdates];
}

- (void)viewWillDisappear:(BOOL)animated
{
    // This keeps us from ending the animation in tether mode
    // The display link will still be active, not much will happen
    if (!_tetheredMode)
        [super viewWillDisappear:animated];
    
	// Stop tracking notifications
    [self unregisterForEvents];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}


// Register for interesting tap events and others
// Note: Fill this in
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidStart:) name:kPanDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidEnd:) name:kPanDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidStart:) name:kZoomGestureDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidEnd:) name:kZoomGestureDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidStart:) name:kMaplyDoubleTapDragDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidEnd:) name:kMaplyDoubleTapDragDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(animationDidEnd:) name:kWKViewAnimationEnded object:nil];
}

- (void)unregisterForEvents
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:MaplyTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPanDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPanDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kZoomGestureDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kZoomGestureDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kMaplyDoubleTapDragDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kMaplyDoubleTapDragDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWKViewAnimationEnded object:nil];
}

- (bool)panGesture
{
    return panDelegate.gestureRecognizer.enabled;
}


- (void)setPanGesture:(bool)enabled
{
    panDelegate.gestureRecognizer.enabled = enabled;
}

- (void)setRotateGesture:(bool)rotateGesture
{
    _rotateGesture = rotateGesture;
    if (rotateGesture)
    {
        if (!rotateDelegate)
        {
            rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:glView mapView:mapView];
        }
    } else {
        if (rotateDelegate)
        {
            UIRotationGestureRecognizer *rotRecog = nil;
            for (UIGestureRecognizer *recog in glView.gestureRecognizers)
                if ([recog isKindOfClass:[UIRotationGestureRecognizer class]])
                    rotRecog = (UIRotationGestureRecognizer *)recog;
           [glView removeGestureRecognizer:rotRecog];
           rotateDelegate = nil;
        }
    }
}

- (void)setDoubleTapZoomGesture:(bool)doubleTapZoomGesture
{
    _doubleTapZoomGesture = doubleTapZoomGesture;
    if (doubleTapZoomGesture)
    {
        if (!doubleTapDelegate)
        {
            doubleTapDelegate = [MaplyDoubleTapDelegate doubleTapDelegateForView:glView mapView:mapView];
            doubleTapDelegate.minZoom = [mapView minHeightAboveSurface];
            doubleTapDelegate.maxZoom = [mapView maxHeightAboveSurface];
        }
    } else {
        if (doubleTapDelegate)
        {
            [glView removeGestureRecognizer:doubleTapDelegate.gestureRecognizer];
            doubleTapDelegate.gestureRecognizer = nil;
            doubleTapDelegate = nil;
        }
    }
}

- (void)setTwoFingerTapGesture:(bool)twoFingerTapGesture
{
    _twoFingerTapGesture = twoFingerTapGesture;
    if (twoFingerTapGesture)
    {
        if (!twoFingerTapDelegate)
        {
            twoFingerTapDelegate = [MaplyTwoFingerTapDelegate twoFingerTapDelegateForView:glView mapView:mapView];
            twoFingerTapDelegate.minZoom = [mapView minHeightAboveSurface];
            twoFingerTapDelegate.maxZoom = [mapView maxHeightAboveSurface];
            if (pinchDelegate)
                [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        }
    } else {
        if (twoFingerTapDelegate)
        {
            [glView removeGestureRecognizer:twoFingerTapDelegate.gestureRecognizer];
            twoFingerTapDelegate.gestureRecognizer = nil;
            twoFingerTapDelegate = nil;
        }
    }
}

- (void)setDoubleTapDragGesture:(bool)doubleTapDragGesture
{
    _doubleTapZoomGesture = doubleTapDragGesture;
    if (doubleTapDragGesture)
    {
        if (!doubleTapDragDelegate)
        {
            doubleTapDragDelegate = [MaplyDoubleTapDragDelegate doubleTapDragDelegateForView:glView mapView:mapView];
            doubleTapDragDelegate.minZoom = [mapView minHeightAboveSurface];
            doubleTapDragDelegate.maxZoom = [mapView maxHeightAboveSurface];
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
            [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
    } else {
        if (doubleTapDragDelegate)
        {
            [glView removeGestureRecognizer:doubleTapDragDelegate.gestureRecognizer];
            doubleTapDragDelegate.gestureRecognizer = nil;
            doubleTapDragDelegate = nil;
        }
    }
}

#pragma mark - Interaction

/// Return the view extents.  This is the box the view point is allowed to be within.
- (void)getViewExtentsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    *ll = boundLL;
    *ur = boundUR;
}

- (float)height
{
    return mapView.loc.z();
}

- (void)setHeight:(float)height
{
    mapView.loc.z() = height;
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur
{
    CoordSystemDisplayAdapter *adapter = mapView.coordAdapter;
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
        [flatView setExtents:Mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y))];
    }
    
    if (panDelegate)
        [panDelegate setBounds:bounds];
    if (pinchDelegate)
        [pinchDelegate setBounds:bounds];
    if (doubleTapDelegate)
        [doubleTapDelegate setBounds:bounds];
}

// Internal animation handler
- (void)animateToPoint:(Point3f)newLoc time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;
    
    [mapView cancelAnimation];

    MaplyAnimateViewTranslation *animTrans = [[MaplyAnimateViewTranslation alloc] initWithView:mapView translate:newLoc howLong:howLong];
    curAnimation = animTrans;
    mapView.delegate = animTrans;
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

    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
    loc.z() = mapView.loc.z();
    [self animateToPoint:loc time:howLong];
}

// Note: This may not work with a tilt
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return false;
    
    // Figure out where the point lands on the map
    Eigen::Matrix4d modelTrans = [mapView calcFullMatrix];
    Point3d whereLoc;
    if ([mapView pointOnPlaneFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc clip:true])
    {
        Point3f diffLoc(whereLoc.x()-mapView.loc.x(),whereLoc.y()-mapView.loc.y(),0.0);
        Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
        loc.x() -= diffLoc.x();
        loc.y() -= diffLoc.y();
        loc.z() = mapView.loc.z();
        [self animateToPoint:loc time:howLong];
        return true;
    } else
        return false;
}

// This version takes a height as well
- (void)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;

    if (pinchDelegate)
    {
        if (newHeight < pinchDelegate.minZoom)
            newHeight = pinchDelegate.minZoom;
        if (newHeight > pinchDelegate.maxZoom)
            newHeight = pinchDelegate.maxZoom;
    }

    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;

    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
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
    MaplyAnimateFlat *animate = [[MaplyAnimateFlat alloc] initWithView:flatView destWindow:windowSize2f destContentOffset:contentOffset2f howLong:howLong];
    curAnimation = animate;
    flatView.delegate = animate;
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    if (scrollView)
        return;

    Point3f loc = Vector3dToVector3f(mapView.loc);
    [self setPosition:newPos height:loc.z()];
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    if (scrollView)
        return;

    [self handleStartMoving:NO];
    
    [mapView cancelAnimation];
    
    if (pinchDelegate)
    {
        if (height < pinchDelegate.minZoom)
            height = pinchDelegate.minZoom;
        if (height > pinchDelegate.maxZoom)
            height = pinchDelegate.maxZoom;
    }
    
    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;
    
    Point3d loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = height;
    mapView.loc = loc;
    [self handleStopMoving:NO];
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    Point3d loc = mapView.loc;
    GeoCoord geoCoord = mapView.coordAdapter->getCoordSystem()->localToGeographic(mapView.coordAdapter->displayToLocal(loc));
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
    *height = loc.z();
}

- (void)setHeading:(float)heading
{
    mapView.rotAngle = heading;
}

- (float)heading
{
    return mapView.rotAngle;
}

/// Return the min and max heights above the globe for zooming
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight
{
    if (pinchDelegate)
    {
        *minHeight = pinchDelegate.minZoom;
        *maxHeight = pinchDelegate.maxZoom;
    }
}

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight
{
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
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
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
    
    CGRect frame = self.view.frame;
    
    MaplyCoordinate ul;
    ul.x = mbr.ul().x();
    ul.y = mbr.ul().y();
    
    MaplyCoordinate lr;
    lr.x = mbr.lr().x();
    lr.y = mbr.lr().y();
    
    CGPoint ulScreen = [self screenPointFromGeo:ul];
    CGPoint lrScreen = [self screenPointFromGeo:lr];
    
    return std::abs(lrScreen.x - ulScreen.x) < frame.size.width && std::abs(lrScreen.y - ulScreen.y) < frame.size.height;
}

- (float)findHeightToViewBounds:(MaplyBoundingBox *)bbox pos:(MaplyCoordinate)pos
{
    Point3d oldLoc = mapView.loc;
    Point3d newLoc = Point3d(pos.x,pos.y,oldLoc.z());
    [mapView setLoc:newLoc runUpdates:false];
    
    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    
    float minHeight = mapView.minHeightAboveSurface;
    float maxHeight = mapView.maxHeightAboveSurface;
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minZoom);
        maxHeight = std::min(maxHeight,pinchDelegate.maxZoom);
    }
    
    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr mapView:mapView height:minHeight];
    bool maxOnScreen = [self checkCoverage:mbr mapView:mapView height:maxHeight];
    if (!minOnScreen && !maxOnScreen)
    {
        [mapView setLoc:oldLoc runUpdates:false];
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
            maxOnScreen = YES;
        } else if (!midOnScreen && maxOnScreen) {
            minHeight = midHeight;
            minOnScreen = NO;
        } else {
            // Not expecting this
            break;
        }
        
    } while (maxHeight-minHeight > minRange);
    
    //set map back to pre-search state
    [mapView setLoc:oldLoc runUpdates:false];

    return maxHeight;
}

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

// Called when the pan delegate starts moving
- (void) panDidStart:(NSNotification *)note
{
    if (note.object != mapView)
        return;
    
    //    NSLog(@"Pan started");
    
    [self handleStartMoving:true];
}

// Called when the pan delegate stops moving
- (void) panDidEnd:(NSNotification *)note
{
    if (note.object != mapView)
        return;
    
    [self handleStopMoving:true];
}

- (void) zoomGestureDidStart:(NSNotification *)note
{
    if (note.object != mapView)
        return;

    [self handleStartMoving:true];
}

- (void) zoomGestureDidEnd:(NSNotification *)note
{
    if (note.object != mapView)
        return;
    
    [self handleStopMoving:true];
}

- (void) animationDidEnd:(NSNotification *)note
{
    if (note.object != mapView)
        return;

    [self handleStopMoving:NO];
}

// Convenience routine to handle the end of moving
- (void)handleStartMoving:(bool)userMotion
{
    if([self.delegate respondsToSelector:@selector(maplyViewControllerDidStartMoving:userMotion:)])
    {
        [self.delegate maplyViewControllerDidStartMoving:self userMotion:userMotion];
    }
}

// Convenience routine to handle the end of moving
- (void)handleStopMoving:(bool)userMotion
{
    if([self.delegate respondsToSelector:@selector(maplyViewController:didStopMoving:userMotion:)])
    {
        MaplyCoordinate corners[4];
        [self corners:corners];
        [self.delegate maplyViewController:self didStopMoving:corners userMotion:userMotion];
    }
}

- (MaplyCoordinate)geoFromScreenPoint:(CGPoint)point {
  	Point3d hit;
    WhirlyKitSceneRendererES *sceneRender = glView.renderer;
    Eigen::Matrix4d theTransform = [mapView calcFullMatrix];
    if ([mapView pointOnPlaneFromScreen:point transform:&theTransform frameSize:Point2f(sceneRender.framebufferWidth/glView.contentScaleFactor,sceneRender.framebufferHeight/glView.contentScaleFactor) hit:&hit clip:true])
    {
        Point3d localPt = coordAdapter->displayToLocal(hit);
		GeoCoord coord = coordAdapter->getCoordSystem()->localToGeographic(localPt);
        MaplyCoordinate maplyCoord;
        maplyCoord.x = coord.x();
        maplyCoord.y  = coord.y();
        return maplyCoord;
    } else {
        return MaplyCoordinateMakeWithDegrees(0, 0);
    }
}


- (void)corners:(MaplyCoordinate *)corners
{
    CGPoint screenCorners[4];
    screenCorners[0] = CGPointMake(0.0, 0.0);
    screenCorners[1] = CGPointMake(sceneRenderer.framebufferWidth,0.0);
    screenCorners[2] = CGPointMake(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    screenCorners[3] = CGPointMake(0.0, sceneRenderer.framebufferHeight);
    
    for (unsigned int ii=0;ii<4;ii++)
    {
        corners[ii] = [self geoFromScreenPoint:screenCorners[ii]];
    }
}

- (MaplyBoundingBox) getCurrentExtents
{

    MaplyBoundingBox bbox;
    
    CGRect frame = self.view.frame;
    
    CGPoint pt = CGPointMake(0,frame.size.height);
    bbox.ll = [self geoFromScreenPoint:pt];
    
    pt = CGPointMake(frame.size.width,0);
    bbox.ur = [self geoFromScreenPoint:pt];
    
    return bbox;
    
}

- (void)calloutViewClicked:(SMCalloutView *)calloutView
{
    if([self.delegate respondsToSelector:@selector(maplyViewController:didTapAnnotation:)]) {
        for(MaplyAnnotation *annotation in self.annotations) {
            if(annotation.calloutView == calloutView) {
                [self.delegate maplyViewController:self didTapAnnotation:annotation];
                return;
            }
        }
    }

    // Note: Old version of name
    if([self.delegate respondsToSelector:@selector(maplyViewController:didClickAnnotation:)]) {
        for(MaplyAnnotation *annotation in self.annotations) {
            if(annotation.calloutView == calloutView) {
                [self.delegate maplyViewController:self didClickAnnotation:annotation];
                return;
            }
        }
    }
}



@end
