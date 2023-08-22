/*  MaplyViewController.mm
 *  MaplyComponent
 *
 *  Created by Steve Gifford on 9/6/12.
 *  Copyright 2012-2022 mousebird consulting
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
 */

#import <WhirlyGlobe_iOS.h>
#import "MaplyViewController.h"
#import "MaplyAnimateTranslateMomentum.h"
#import "GlobeView_iOS.h"
#import "private/MaplyBaseViewController_private.h"
#import "private/MaplyViewController_private.h"
#import "private/MaplyInteractionLayer_private.h"
#import "private/MaplyCoordinateSystem_private.h"
#import "private/MaplyAnnotation_private.h"
#import "private/MaplyDoubleTapDelegate_private.h"
#import "private/MaplyDoubleTapDragDelegate_private.h"
#import "private/MaplyPanDelegate_private.h"
#import "private/MaplyPinchDelegate_private.h"
#import "private/MaplyRotateDelegate_private.h"
#import "private/MaplyTapDelegate_private.h"
#import "private/MaplyTapMessage_private.h"
#import "private/MaplyTouchCancelAnimationDelegate_private.h"
#import "private/MaplyTwoFingerTapDelegate_private.h"
#import "private/MaplyZoomGestureDelegate_private.h"

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

    if (auto easing = _zoomEasing)
    {
        state.height = easing(startState.height, endState.height, t);
    }
    else
    {
        state.height = exp((log(endState.height) - log(startState.height))*t + log(startState.height));
    }

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
struct MaplyViewControllerAnimationWrapper : public Maply::MapViewAnimationDelegate, public ViewWatcher
{
    MaplyViewControllerAnimationWrapper(MaplyViewController *control) : control(control)
    {
    }

    // Called by the View to set up view state per frame
    virtual void updateView(WhirlyKit::View *view) override
    {
        [control updateView:(Maply::MapView *)view];
    }
    
    // Called by the view when things are changed
    virtual void viewUpdated(View *view) override
    {
        [control viewUpdated:view];
    }

    virtual bool isUserMotion() const override { return false; }

private:
    MaplyViewController __weak * control = nil;
};

@implementation MaplyViewController
{
    // Content scale for scroll view mode
    float scale;
    bool scheduledToDraw;
    
    /// Boundary quad that we're to stay within, in display coords
    Point2dVector bounds;
    Point2d bounds2d[4];
    
    std::shared_ptr<MaplyViewControllerAnimationWrapper> animWrapper;
}

- (instancetype)initWithMapType:(MaplyMapType)inMapType
{
    self = [super init];
    if (!self)
        return nil;
    mapType = inMapType;
        
    animWrapper = std::make_shared<MaplyViewControllerAnimationWrapper>(self);

    if (mapType == MaplyMapType3D) {
        _autoMoveToTap = true;
    }
    else {
        // Turn off lighting
        [self setHints:@{kMaplyRendererLightingMode: @"none"}];
    }

    _isPanning = false;
    _isZooming = false;
    _isAnimating = false;
    _pinchGesture = true;
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;
    _rotateGestureThreshold = 0.0f;
//    self.useOpenGLES = true;

    return self;
}

- (instancetype)init
{
    self = [super init];
    if (!self)
        return nil;

    animWrapper = std::make_shared<MaplyViewControllerAnimationWrapper>(self);

    _isPanning = false;
    _isZooming = false;
    _isAnimating = false;
    _autoMoveToTap = true;
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;
    _rotateGestureThreshold = 0.0f;
//    self.useOpenGLES = true;

    return self;
}

- (instancetype)initAsFlatMap
{
    self = [super init];
    if (!self)
        return nil;

    animWrapper = std::make_shared<MaplyViewControllerAnimationWrapper>(self);

    // Turn off lighting
    [self setHints:@{kMaplyRendererLightingMode: @"none"}];
    _isPanning = false;
    _isZooming = false;
    _isAnimating = false;
    _rotateGesture = true;
    _doubleTapDragGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapZoomGesture = true;
    _rotateGestureThreshold = 0.0f;

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
    delegateRespondsToViewUpdate = [delegate respondsToSelector:@selector(maplyViewController:didMove:)];
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
#if !MAPLY_MINIMAL
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
#endif //!MAPLY_MINIMAL
}

- (ViewRef) loadSetup_view
{
    if (mapType == MaplyMapTypeOverlay || !_coordSys) {
        // In this case the view is tied to an outside matrix
        const auto originLon = 0.0;
        const auto ll = GeoCoord::CoordFromDegrees(-180.0,-90.0);
        const auto ur = GeoCoord::CoordFromDegrees(180.0,90.0);
        coordAdapter = std::make_shared<SphericalMercatorDisplayAdapter>(originLon, ll, ur);
    } else {
        const auto bbox = [_coordSys getBounds];
        const auto ll3d = Point3d(bbox.ll.x, bbox.ll.y, 0);
        const auto ur3d = Point3d(bbox.ur.x, bbox.ur.y, 0);
        const Point3d diff = ur3d - ll3d;
        const auto center3d = Point3d(_displayCenter.x,_displayCenter.y,_displayCenter.z);
        // May need to scale this to the space we're expecting
        const auto scaleFactor = (std::abs(diff.x()) > 10.0 || std::abs(diff.y()) > 10.0) ?
        4.0/std::max(diff.x(),diff.y()) : 1.0;
        coordAdapter = std::make_shared<GeneralCoordSystemDisplayAdapter>(
                                                                          [_coordSys getCoordSystem].get(),ll3d,ur3d,center3d,
                                                                          Point3d(scaleFactor,scaleFactor,1));
    }
    
    if (mapType == MaplyMapTypeOverlay) {
        mapView = std::make_shared<MapViewOverlay_iOS>(coordAdapter.get());
    } else {
        mapView = std::make_shared<MapView_iOS>(coordAdapter.get());
    }
    mapView->setContinuousZoom(true);
    mapView->setWrap(_viewWrap);
    mapView->addWatcher(animWrapper);

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
    
    // Make the UIViews transparent to touch
    if (mapType == MaplyMapTypeOverlay) {
        self.view.userInteractionEnabled = false;
        wrapView.userInteractionEnabled = false;
    }
    
    allowRepositionForAnnnotations = false;
    
    Point2d ll(0,0),ur(0,0);
    coordAdapter->getGeoBounds(ll, ur);

    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();

    // Let them move E/W infinitely, but only with the default coordinate system
    if (_viewWrap && !_coordSys)
    {
        boundLL.x = -MAXFLOAT;
        boundUR.x = MAXFLOAT;
    }
    
    if (mapType != MaplyMapTypeOverlay) {
        // Wire up the gesture recognizers
        tapDelegate = [MaplyTapDelegate tapDelegateForView:wrapView mapView:mapView.get()];
        panDelegate = [MaplyPanDelegate panDelegateForView:wrapView mapView:mapView useCustomPanRecognizer:false];
        if (_pinchGesture)
        {
            pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:wrapView mapView:mapView];
            pinchDelegate.minZoom = mapView->minHeightAboveSurface();
            pinchDelegate.maxZoom = mapView->maxHeightAboveSurface();
        }
        if(_rotateGesture) {
            rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:wrapView mapView:mapView.get()];
            rotateDelegate.rotateThreshold = _rotateGestureThreshold;
        }
        const auto __strong tapRecognizer = tapDelegate.gestureRecognizer;
        if(_doubleTapZoomGesture)
        {
            doubleTapDelegate = [MaplyDoubleTapDelegate doubleTapDelegateForView:wrapView mapView:mapView];
            doubleTapDelegate.minZoom = mapView->minHeightAboveSurface();
            doubleTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
            doubleTapDelegate.approveAllGestures = self.fastGestures;
            [tapRecognizer requireGestureRecognizerToFail:doubleTapDelegate.gestureRecognizer];
        }
        if(_twoFingerTapGesture)
        {
            twoFingerTapDelegate = [MaplyTwoFingerTapDelegate twoFingerTapDelegateForView:wrapView mapView:mapView];
            twoFingerTapDelegate.minZoom = mapView->minHeightAboveSurface();
            twoFingerTapDelegate.maxZoom = mapView->maxHeightAboveSurface();
            twoFingerTapDelegate.approveAllGestures = self.fastGestures;
            if (pinchDelegate && !self.fastGestures)
                [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
            [tapRecognizer requireGestureRecognizerToFail:twoFingerTapDelegate.gestureRecognizer];
        }
        if (_doubleTapDragGesture)
        {
            doubleTapDragDelegate = [MaplyDoubleTapDragDelegate doubleTapDragDelegateForView:wrapView mapView:mapView];
            doubleTapDragDelegate.minZoom = mapView->minHeightAboveSurface();
            doubleTapDragDelegate.maxZoom = mapView->maxHeightAboveSurface();
            if (self.fastGestures)
                doubleTapDragDelegate.minimumPressDuration = 0.01;
            [tapRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
            if (!self.fastGestures)
                [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
        if(_cancelAnimationOnTouch)
        {
            touchDelegate = [MaplyTouchCancelAnimationDelegate touchDelegateForView:wrapView mapView:mapView.get()];
        }
    }

    if (boundLL.x != boundUR.x || boundLL.y != boundUR.y)
    {
        [self setViewExtentsLL:boundLL ur:boundUR];
    }
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

    Point2d ll(0,0),ur(0,0);
    coordAdapter->getGeoBounds(ll, ur);
    boundLL.x = ll.x();  boundLL.y = ll.y();
    boundUR.x = ur.x();  boundUR.y = ur.y();
    
    // Let them move E/W infinitely
    if (_viewWrap)
    {
        boundLL.x = -MAXFLOAT;
        boundUR.x = MAXFLOAT;
    }

    if (boundLL.x != boundUR.x || boundLL.y != boundUR.y)
    {
        [self setViewExtentsLL:boundLL ur:boundUR];
    }
    
    mapView->setWrap(_viewWrap);
}

- (void)setIsPanning:(bool)isPanning
{
    _isPanning = isPanning;
    if (renderControl && renderControl->visualView)
    {
        renderControl->visualView->setIsPanning(isPanning);
    }
}

- (void)setIsZooming:(bool)isZooming
{
    _isZooming = isZooming;
    if (renderControl && renderControl->visualView)
    {
        renderControl->visualView->setIsZooming(isZooming);
    }
}

- (void)setIsAnimating:(bool)isAnimating
{
    _isAnimating = isAnimating;
    if (renderControl && renderControl->visualView)
    {
        renderControl->visualView->setIsAnimating(isAnimating);
    }
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
    if (mapView) {
        mapView->runViewUpdates();
    }
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
    if (pinchGesture && !pinchDelegate)
    {
        // gesture enabled but no delegate is set, create one.
        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:wrapView mapView:mapView];
        pinchDelegate.minZoom = mapView->minHeightAboveSurface();
        pinchDelegate.maxZoom = mapView->maxHeightAboveSurface();

        if (twoFingerTapDelegate)
            [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
    }
    else if (!pinchGesture && pinchDelegate)
    {
        // gesture disabled but a delegate is set, remove it
        if (pinchDelegate.gestureRecognizer) {
            [wrapView removeGestureRecognizer:pinchDelegate.gestureRecognizer];
        }
        pinchDelegate = nil;
    }
}

- (void)setRotateGesture:(bool)rotateGesture
{
    _rotateGesture = rotateGesture;
    if (rotateGesture && !rotateDelegate)
    {
        // Rotate gesture enabled but no rotate delegate, create one
        rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:wrapView mapView:mapView.get()];
        rotateDelegate.rotateThreshold = _rotateGestureThreshold;
    }
    else if (!rotateGesture && rotateDelegate)
    {
        // Rotate gesture disabled but a delegate exists, remove it.
        if (rotateDelegate.gestureRecognizer) {
            [wrapView removeGestureRecognizer:rotateDelegate.gestureRecognizer];
        }
        rotateDelegate = nil;
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
    if (height != loc.z())
    {
        loc.z() = height;
        mapView->setLoc(loc, true);
        mapView->setHasZoomed(true);
    }
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtents:(MaplyBoundingBox)box
{
    [self setViewExtentsLL:box.ll ur:box.ur];
}

/// Set the view extents.  This is the box the view point is allowed to be within.
- (void)setViewExtentsLL:(MaplyCoordinate)ll ur:(MaplyCoordinate)ur
{
    const auto adapter = mapView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();

    boundLL = ll;
    boundUR = ur;
    
    // Convert the bounds to a rectangle in local coordinates
    const Point3f bounds3d[4] = {
        adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ll.y))),
        adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ll.y))),
        adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ur.x,ur.y))),
        adapter->localToDisplay(coordSys->geographicToLocal(GeoCoord(ll.x,ur.y))),
    };
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
    
    const auto render = renderControl->sceneRenderer.get();
    AnimateViewTranslationRef anim = std::make_shared<AnimateViewTranslation>(mapView,render,newLoc,howLong);
    anim->userMotion = false;
    anim->setBounds(bounds2d);

    curAnimation = anim;
    mapView->setDelegate(std::move(anim));
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(TimeInterval)howLong
{
    // Snap to the bounds
    if (newPos.x > boundUR.x)  newPos.x = boundUR.x;
    if (newPos.y > boundUR.y)  newPos.y = boundUR.y;
    if (newPos.x < boundLL.x)  newPos.x = boundLL.x;
    if (newPos.y < boundLL.y)  newPos.y = boundLL.y;

    const auto adapter = mapView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();
    Point3d loc = adapter->localToDisplay(coordSys->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = mapView->getLoc().z();
    [self animateToPoint:loc time:howLong];
}

// Note: This may not work with a tilt
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(TimeInterval)howLong
{
    if (!renderControl || !renderControl->scene)
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
        const auto adapter = mapView->getCoordAdapter();
        const CoordSystem *coordSys = adapter->getCoordSystem();
        Point3d loc = adapter->localToDisplay(coordSys->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
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
    if (!renderControl || !renderControl->scene)
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

    const auto adapter = mapView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();
    Point3d loc = adapter->localToDisplay(coordSys->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
    loc.z() = newHeight;
    
    [self animateToPoint:loc time:howLong];
}

- (bool)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight heading:(float)newHeading time:(TimeInterval)howLong
{
    if (!renderControl || !renderControl->scene)
        return false;

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
    anim.zoomEasing = self.animationZoomEasing;
    [self animateWithDelegate:anim time:howLong];

    return true;
}

- (bool)animateToPositionD:(MaplyCoordinateD)newPos height:(double)newHeight heading:(double)newHeading time:(TimeInterval)howLong
{
    if (!renderControl || !renderControl->scene)
        return false;

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
    anim.zoomEasing = self.animationZoomEasing;

    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

- (bool)animateToPosition:(MaplyCoordinate)newPos
                 onScreen:(CGPoint)loc
                   height:(float)newHeight
                  heading:(float)newHeading
                     time:(TimeInterval)howLong {
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"MaplyViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    if (!renderControl || !renderControl->scene)
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
        anim.zoomEasing = self.animationZoomEasing;

        [self animateWithDelegate:anim time:howLong];
    }
    
    return valid;
}

// Bounds check on a single point
- (bool)withinBounds:(const Point3d &)loc
                view:(UIView *)view
            renderer:(SceneRenderer *)sceneRender
             mapView:(Maply::MapView *)testMapView
           newCenter:(Point3d *)newCenter
{
    return bounds.empty() || MaplyGestureWithinBounds(bounds,loc,sceneRender,testMapView,newCenter);
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
    
    const auto adapter = mapView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();
    const Point3d curLoc = coordSys->geographicToLocal3d(GeoCoord(newPos.x,newPos.y));
    const Point3d newLoc(curLoc.x(), curLoc.y(), height);

    // Do a validity check and possibly adjust the center
    Maply::MapView testMapView(*(mapView.get()));
    Point3d newCenter;
    if ([self withinBounds:newLoc view:wrapView renderer:renderControl->sceneRenderer.get() mapView:&testMapView newCenter:&newCenter])
    {
        mapView->setLoc(newCenter);
        if (newCenter.x() != curLoc.x() || newCenter.y() != curLoc.y())
        {
            mapView->setHasMoved(true);
        }
        if (newCenter.z() != curLoc.z())
        {
            mapView->setHasZoomed(true);
        }
    }

    [self handleStopMoving:NO];
}

- (MaplyCoordinate)getPosition
{
    const auto adapter = mapView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();
    const GeoCoord geoCoord = coordSys->localToGeographic(adapter->displayToLocal(mapView->getLoc()));
    return {.x = geoCoord.x(), .y = geoCoord.y()};
}

- (float)getHeight
{
    return mapView->getLoc().z();
}

- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height
{
    const Point3d loc = mapView->getLoc();
    const CoordSystem *coordSys = mapView->getCoordAdapter()->getCoordSystem();
    const GeoCoord geoCoord = coordSys->localToGeographic(loc);
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
    
    const auto adapter = mapView->getCoordAdapter();
    Point3d localLoc = adapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x, animState.pos.y));
    localLoc.z() = animState.height;

    if (animState.screenPos.x >= 0.0 && animState.screenPos.y >= 0.0)
    {
        const auto adapter = mapView->getCoordAdapter();
        Point3d displayLoc = adapter->localToDisplay(localLoc);
        displayLoc.z() = animState.height;

        Eigen::Matrix4d modelTrans = mapView->calcFullMatrix();
        Point3d hit;
        auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
        Point2f screenPos2f(animState.screenPos.x,animState.screenPos.y);
        if (mapView->pointOnPlaneFromScreen(screenPos2f, &modelTrans, frameSizeScaled, &hit, true))
        {
            Point3d oldLoc = mapView->getLoc();
            Point3f diffLoc(hit.x()-oldLoc.x(),hit.y()-oldLoc.y(),0.0);
            displayLoc.x() -= diffLoc.x();
            displayLoc.y() -= diffLoc.y();

        }
        localLoc = adapter->displayToLocal(displayLoc);
    }
    mapView->setLoc(localLoc, false);
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
    TimeInterval now = renderControl->scene->getCurrentTime();
    animationDelegate = inAnimationDelegate;
    animationDelegateEnd = now+howLong;
    
    MaplyViewControllerAnimationState *stateStart = [self getViewState];
    // Tell the delegate what we're up to
    [animationDelegate mapViewController:self startState:stateStart startTime:now endTime:animationDelegateEnd];
    
    mapView->setDelegate(std::make_shared<MaplyViewControllerAnimationWrapper>(self));
}

// Called every frame from within the map view
- (void)updateView:(Maply::MapView *)theMapView
{
    if (!renderControl)
        return;
    
    TimeInterval now = renderControl->scene->getCurrentTime();
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
    const CoordSystem *coordSys = coordAdapter->getCoordSystem();
    Point3d loc = coordSys->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y));
    loc.z() = animState.height;
    Maply::MapView testMapView(*(mapView.get()));
    Point3d newCenter {0,0,0};
    if ([self withinBounds:loc view:wrapView renderer:renderControl->sceneRenderer.get() mapView:&testMapView newCenter:&newCenter])
    {
        GeoCoord geoCoord = coordSys->localToGeographic(newCenter);
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
    if (heading != mapView->getRotAngle())
    {
        mapView->setHasRotated(true);
    }
    mapView->setRotAngle(heading,true);
}

- (float)heading
{
    return mapView->getRotAngle();
}

- (void)setRotateGestureThreshold:(float)threshold
{
    if (threshold != _rotateGestureThreshold) {
        _rotateGestureThreshold = threshold;
        if (rotateDelegate) {
            rotateDelegate.rotateThreshold = threshold;
        }
    }
    
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
    {
        return CGPointZero;
    }

    const auto adapter = theView->getCoordAdapter();
    const CoordSystem *coordSys = adapter->getCoordSystem();
    const Point3d localPt = coordSys->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y));
    const Point3d displayPt = adapter->localToDisplay(localPt);
    const Eigen::Matrix4d modelTrans = theView->calcFullMatrix();
    const Point2f frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
    const Point2f screenPt = theView->pointOnScreenFromPlane(displayPt, &modelTrans, frameSizeScaled);
    return CGPointMake(screenPt.x(),screenPt.y());
}

// See if the given bounding box is all on screen
- (bool)checkCoverage:(const Mbr &)mbr
              mapView:(Maply::MapView *)theView
                  loc:(MaplyCoordinate)loc
               height:(float)height
                frame:(CGRect)frame
               newLoc:(MaplyCoordinate *)newLoc
               margin:(const Point2d &)margin
{
    if (frame.size.width == 0 || frame.size.height == 0)
    {
        return false;
    }
    if (newLoc)
    {
        *newLoc = loc;
    }

    const auto &coordAdapter = mapView->getCoordAdapter();
    const auto *coordSys = coordAdapter->getCoordSystem();

    // Center the given location
    Point3d localMid = coordSys->geographicToLocal(Point2d(loc.x, loc.y));
    theView->setLoc(Point3d(localMid.x(),localMid.y(),height),false);

    // If they want to center in an area other than the whole view frame, we need to work out
    // what center point will place the given location at the center of the given view frame.
    const auto screenCenter = CGPointMake(CGRectGetMidX(self.view.frame), CGRectGetMidY(self.view.frame));
    const auto frameCenter = CGPointMake(CGRectGetMidX(frame), CGRectGetMidY(frame));
    const auto offset = CGPointMake(frameCenter.x - screenCenter.x, frameCenter.y - screenCenter.y);
    if (offset.x != 0 || offset.y != 0)
    {
        const auto invCenter = CGPointMake(screenCenter.x - offset.x,screenCenter.y - offset.y);
        MaplyCoordinate invGeo = {0,0};
        if (![self geoFromScreenPoint:invCenter view:theView geo:&invGeo])
        {
            return false;
        }

        // Place the given location at the center of the given frame
        localMid = coordSys->geographicToLocal(Point2d(invGeo.x, invGeo.y));
        theView->setLoc(Point3d(localMid.x(), localMid.y(), height), false);
        
        if (newLoc)
        {
            *newLoc = invGeo;
        }
    }

    // Get the corners
    Point2fVector pts;
    mbr.asPoints(pts);

    // Check if each corner is within the given frame in this view
    for (const auto &pt : pts)
    {
        const CGPoint screenPt = [self screenPointFromGeo:{pt.x(),pt.y()} mapView:theView];
        if (!std::isfinite(screenPt.y) ||
            screenPt.x < frame.origin.x - margin.x() ||
            screenPt.y < frame.origin.y - margin.y() ||
            screenPt.x > frame.origin.x + frame.size.width + margin.x() ||
            screenPt.y > frame.origin.y + frame.size.height + margin.y())
        {
            return false;
        }
    }
    return true;
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox
                            pos:(MaplyCoordinate)pos
{
    return [self findHeightToViewBounds:bbox
                                    pos:pos
                                marginX:0
                                marginY:0];
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox
                            pos:(MaplyCoordinate)pos
                        marginX:(double)marginX
                        marginY:(double)marginY
{
    return [self findHeightToViewBounds:bbox
                                    pos:pos
                                  frame:self.view.frame
                                 newPos:nil
                                marginX:marginX
                                marginY:marginY];
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox
                            pos:(MaplyCoordinate)pos
                          frame:(CGRect)frame
                         newPos:(MaplyCoordinate *)newPos
                        marginX:(double)marginX
                        marginY:(double)marginY
{
    if (!mapView)
    {
        return 0;
    }

    // checkCoverage won't work if the frame size isn't set
    if (frame.size.height * frame.size.width == 0)
    {
        return 0;
    }

    Maply::MapView tempMapView(*mapView);

    // Center the temporary view on the new center point at the current height
    //const Point3d oldLoc = tempMapView.getLoc();

    const Mbr mbr { { bbox.ll.x, bbox.ll.y }, { bbox.ur.x, bbox.ur.y } };
    const Point2d margin(marginX,marginY);

    float minHeight = tempMapView.minHeightAboveSurface();
    float maxHeight = tempMapView.maxHeightAboveSurface();
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minZoom);
        maxHeight = std::min(maxHeight,pinchDelegate.maxZoom);
    }
    
    // Check that we can at least see it
    MaplyCoordinate minPos, maxPos;
    const bool minOnScreen = [self checkCoverage:mbr mapView:&tempMapView loc:pos height:minHeight
                                           frame:frame newLoc:&minPos margin:margin];
    const bool maxOnScreen = [self checkCoverage:mbr mapView:&tempMapView loc:pos height:maxHeight
                                           frame:frame newLoc:&maxPos margin:margin];
    if (!minOnScreen && !maxOnScreen)
    {
        if (newPos)
        {
            *newPos = pos;
        }
        return 0.0;
    }
    else if (minOnScreen)
    {
        // already fits at min zoom
        if (newPos)
        {
            *newPos = minPos;
        }
        return minHeight;
    }

    // minHeight is out but maxHeight works.
    // Binary search to find the lowest height that still works.
    constexpr float minRange = 1e-5;
    while (maxHeight - minHeight > minRange)
    {
        const float midHeight = (minHeight + maxHeight)/2.0;
        if ([self checkCoverage:mbr mapView:&tempMapView loc:pos height:midHeight
                         frame:frame newLoc:newPos margin:margin])
        {
            maxHeight = midHeight;
        }
        else
        {
            minHeight = midHeight;
        }
    }
    return maxHeight;
}

#if !MAPLY_MINIMAL
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

    const auto delegate = _delegate;
    if ([selectedObjs count] > 0 && self.selection)
    {
        // The user selected something, so let the delegate know
        if ([delegate respondsToSelector:@selector(maplyViewController:allSelect:atLoc:onScreen:)])
            [delegate maplyViewController:self allSelect:selectedObjs atLoc:coord onScreen:msg.touchLoc];
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

            if ([delegate respondsToSelector:@selector(maplyViewController:didSelect:atLoc:onScreen:)])
                [delegate maplyViewController:self didSelect:selObj.selectedObj atLoc:coord onScreen:msg.touchLoc];
            else if ([delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
                [delegate maplyViewController:self didSelect:selObj.selectedObj];
        }
    } else {
        // The user didn't select anything, let the delegate know.
        if ([delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
            [delegate maplyViewController:self didTapAt:coord];
        if (_autoMoveToTap)
            [self animateToPosition:coord time:1.0];
    }
}
#endif //!MAPLY_MINIMAL


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
    self.isPanning = true;
}

// Called when the pan delegate stops moving
- (void) panDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;

    self.isPanning = false;
    [self handleStopMoving:true];
}

- (void) zoomGestureDidStart:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;

    if (self.fastGestures) {
        // Cancel any pending recognition of other gestures.
        // ("If you change this property to NO while a gesture recognizer is currently
        //   regognizing a gesture, the gesture recognizer transitions to a cancelled state.")
        UIGestureRecognizer __strong *panRec = panDelegate.gestureRecognizer;
        panRec.enabled = NO;
        panRec.enabled = YES;
        UIGestureRecognizer __strong *tapRec = twoFingerTapDelegate.gestureRecognizer;
        tapRec.enabled = NO;
        tapRec.enabled = YES;
    }

    [self handleStartMoving:true];
    self.isZooming = true;
}

- (void) zoomGestureDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    self.isZooming = false;
    [self handleStopMoving:true];
}

- (void) animationDidStart:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    [self handleStartMoving:false];
    self.isAnimating = true;
}

- (void) animationDidEnd:(NSNotification *)note
{
    if (note.object != mapView->tag)
        return;
    
    const auto delegate = mapView->getDelegate();
    const bool userMotion = delegate && delegate->isUserMotion();

    self.isAnimating = false;
    [self handleStopMoving:userMotion];
}

// Convenience routine to handle the end of moving
- (void)handleStartMoving:(bool)userMotion
{
    [super handleStartMoving:userMotion];

    if (!_isPanning && !_isZooming && !_isAnimating)
    {
        const auto __strong delegate = self.delegate;
        if ([delegate respondsToSelector:@selector(maplyViewControllerDidStartMoving:userMotion:)])
            [delegate maplyViewControllerDidStartMoving:self userMotion:userMotion];
    }
}

// Convenience routine to handle the end of moving
- (void)handleStopMoving:(bool)userMotion
{
    [super handleStopMoving:userMotion];

    if (_isPanning || _isZooming || _isAnimating)
        return;

    const auto __strong delegate = self.delegate;
    if ([delegate respondsToSelector:@selector(maplyViewController:didStopMoving:userMotion:)])
    {
        MaplyCoordinate corners[4];
        [self corners:corners];
        [delegate maplyViewController:self didStopMoving:corners userMotion:userMotion];
    }
}

- (MaplyCoordinate)geoFromScreenPoint:(CGPoint)point
{
    return [self geoFromScreenPoint:point view:mapView.get()];
}

- (MaplyCoordinate)geoFromScreenPoint:(CGPoint)point view:(MapView*)view
{
    MaplyCoordinate geo = {0,0};
    [self geoFromScreenPoint:point view:view geo:&geo];
    return geo;
}

- (bool)geoFromScreenPoint:(CGPoint)point geo:(MaplyCoordinate*)geo
{
    return [self geoFromScreenPoint:point view:mapView.get() geo:geo];
}

- (bool)geoFromScreenPoint:(CGPoint)point view:(MapView*)view geo:(MaplyCoordinate*)geo
{
    if (view)
    {
        SceneRenderer *sceneRender = wrapView.renderer;
        Eigen::Matrix4d theTransform = view->calcFullMatrix();
        const auto frameSizeScaled = sceneRender->getFramebufferSizeScaled();

        Point2f point2f(point.x,point.y);
        Point3d hit;
        if (view->pointOnPlaneFromScreen(point2f, &theTransform, frameSizeScaled, &hit, true))
        {
            if (geo)
            {
                const Point3d localPt = coordAdapter->displayToLocal(hit);
                const GeoCoord coord = coordAdapter->getCoordSystem()->localToGeographic(localPt);
                *geo = {coord.x(), coord.y()};
            }
            return true;
        }
    }
    return false;
}


- (void)corners:(MaplyCoordinate *)corners
{
    if (!renderControl)
        return;
    
    const Point2f frameSize = renderControl->sceneRenderer->getFramebufferSize();
    const CGPoint screenCorners[4] = {
        CGPointMake(0.0f, 0.0f),
        CGPointMake(frameSize.x(),0.0f),
        CGPointMake(frameSize.x(),frameSize.y()),
        CGPointMake(0.0f, frameSize.y()),
    };
    
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
    const auto __strong delegate = self.delegate;
    if([delegate respondsToSelector:@selector(maplyViewController:didTapAnnotation:)]) {
        for(MaplyAnnotation *annotation in self.annotations) {
            if(annotation.calloutView == calloutView) {
                [delegate maplyViewController:self didTapAnnotation:annotation];
                return;
            }
        }
    }

    if([delegate respondsToSelector:@selector(maplyViewController:didClickAnnotation:)]) {
        for(MaplyAnnotation *annotation in self.annotations) {
            if(annotation.calloutView == calloutView) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                [delegate maplyViewController:self didClickAnnotation:annotation];
                return;
#pragma clang diagostic pop
            }
        }
    }
}

- (void)requirePanGestureRecognizerToFailForGesture:(UIGestureRecognizer *)other {
    auto const __strong recognizer = panDelegate.gestureRecognizer;
    if (recognizer)
        [other requireGestureRecognizerToFail:recognizer];
}

- (void)assignViewMatrixFromMaplibre:(double * __nonnull)matrixValues scale:(double)scale tileSize:(int)tileSize
{
    if (mapType == MaplyMapTypeOverlay) {
        MapViewOverlay_iOSRef theMapView = std::dynamic_pointer_cast<MapViewOverlay_iOS>(mapView);
        if (theMapView) {
            Eigen::Matrix4d inMvp;
            
            memcpy(inMvp.data(),matrixValues,sizeof(double)*16);
                
            // Apply a scale to our data first
            double worldSize = tileSize / (M_PI) * pow(2.0,scale) ;
            // Note: Can see something with this
        //    double scale = 40075016.68 / (2*8192*M_PI) ;
            const Eigen::Affine3d scaleTrans(Eigen::Scaling(worldSize,-worldSize,1.0));
            const Eigen::Affine3d transTrans(Eigen::Translation3d(M_PI,-M_PI,0.0));
            Eigen::Matrix4d mvp = (inMvp * (scaleTrans * transTrans) ).matrix();

            theMapView->assignMatrix(mvp);
        }
    }
}

@end
