/*
 *  MaplyViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
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

#import <WhirlyGlobe_iOS.h>
#import "MaplyViewController.h"
#import "MaplyViewController_private.h"
#import "MaplyInteractionLayer_private.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyAnnotation_private.h"
#import "MaplyAnimateTranslateMomentum.h"
#import "GlobeView_iOS.h"
#import "SceneGLES.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@implementation MaplyViewControllerAnimationState

- (instancetype)init
{
    self = [super init];
    _heading = DBL_MAX;
    _height = 1.0;
    _pos.x = _pos.y = 0.0;
    _screenPos = {-1,-1};
    
    return self;
}

+ (MaplyViewControllerAnimationState *)Interpolate:(double)t from:(MaplyViewControllerAnimationState *)stateA to:(MaplyViewControllerAnimationState *)stateB
{
    MaplyViewControllerAnimationState *newState = [[MaplyViewControllerAnimationState alloc] init];
    
    newState.heading = (stateB.heading-stateA.heading)*t + stateA.heading;
    newState.height = (stateB.height-stateA.height)*t + stateA.height;
    newState.pos = MaplyCoordinateDMake((stateB.pos.x-stateA.pos.x)*t + stateA.pos.x,(stateB.pos.y-stateA.pos.y)*t + stateA.pos.y);
    if (stateA.screenPos.x >= 0.0 && stateA.screenPos.y >= 0.0 &&
        stateB.screenPos.x >= 0.0 && stateB.screenPos.y >= 0.0)
    {
        newState.screenPos = CGPointMake((stateB.screenPos.x - stateA.screenPos.x)*t + stateA.screenPos.x,(stateB.screenPos.y - stateA.screenPos.y)*t + stateA.screenPos.y);
    } else
        newState.screenPos = stateB.screenPos;
    
    return newState;
}

- (BOOL) isEqual:(id)object
{
    if (![object isKindOfClass:[MaplyViewControllerAnimationState class]])
        return false;
    MaplyViewControllerAnimationState *otherState = object;
    
    return self.pos.x == otherState.pos.x && self.pos.y == otherState.pos.y &&
    self.heading == otherState.heading && self.height == otherState.height &&
    self.screenPos.x == otherState.screenPos.x && self.screenPos.y == otherState.screenPos.y;
}

@end


@implementation MaplyViewControllerSimpleAnimationDelegate
{
    MaplyViewControllerAnimationState *startState;
    MaplyViewControllerAnimationState *endState;
    TimeInterval startTime,endTime;
}

- (instancetype)initWithState:(MaplyViewControllerAnimationState *)inEndState
{
    self = [super init];
    endState = inEndState;
    
    return self;
}

- (void)mapViewController:(MaplyViewController *__nonnull)viewC startState:(MaplyViewControllerAnimationState *__nonnull)inStartState startTime:(TimeInterval)inStartTime endTime:(TimeInterval)inEndTime
{
    startState = inStartState;
    if (!endState)
    {
        endState = [[MaplyViewControllerAnimationState alloc] init];
        endState.heading = _heading;
        endState.height = _height;
        endState.pos = _loc;
    }
    startTime = inStartTime;
    endTime = inEndTime;
}

- (nonnull MaplyViewControllerAnimationState *)mapViewController:(MaplyViewController *__nonnull)viewC stateForTime:(TimeInterval)currentTime
{
    MaplyViewControllerAnimationState *state = [[MaplyViewControllerAnimationState alloc] init];
    double t = (currentTime-startTime)/(endTime-startTime);
    if (t < 0.0)  t = 0.0;
    if (t > 1.0)  t = 1.0;
    
    float dHeading = endState.heading - startState.heading;
    if (ABS(dHeading) <= M_PI)
        state.heading = (dHeading)*t + startState.heading;
    else if (dHeading > 0)
        state.heading = (dHeading - 2.0*M_PI)*t + startState.heading;
    else
        state.heading = (dHeading + 2.0*M_PI)*t + startState.heading;
    
    state.height = (endState.height - startState.height)*t + startState.height;
    MaplyCoordinateD pos;
    pos.x = (endState.pos.x - startState.pos.x)*t + startState.pos.x;
    pos.y = (endState.pos.y - startState.pos.y)*t + startState.pos.y;
    state.pos = pos;
    
    return state;
}

- (void)mapViewControllerDidFinishAnimation:(MaplyViewController *__nonnull)viewC
{
}

@end



@interface MaplyViewController () <MaplyInteractionLayerDelegate>
- (void)updateView:(Maply::MapView *)theMapView;
- (void)viewUpdated:(View *)view;
@end

// Interface object between Obj-C and C++ for animation callbacks
// Also used to catch view geometry updates
class MaplyViewControllerAnimationWrapper : public Maply::MapViewAnimationDelegate, public ViewWatcher
{
public:
    MaplyViewControllerAnimationWrapper()
    : control(nil)
    {
    }
    
    // Called by the View to set up view state per frame
    void updateView(Maply::MapView *mapView)
    {
        [control updateView:mapView];
    }
    
    // Called by the view when things are changed
    virtual void viewUpdated(View *view)
    {
        [control viewUpdated:view];
    }
    
public:
    MaplyViewController __weak * control;
};

@implementation MaplyViewController
{
    // Content scale for scroll view mode
    float scale;
    bool scheduledToDraw;
    bool isPanning,isZooming,isAnimating;
    
    /// Boundary quad that we're to stay within, in display coords
    Point2dVector bounds;
    Point2d bounds2d[4];
    
    MaplyViewControllerAnimationWrapper animWrapper;
}

- (instancetype)initWithMapType:(MaplyMapType)mapType
{
	self = [super init];
	if (!self)
		return nil;
    animWrapper.control = self;

	if (mapType == MaplyMapType3D) {
		_autoMoveToTap = true;
	}
	else {
		// Turn off lighting
		[self setHints:@{kMaplyRendererLightingMode: @"none"}];
	}

    _pinchGesture = true;
	_rotateGesture = true;
	_doubleTapDragGesture = true;
	_twoFingerTapGesture = true;
	_doubleTapZoomGesture = true;
//    self.useOpenGLES = true;

	return self;
}

- (instancetype)init
{
    self = [super init];
    if (!self)
        return nil;
    animWrapper.control = self;

    _autoMoveToTap = true;
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;
//    self.useOpenGLES = true;

    return self;
}

- (instancetype)initAsFlatMap
{
    self = [super init];
    if (!self)
        return nil;
    animWrapper.control = self;

    // Turn off lighting
    [self setHints:@{kMaplyRendererLightingMode: @"none"}];
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;

    return self;
}

// Tear down layers and layer thread
- (void) clear
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    [self stopAnimation];
    
    if (_coordSys)
        [renderControl->baseLayerThread addThingToRelease:_coordSys];
    
    [super clear];
    
    mapScene = NULL;
    mapView = nil;
    wrapView = nil;

    mapInteractLayer = nil;
    
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;
    rotateDelegate = nil;
    doubleTapDelegate = nil;
    twoFingerTapDelegate = nil;
    doubleTapDragDelegate = nil;
    
    coordAdapter = NULL;
    
    delegateRespondsToViewUpdate = false;
}

- (void)dealloc
{
}

- (void)setDelegate:(NSObject<MaplyViewControllerDelegate> *)delegate
{
    _delegate = delegate;
    delegateRespondsToViewUpdate = [_delegate respondsToSelector:@selector(maplyViewController:didMove:)];
}

// Called by the globe view when something changes
- (void)viewUpdated:(View *)view
{
    if (delegateRespondsToViewUpdate)
    {
        MaplyCoordinate corners[4];
        [self corners:corners];
        [_delegate maplyViewController:self didMove:corners];
    }
}

- (void) loadSetup_lighting
{
    NSString *lightingType = renderControl->hints[kWGRendererLightingMode];
    int lightingRegular = true;
    if ([lightingType respondsToSelector:@selector(compare:)])
        lightingRegular = [lightingType compare:@"none"];
    
    // Regular lighting is on by default
    // We need to add a new shader to turn it off
    if (!lightingRegular)
    {
        // Note: Porting
//        SimpleIdentity triNoLighting = renderControl->scene->getProgramIDByName(kToolkitDefaultTriangleNoLightingProgram);
//        if (triNoLighting != EmptyIdentity)
//            renderControl->scene->setSceneProgram(kSceneDefaultTriShader, triNoLighting);
        std::vector<DirectionalLight> lights;
        renderControl->sceneRenderer->replaceLights(lights);
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

- (ViewRef) loadSetup_view
{
    if (_coordSys)
    {
        MaplyCoordinate ll,ur;
        [_coordSys getBoundsLL:&ll ur:&ur];
        Point3d ll3d(ll.x,ll.y,0.0),ur3d(ur.x,ur.y,0.0);
        // May need to scale this to the space we're expecting
        double scaleFactor = 1.0;
        if (std::abs(ur.x-ll.x) > 10.0 || std::abs(ur.y-ll.y) > 10.0)
        {
            Point3d diff = ur3d - ll3d;
            scaleFactor = 4.0/std::max(diff.x(),diff.y());
        }
        Point3d center3d(_displayCenter.x,_displayCenter.y,_displayCenter.z);
        GeneralCoordSystemDisplayAdapter *genCoordAdapter = new GeneralCoordSystemDisplayAdapter([_coordSys getCoordSystem].get(),ll3d,ur3d,center3d,Point3d(scaleFactor,scaleFactor,scaleFactor));
        coordAdapter = CoordSystemDisplayAdapterRef(genCoordAdapter);
    } else {
        coordAdapter = CoordSystemDisplayAdapterRef(new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0)));
    }
    
    mapView = Maply::MapView_iOSRef(new Maply::MapView_iOS(coordAdapter.get()));
    mapView->continuousZoom = true;
    mapView->setWrap(_viewWrap);
    mapView->addWatcher(&animWrapper);

    return mapView;
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
    
    Point3f ll,ur;
    coordAdapter->getBounds(ll, ur);
    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();

    // Let them move E/W infinitely, but only with the default coordinate system
    if (_viewWrap && !_coordSys)
    {
        boundLL.x = -MAXFLOAT;
        boundUR.x = MAXFLOAT;
    }
    
    // Wire up the gesture recognizers
    tapDelegate = [MaplyTapDelegate tapDelegateForView:wrapView mapView:mapView.get()];
    panDelegate = [MaplyPanDelegate panDelegateForView:wrapView mapView:mapView.get() useCustomPanRecognizer:nil];
    if (_pinchGesture)
    {
        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:wrapView mapView:mapView];
        pinchDelegate.minZoom = mapView->minHeightAboveSurface();
        pinchDelegate.maxZoom = mapView->maxHeightAboveSurface();
    }
    if(_rotateGesture)
        rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:wrapView mapView:mapView.get()];
    if(_doubleTapZoomGesture)
    {
        doubleTapDelegate = [MaplyDoubleTapDelegate doubleTapDelegateForView:wrapView mapView:mapView];
        doubleTapDelegate.minZoom = mapView->minHeightAboveSurface();
        doubleTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDelegate.gestureRecognizer];
    }
    if(_twoFingerTapGesture)
    {
        twoFingerTapDelegate = [MaplyTwoFingerTapDelegate twoFingerTapDelegateForView:wrapView mapView:mapView];
        twoFingerTapDelegate.minZoom = mapView->minHeightAboveSurface();
        twoFingerTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
        if (pinchDelegate)
            [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:twoFingerTapDelegate.gestureRecognizer];
    }
    if (_doubleTapDragGesture)
    {
        doubleTapDragDelegate = [MaplyDoubleTapDragDelegate doubleTapDragDelegateForView:wrapView mapView:mapView];
        doubleTapDragDelegate.minZoom = mapView->minHeightAboveSurface();
        doubleTapDragDelegate.maxZoom = mapView->maxHeightAboveSurface();
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
    }
    if(_cancelAnimationOnTouch)
    {
        touchDelegate = [MaplyTouchCancelAnimationDelegate touchDelegateForView:wrapView mapView:mapView.get()];
    }

    [self setViewExtentsLL:boundLL ur:boundUR];
}

- (void)handleLongPress:(UIGestureRecognizer*)sender {
    if(sender.state == UIGestureRecognizerStateBegan) {
        mapView->cancelAnimation();
    }
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
    
    mapView->setWrap(_viewWrap);
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
    mapView->runViewUpdates();
}

- (void)viewWillDisappear:(BOOL)animated
{
    // This keeps us from ending the animation in tether mode
    // The display link will still be active, not much will happen
    [super viewWillDisappear:animated];
    
	// Stop tracking notifications
    [self unregisterForEvents];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    // Note: This is experimental for now
    return;
    
//    dispatch_async(dispatch_get_main_queue(),
//    ^{
//        bool newCoordValid = false;
//        MaplyCoordinate newCoord;
//        double newHeight = 0.0;
//
//        // Let's rerun the view constrants if we have them, because things can move around
//        Point3d newCenter;
//        MaplyView *thisMapView = [[MaplyView alloc] initWithView:mapView];
//        bool valid = [self withinBounds:mapView.loc view:wrapView renderer:renderControl->sceneRenderer mapView:thisMapView newCenter:&newCenter];
//        if (valid)
//        {
//            if (mapView.loc.x() != newCenter.x() || mapView.loc.y() != newCenter.y())
//            {
//                GeoCoord geoCoord = coordAdapter->getCoordSystem()->localToGeographic(newCenter);
//                newCoord = {geoCoord.x(),geoCoord.y()};
//                newHeight = self.height;
//            }
//        } else {
//            // Mess with the height
//            MaplyBoundingBox bbox = [self getViewExtents];
//            if (bbox.ll.x < bbox.ur.x)
//            {
//                MaplyCoordinate coord;  coord.x = (bbox.ll.x+bbox.ur.x)/2.0;  coord.y = (bbox.ll.y+bbox.ur.y)/2.0;
//                float testHeight = [self findHeightToViewBounds:bbox pos:coord];
//                if (testHeight != self.height)
//                {
//                    Point3d newLoc(coord.x,coord.y,testHeight);
//                    Point3d newNewCenter;
//                    bool valid = [self withinBounds:newLoc view:wrapView renderer:renderControl->sceneRenderer mapView:thisMapView newCenter:&newNewCenter];
//
//                    newCoordValid = true;
//                    newHeight = testHeight;
//                    if (valid)
//                    {
//                        newCoord = {coord.x,coord.y};
//                    } else {
//                        GeoCoord geoCoord = coordAdapter->getCoordSystem()->localToGeographic(newNewCenter);
//                        newCoord = {geoCoord.x(),geoCoord.y()};
//                    }
//                }
//            }
//        }
//
//        if (newCoordValid)
//            [self setPosition:newCoord height:newHeight];
//    });
}

// Register for interesting tap events and others
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidStart:) name:kPanDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidEnd:) name:kPanDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidStart:) name:kZoomGestureDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidEnd:) name:kZoomGestureDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidStart:) name:kMaplyDoubleTapDragDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(zoomGestureDidEnd:) name:kMaplyDoubleTapDragDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(animationDidStart:) name:kWKViewAnimationStarted object:nil];
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
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWKViewAnimationStarted object:nil];
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

- (void)setPinchGesture:(bool)pinchGesture
{
    _pinchGesture = pinchGesture;
    if (pinchGesture)
    {
        if (!pinchDelegate)
        {
            pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:wrapView mapView:mapView];
            pinchDelegate.minZoom = mapView->minHeightAboveSurface();
            pinchDelegate.maxZoom = mapView->maxHeightAboveSurface();
            
            if (twoFingerTapDelegate)
                [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        }
    } else {
        if (pinchDelegate)
        {
            [wrapView removeGestureRecognizer:pinchDelegate.gestureRecognizer];
            pinchDelegate = nil;
        }
    }
}

- (void)setRotateGesture:(bool)rotateGesture
{
    _rotateGesture = rotateGesture;
    if (rotateGesture)
    {
        if (!rotateDelegate)
        {
            rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:wrapView mapView:mapView.get()];
        }
    } else {
        if (rotateDelegate)
        {
            UIRotationGestureRecognizer *rotRecog = nil;
            for (UIGestureRecognizer *recog in wrapView.gestureRecognizers)
                if ([recog isKindOfClass:[UIRotationGestureRecognizer class]])
                    rotRecog = (UIRotationGestureRecognizer *)recog;
           [wrapView removeGestureRecognizer:rotRecog];
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
            doubleTapDelegate = [MaplyDoubleTapDelegate doubleTapDelegateForView:wrapView mapView:mapView];
            doubleTapDelegate.minZoom = mapView->minHeightAboveSurface();
            doubleTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
        }
    } else {
        if (doubleTapDelegate)
        {
            [wrapView removeGestureRecognizer:doubleTapDelegate.gestureRecognizer];
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
            twoFingerTapDelegate = [MaplyTwoFingerTapDelegate twoFingerTapDelegateForView:wrapView mapView:mapView];
            twoFingerTapDelegate.minZoom = mapView->minHeightAboveSurface();
            twoFingerTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
            if (pinchDelegate)
                [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        }
    } else {
        if (twoFingerTapDelegate)
        {
            [wrapView removeGestureRecognizer:twoFingerTapDelegate.gestureRecognizer];
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
            doubleTapDragDelegate = [MaplyDoubleTapDragDelegate doubleTapDragDelegateForView:wrapView mapView:mapView];
            doubleTapDragDelegate.minZoom = mapView->minHeightAboveSurface();
            doubleTapDragDelegate.maxZoom = mapView->maxHeightAboveSurface();
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
            [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
    } else {
        if (doubleTapDragDelegate)
        {
            [wrapView removeGestureRecognizer:doubleTapDragDelegate.gestureRecognizer];
            doubleTapDragDelegate.gestureRecognizer = nil;
            doubleTapDragDelegate = nil;
        }
    }
}

- (void)setCancelAnimationOnTouch:(bool)cancelAnimationOnTouch
{
    _cancelAnimationOnTouch = cancelAnimationOnTouch;
    if(cancelAnimationOnTouch)
    {
        if(!touchDelegate)
        {
            touchDelegate = [MaplyTouchCancelAnimationDelegate touchDelegateForView:wrapView mapView:mapView.get()];
        }
    } else {
        [wrapView removeGestureRecognizer:touchDelegate.gestureRecognizer];
        touchDelegate.gestureRecognizer = nil;
        touchDelegate = nil;
    }
}

#pragma mark - Interaction

/// Return the view extents.  This is the box the view point is allowed to be within.
- (MaplyBoundingBox)getViewExtents
{
	MaplyBoundingBox box;

	box.ll = boundLL;
	box.ur = boundUR;

	return box;
}

/// Return the view extents.  This is the box the view point is allowed to be within.
- (void)getViewExtentsLL:(MaplyCoordinate *)ll ur:(MaplyCoordinate *)ur
{
    *ll = boundLL;
    *ur = boundUR;
}

- (float)height
{
    return mapView->getLoc().z();
}

- (void)setHeight:(float)height
{
    Point3d loc = mapView->getLoc();
    loc.z() = height;
    mapView->setLoc(loc, true);
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtents:(MaplyBoundingBox)box
{
	[self setViewExtentsLL:box.ll ur:box.ur];
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur
{
    CoordSystemDisplayAdapter *adapter = mapView->coordAdapter;
    CoordSystem *coordSys = adapter->getCoordSystem();
    boundLL = ll;    boundUR = ur;
    
    // Convert the bounds to a rectangle in local coordinates
    Point3f bounds3d[4];
    bounds3d[0] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ll.y)));
    bounds3d[1] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ll.y)));
    bounds3d[2] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ur.y)));
    bounds3d[3] = adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ur.y)));
    bounds.clear();
    for (unsigned int ii=0;ii<4;ii++)
    {
        bounds2d[ii] = Point2d(bounds3d[ii].x(),bounds3d[ii].y());
        bounds.push_back(bounds2d[ii]);
    }
    
    if (panDelegate)
        [panDelegate setBounds:bounds2d];
    if (pinchDelegate)
        [pinchDelegate setBounds:bounds2d];
    if (doubleTapDelegate)
        [doubleTapDelegate setBounds:bounds2d];
    if (twoFingerTapDelegate)
        [twoFingerTapDelegate setBounds:bounds2d];
    if (doubleTapDragDelegate)
        [doubleTapDragDelegate setBounds:bounds2d];
}

// Internal animation handler
- (void)animateToPoint:(Point3d)newLoc time:(TimeInterval)howLong
{
    mapView->cancelAnimation();
    
    AnimateViewTranslationRef anim = AnimateViewTranslationRef(new AnimateViewTranslation(mapView,renderControl->sceneRenderer.get(),newLoc,howLong));
    anim->userMotion = false;
    anim->setBounds(bounds2d);

    curAnimation = anim;
    mapView->setDelegate(anim);
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(TimeInterval)howLong
{
    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;

    Point3d loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = mapView->getLoc().z();
    [self animateToPoint:loc time:howLong];
}

// Note: This may not work with a tilt
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(TimeInterval)howLong
{
    if (!renderControl)
        return false;
    
    // Figure out where the point lands on the map
    Eigen::Matrix4d modelTrans = mapView->calcFullMatrix();
    Point3d whereLoc;
    Point2f loc2f(loc.x,loc.y);
    auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
    if (mapView->pointOnPlaneFromScreen(loc2f, &modelTrans, frameSizeScaled, &whereLoc, true))
    {
        Point3d oldLoc = mapView->getLoc();
        Point3f diffLoc(whereLoc.x()-oldLoc.x(),whereLoc.y()-oldLoc.y(),0.0);
        Point3d loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
        loc.x() -= diffLoc.x();
        loc.y() -= diffLoc.y();
        loc.z() = oldLoc.z();
        [self animateToPoint:loc time:howLong];
        return true;
    } else
        return false;
}

// This version takes a height as well
- (void)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight time:(TimeInterval)howLong
{
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

    Point3d loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = newHeight;
    
    [self animateToPoint:loc time:howLong];
}

- (bool)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight heading:(float)newHeading time:(TimeInterval)howLong
{
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"MaplyViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    
    mapView->cancelAnimation();
    
    MaplyViewControllerSimpleAnimationDelegate *anim = [[MaplyViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = MaplyCoordinateDMakeWithMaplyCoordinate(newPos);
    anim.heading = newHeading;
    anim.height = newHeight;
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

- (bool)animateToPositionD:(MaplyCoordinateD)newPos height:(double)newHeight heading:(double)newHeading time:(TimeInterval)howLong
{
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"MaplyViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    
    mapView->cancelAnimation();
    
    MaplyViewControllerSimpleAnimationDelegate *anim = [[MaplyViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = newPos;
    anim.heading = newHeading;
    anim.height = newHeight;
    
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc height:(float)newHeight heading:(float)newHeading time:(TimeInterval)howLong {
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"MaplyViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    if (!renderControl)
        return false;
    
    mapView->cancelAnimation();

    // save current view state
    MaplyViewControllerAnimationState *curState = [self getViewState];
    
    // temporarily change view state, without propagating updates, to find offset coordinate
    MaplyViewControllerAnimationState *nextState = [[MaplyViewControllerAnimationState alloc] init];
    nextState.heading = newHeading;
    nextState.pos = MaplyCoordinateDMakeWithMaplyCoordinate(newPos);
    nextState.height = newHeight;
    [self setViewStateInternal:nextState runViewUpdates:false];
    
    // find offset coordinate
    CGPoint invPoint = CGPointMake(self.view.frame.size.width/2+loc.x, self.view.frame.size.height/2+loc.y);
    MaplyCoordinate geoCoord = [self geoFromScreenPoint:invPoint];
    
    // check if within bounds
    nextState.pos = MaplyCoordinateDMakeWithMaplyCoordinate(geoCoord);
    [self setViewStateInternal:nextState runViewUpdates:false];
    Point3d newCenter;
    Point3d oldLoc = mapView->getLoc();
    bool valid = [self withinBounds:oldLoc view:wrapView renderer:renderControl->sceneRenderer.get() mapView:mapView.get() newCenter:&newCenter];
    
    // restore current view state
    [self setViewStateInternal:curState runViewUpdates:false];

    if (valid)
    {
        // animate to offset coordinate
        MaplyViewControllerSimpleAnimationDelegate *anim = [[MaplyViewControllerSimpleAnimationDelegate alloc] init];
        anim.loc = MaplyCoordinateDMakeWithMaplyCoordinate(geoCoord);
        anim.heading = newHeading;
        anim.height = newHeight;
        [self animateWithDelegate:anim time:howLong];
    }
    
    return valid;
}

// Bounds check on a single point
- (bool)withinBounds:(Point3d &)loc view:(UIView *)view renderer:(SceneRenderer *)sceneRender mapView:(Maply::MapView *)testMapView newCenter:(Point3d *)newCenter
{
    if (bounds.empty())
        return true;
    
    return MaplyGestureWithinBounds(bounds,loc,sceneRender,testMapView,newCenter);
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    Point3f loc = Vector3dToVector3f(mapView->getLoc());
    [self setPosition:newPos height:loc.z()];
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    if (!renderControl)
        return;

    [self handleStartMoving:NO];
    
    mapView->cancelAnimation();
    
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
    
    Point3d loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = height;

    // Do a validity check and possibly adjust the center
    Maply::MapView testMapView(*(mapView.get()));
    Point3d newCenter;
    if ([self withinBounds:loc view:wrapView renderer:renderControl->sceneRenderer.get() mapView:&testMapView newCenter:&newCenter])
    {
        mapView->setLoc(newCenter);
    }

    [self handleStopMoving:NO];
}

- (MaplyCoordinate)getPosition
{
	GeoCoord geoCoord = mapView->coordAdapter->getCoordSystem()->localToGeographic(mapView->coordAdapter->displayToLocal(mapView->getLoc()));

	return {.x = geoCoord.x(), .y = geoCoord.y()};
}

- (float)getHeight
{
	return mapView->getLoc().z();
}

- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height
{
    Point3d loc = mapView->getLoc();
    GeoCoord geoCoord = mapView->coordAdapter->getCoordSystem()->localToGeographic(mapView->coordAdapter->displayToLocal(loc));
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
    *height = loc.z();
}

- (void)setViewState:(MaplyViewControllerAnimationState *)animState
{
    [self setViewState:animState runViewUpdates:true];
}

- (void)setViewState:(MaplyViewControllerAnimationState *)animState runViewUpdates:(bool)runViewUpdates
{
    mapView->cancelAnimation();
    [self setViewStateInternal:animState runViewUpdates:runViewUpdates];
}

- (void)setViewStateInternal:(MaplyViewControllerAnimationState *)animState
{
    [self setViewStateInternal:animState runViewUpdates:true];
}

- (void)setViewStateInternal:(MaplyViewControllerAnimationState *)animState runViewUpdates:(bool)runViewUpdates
{
    if (!renderControl)
        return;
    
    Point3f loc = mapView->coordAdapter->localToDisplay(mapView->coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(animState.pos.x, animState.pos.y)));
    loc.z() = animState.height;

    if (animState.screenPos.x >= 0.0 && animState.screenPos.y >= 0.0)
    {
        Eigen::Matrix4d modelTrans = mapView->calcFullMatrix();
        Point3d hit;
        auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
        Point2f screenPos2f(animState.screenPos.x,animState.screenPos.y);
        if (mapView->pointOnPlaneFromScreen(screenPos2f, &modelTrans, frameSizeScaled, &hit, true))
        {
            Point3d oldLoc = mapView->getLoc();
            Point3f diffLoc(hit.x()-oldLoc.x(),hit.y()-oldLoc.y(),0.0);
            loc.x() -= diffLoc.x();
            loc.y() -= diffLoc.y();

        }
    }
    Point3d loc3d = loc.cast<double>();
    mapView->setLoc(loc3d, false);
    mapView->setRotAngle(-animState.heading, runViewUpdates);
}

- (MaplyViewControllerAnimationState *)getViewState {
    MaplyViewControllerAnimationState *state = [[MaplyViewControllerAnimationState alloc] init];
    state.heading = -mapView->getRotAngle();
    MaplyCoordinate pos;
    float height;
    [self getPosition:&pos height:&height];
    state.pos = MaplyCoordinateDMakeWithMaplyCoordinate(pos);
    state.height = height;
    return state;
}

- (void)applyConstraintsToViewState:(MaplyViewControllerAnimationState *)viewState
{
    if (pinchDelegate)
    {
        if (viewState.height < pinchDelegate.minZoom)
            viewState.height = pinchDelegate.minZoom;
        if (viewState.height > pinchDelegate.maxZoom)
            viewState.height = pinchDelegate.maxZoom;
    }
}


- (void)animateWithDelegate:(NSObject<MaplyViewControllerAnimationDelegate> *)inAnimationDelegate time:(TimeInterval)howLong
{
    TimeInterval now = mapScene->getCurrentTime();
    animationDelegate = inAnimationDelegate;
    animationDelegateEnd = now+howLong;
    
    MaplyViewControllerAnimationState *stateStart = [self getViewState];
    // Tell the delegate what we're up to
    [animationDelegate mapViewController:self startState:stateStart startTime:now endTime:animationDelegateEnd];
    
    MaplyViewControllerAnimationWrapper *delegate = new MaplyViewControllerAnimationWrapper();
    delegate->control = self;
    mapView->setDelegate(MapViewAnimationDelegateRef(delegate));
}

// Called every frame from within the map view
- (void)updateView:(Maply::MapView *)theMapView
{
    if (!renderControl)
        return;
    
    TimeInterval now = mapScene->getCurrentTime();
    if (!animationDelegate)
    {
        theMapView->cancelAnimation();
        return;
    }
    
    bool lastOne = false;
    if (now > animationDelegateEnd)
        lastOne = true;
    
    // Current view state
    MaplyViewControllerAnimationState *oldState = [self getViewState];

    // Ask the delegate where we're supposed to be
    MaplyViewControllerAnimationState *animState = [animationDelegate mapViewController:self stateForTime:now];
    
    // Do a validity check and possibly adjust the center
    Point3d loc = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y));
    loc.z() = animState.height;
    Maply::MapView testMapView(*(mapView.get()));
    Point3d newCenter;
    if ([self withinBounds:loc view:wrapView renderer:renderControl->sceneRenderer.get() mapView:&testMapView newCenter:&newCenter])
    {
        GeoCoord geoCoord = coordAdapter->getCoordSystem()->localToGeographic(newCenter);
        animState.pos = {geoCoord.x(),geoCoord.y()};
        animState.height = newCenter.z();
        
        if (![oldState isEqual:animState])
            [self setViewStateInternal:animState];
    } else
        lastOne = true;

    if (lastOne)
    {
        theMapView->cancelAnimation();
        if ([animationDelegate respondsToSelector:@selector(mapViewControllerDidFinishAnimation:)])
            [animationDelegate mapViewControllerDidFinishAnimation:self];
        animationDelegate = nil;
    }
}

- (void)setHeading:(float)heading
{
    mapView->setRotAngle(heading,true);
}

- (float)heading
{
    return mapView->getRotAngle();
}

- (float)getMinZoom
{
	if (pinchDelegate)
		return pinchDelegate.minZoom;

	return FLT_MIN;
}

- (float)getMaxZoom
{
	if (pinchDelegate)
		return pinchDelegate.maxZoom;

	return FLT_MIN;
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
        Point3d loc = mapView->getLoc();
        if (mapView->heightAboveSurface() < minHeight)
            mapView->setLoc(Point3d(loc.x(),loc.y(),minHeight));
        if (mapView->heightAboveSurface() > maxHeight)
            mapView->setLoc(Point3d(loc.x(),loc.y(),maxHeight));
    }
    if(twoFingerTapDelegate)
    {
        twoFingerTapDelegate.minZoom = minHeight;
        twoFingerTapDelegate.maxZoom = maxHeight;
    }
    if(doubleTapDelegate)
    {
        doubleTapDelegate.minZoom = minHeight;
        doubleTapDelegate.maxZoom = maxHeight;
    }
    if(doubleTapDragDelegate)
    {
        doubleTapDragDelegate.minZoom = minHeight;
        doubleTapDragDelegate.maxZoom = maxHeight;
    }
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    return [self screenPointFromGeo:geoCoord mapView:mapView.get()];
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord mapView:(Maply::MapView *)theView
{
    if (!renderControl)
        return CGPointMake(0.0, 0.0);
    
    Point3d pt = theView->coordAdapter->localToDisplay(theView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = theView->calcFullMatrix();
    auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
    Point2f screenPt = theView->pointOnScreenFromPlane(pt, &modelTrans, frameSizeScaled);
    return CGPointMake(screenPt.x(),screenPt.y());
}

// See if the given bounding box is all on screen
- (bool)checkCoverage:(Mbr &)mbr mapView:(Maply::MapView *)theView height:(float)height margin:(const Point2d &)margin
{
    Point3d loc = mapView->getLoc();
    Point3d testLoc = Point3d(loc.x(),loc.y(),height);
    theView->setLoc(testLoc,false);
    
    CGRect frame = self.view.frame;
    
    MaplyCoordinate ul;
    ul.x = mbr.ul().x();
    ul.y = mbr.ul().y();
    
    MaplyCoordinate lr;
    lr.x = mbr.lr().x();
    lr.y = mbr.lr().y();
    
    CGPoint ulScreen = [self screenPointFromGeo:ul mapView:theView];
    CGPoint lrScreen = [self screenPointFromGeo:lr mapView:theView];
    
    return std::abs(lrScreen.x - ulScreen.x) < (frame.size.width-2*margin.x()) && std::abs(lrScreen.y - ulScreen.y) < (frame.size.height-2*margin.y());
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox pos:(MaplyCoordinate)pos
{
    return [self findHeightToViewBounds:bbox pos:pos marginX:0 marginY:0];
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox pos:(MaplyCoordinate)pos marginX:(double)marginX marginY:(double)marginY
{
    Point2d margin(marginX,marginY);
    Maply::MapView tempMapView(*(mapView.get()));

    Point3d oldLoc = tempMapView.getLoc();
    Point3d newLoc = Point3d(pos.x,pos.y,oldLoc.z());
    tempMapView.setLoc(newLoc, false);
    
    Mbr mbr(Point2f(bbox.ll.x,bbox.ll.y),Point2f(bbox.ur.x,bbox.ur.y));
    
    float minHeight = tempMapView.minHeightAboveSurface();
    float maxHeight = tempMapView.maxHeightAboveSurface();
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minZoom);
        maxHeight = std::min(maxHeight,pinchDelegate.maxZoom);
    }
    
    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr mapView:&tempMapView height:minHeight margin:margin];
    bool maxOnScreen = [self checkCoverage:mbr mapView:&tempMapView height:maxHeight margin:margin];
    if (!minOnScreen && !maxOnScreen)
    {
        tempMapView.setLoc(oldLoc,false);
        return oldLoc.z();
    } else if (minOnScreen) {
        // already fits at min zoom
        maxHeight = minHeight;
    } else {
        // Now for the binary search
        float minRange = 1e-5;
        do
        {
            float midHeight = (minHeight + maxHeight)/2.0;
            bool midOnScreen = [self checkCoverage:mbr mapView:&tempMapView height:midHeight margin:margin];
        
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
    }
    
    return maxHeight;
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSArray *)selectedObjs
{
    MaplyCoordinate coord;
    coord.x = msg.whereGeo.lon();
    coord.y = msg.whereGeo.lat();
    
    // Adjust this if it's outside geographic bounds
    if (coord.x < -M_PI)
        coord.x += 2*M_PI * std::ceil(std::abs((coord.x + M_PI)/(2*M_PI)));
    if (coord.x > M_PI)
        coord.x -= 2*M_PI * std::ceil((coord.x - M_PI)/(2*M_PI));

    if ([selectedObjs count] > 0 && self.selection)
    {
        // The user selected something, so let the delegate know
        if (_delegate)
        {
            if ([_delegate respondsToSelector:@selector(maplyViewController:allSelect:atLoc:onScreen:)])
                [_delegate maplyViewController:self allSelect:selectedObjs atLoc:coord onScreen:msg.touchLoc];
            else {
                MaplySelectedObject *selectVecObj = nil;
                MaplySelectedObject *selObj = nil;
                // If the selected objects are vectors, use the draw priority
                for (MaplySelectedObject *whichObj in selectedObjs)
                {
                    if ([whichObj.selectedObj isKindOfClass:[MaplyVectorObject class]])
                    {
                        MaplyVectorObject *vecObj0 = selectVecObj.selectedObj;
                        MaplyVectorObject *vecObj1 = whichObj.selectedObj;
                        if (!vecObj0 || ([vecObj1.attributes[kMaplyDrawPriority] intValue] > [vecObj0.attributes[kMaplyDrawPriority] intValue]))
                            selectVecObj = whichObj;
                    } else {
                        // If there's a non-vector object just pick it
                        selectVecObj = nil;
                        selObj = whichObj;
                        break;
                    }
                }
                if (selectVecObj)
                    selObj = selectVecObj;

                if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:atLoc:onScreen:)])
                    [_delegate maplyViewController:self didSelect:selObj.selectedObj atLoc:coord onScreen:msg.touchLoc];
                else if ([_delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
                    [_delegate maplyViewController:self didSelect:selObj.selectedObj];
            }
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
    
    // Ignore taps from other view controllers
    if (msg.view != wrapView)
        return;
    
    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [mapInteractLayer userDidTap:msg];
}

// Called when the pan delegate starts moving
- (void) panDidStart:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    //    NSLog(@"Pan started");
    
    [self handleStartMoving:true];
    isPanning = true;
}

// Called when the pan delegate stops moving
- (void) panDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;

    isPanning = false;
    [self handleStopMoving:true];
}

- (void) zoomGestureDidStart:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;

    [self handleStartMoving:true];
    isZooming = true;
}

- (void) zoomGestureDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    isZooming = false;
    [self handleStopMoving:true];
}

- (void) animationDidStart:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    [self handleStartMoving:false];
    isAnimating = true;
}

- (void) animationDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    bool userMotion = false;
    
    auto delegate = mapView->getDelegate();
    AnimateViewTranslation *viewTrans = dynamic_cast<AnimateViewTranslation *>(delegate.get());
    if (viewTrans) {
        userMotion = viewTrans->userMotion;
    } else {
        AnimateTranslateMomentum *viewTrans = dynamic_cast<AnimateTranslateMomentum *>(delegate.get());
        if (viewTrans) {
            userMotion = viewTrans->userMotion;
        }
    }

    isAnimating = false;
    [self handleStopMoving:userMotion];
}

// Convenience routine to handle the end of moving
- (void)handleStartMoving:(bool)userMotion
{
    if (!isPanning && !isZooming && !isAnimating)
    {
        if([self.delegate respondsToSelector:@selector(maplyViewControllerDidStartMoving:userMotion:)])
            [self.delegate maplyViewControllerDidStartMoving:self userMotion:userMotion];
    }
}

// Convenience routine to handle the end of moving
- (void)handleStopMoving:(bool)userMotion
{
    if (isPanning || isZooming || isAnimating)
        return;

    if([self.delegate respondsToSelector:@selector(maplyViewController:didStopMoving:userMotion:)])
    {
        MaplyCoordinate corners[4];
        [self corners:corners];
        [self.delegate maplyViewController:self didStopMoving:corners userMotion:userMotion];
    }
}

- (MaplyCoordinate)geoFromScreenPoint:(CGPoint)point {
  	Point3d hit;
    SceneRenderer *sceneRender = wrapView.renderer;
    Eigen::Matrix4d theTransform = mapView->calcFullMatrix();
    auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();
    Point2f point2f(point.x,point.y);
    if (mapView->pointOnPlaneFromScreen(point2f, &theTransform, frameSizeScaled, &hit, true))
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
    if (!renderControl)
        return;
    
    CGPoint screenCorners[4];
    screenCorners[0] = CGPointMake(0.0, 0.0);
    screenCorners[1] = CGPointMake(renderControl->sceneRenderer->framebufferWidth,0.0);
    screenCorners[2] = CGPointMake(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    screenCorners[3] = CGPointMake(0.0, renderControl->sceneRenderer->framebufferHeight);
    
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

    if([self.delegate respondsToSelector:@selector(maplyViewController:didClickAnnotation:)]) {
        for(MaplyAnnotation *annotation in self.annotations) {
            if(annotation.calloutView == calloutView) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"                
                [self.delegate maplyViewController:self didClickAnnotation:annotation];
                return;
#pragma clang diagostic pop
            }
        }
    }
}

- (void)requirePanGestureRecognizerToFailForGesture:(UIGestureRecognizer *)other {
    if (panDelegate && panDelegate.gestureRecognizer)
        [other requireGestureRecognizerToFail:panDelegate.gestureRecognizer];
}


@end
