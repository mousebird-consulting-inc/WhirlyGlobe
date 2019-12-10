/*
 *  WhirlyGlobeViewController.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2019 mousebird consulting
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
#import "control/WhirlyGlobeViewController.h"
#import "WhirlyGlobeViewController_private.h"
#import "SceneGLES.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeViewControllerSimpleAnimationDelegate
{
    WhirlyGlobeViewControllerAnimationState *startState;
    WhirlyGlobeViewControllerAnimationState *endState;
    TimeInterval startTime,endTime;
}

- (instancetype)init
{
    self = [super init];
    _globeCenter = {-1000,-1000};
    
    return self;
}

- (instancetype)initWithState:(WhirlyGlobeViewControllerAnimationState *)inEndState
{
    self = [super init];
    endState = inEndState;
    
    return self;
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC startState:(WhirlyGlobeViewControllerAnimationState *)inStartState startTime:(TimeInterval)inStartTime endTime:(TimeInterval)inEndTime
{
    startState = inStartState;
    if (!endState)
    {
        endState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
        endState.heading = _heading;
        endState.height = _height;
        endState.tilt = _tilt;
        endState.pos = _loc;
        endState.roll = _roll;
        endState.globeCenter = _globeCenter;
    }
    startTime = inStartTime;
    endTime = inEndTime;
}

- (WhirlyGlobeViewControllerAnimationState *)globeViewController:(WhirlyGlobeViewController *)viewC stateForTime:(TimeInterval)currentTime
{
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
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
    state.tilt = (endState.tilt - startState.tilt)*t + startState.tilt;
    state.roll = (endState.roll - startState.roll)*t + startState.roll;
    MaplyCoordinateD pos;
    pos.x = (endState.pos.x - startState.pos.x)*t + startState.pos.x;
    pos.y = (endState.pos.y - startState.pos.y)*t + startState.pos.y;
    state.pos = pos;
    if (startState.globeCenter.x != -1000 && endState.globeCenter.x != -1000) {
        state.globeCenter = CGPointMake((endState.globeCenter.x - startState.globeCenter.x)*t + startState.globeCenter.x,
                                           (endState.globeCenter.y - startState.globeCenter.y)*t + startState.globeCenter.y);
    }

    return state;
}

- (void)globeViewControllerDidFinishAnimation:(WhirlyGlobeViewController *)viewC
{
}

@end

@interface WhirlyGlobeViewController() <WGInteractionLayerDelegate>
- (void)updateView:(WhirlyGlobe::GlobeView *)theGlobeView;
- (void)viewUpdated:(View *)view;
@end

// Interface object between Obj-C and C++ for animation callbacks
// Also used to catch view geometry updates
class WhirlyGlobeViewWrapper : public WhirlyGlobe::GlobeViewAnimationDelegate, public ViewWatcher
{
public:
    WhirlyGlobeViewWrapper()
    : control(nil)
    {
    }
    
    // Called by the View to set up view state per frame
    void updateView(WhirlyGlobe::GlobeView *globeView)
    {
        [control updateView:globeView];
    }
    
    // Called by the view when things are changed
    virtual void viewUpdated(View *view)
    {
        [control viewUpdated:view];
    }
    
public:
    WhirlyGlobeViewController __weak * control;
};

@implementation WhirlyGlobeViewController
{
    bool isPanning,isRotating,isZooming,isAnimating,isTilting;
    WhirlyGlobeViewWrapper viewWrapper;
    CGPoint globeCenter;
}

- (id) init
{
    self = [super init];
    if (!self)
        return nil;
    
    _autoMoveToTap = true;
    _doubleTapZoomGesture = true;
    _twoFingerTapGesture = true;
    _doubleTapDragGesture = true;
    _zoomTapFactor = 2.0;
    _zoomTapAnimationDuration = 0.1;
    globeCenter = {-1000,-1000};
    viewWrapper.control = self;

    return self;
}

// Tear down layers and layer thread
- (void) clear
{
    [super clear];
    
    globeView = nil;
    globeInteractLayer = nil;
    
    pinchDelegate = nil;
    panDelegate = nil;
    tiltDelegate = nil;
    tapDelegate = nil;
    rotateDelegate = nil;

    doubleTapDelegate = nil;
    twoFingerTapDelegate = nil;
    doubleTapDragDelegate = nil;
    
    delegateRespondsToViewUpdate = false;
}

- (void) dealloc
{
}

- (void)setDelegate:(NSObject<WhirlyGlobeViewControllerDelegate> *)delegate
{
    _delegate = delegate;
    delegateRespondsToViewUpdate = [_delegate respondsToSelector:@selector(globeViewController:didMove:)];
}

// Called by the globe view when something changes
- (void)viewUpdated:(View *)view
{
    if (delegateRespondsToViewUpdate)
    {
        MaplyCoordinate corners[4];
        [self corners:corners forRot:globeView->getRotQuat() viewMat:globeView->calcViewMatrix()];
        [_delegate globeViewController:self didMove:corners];
    }
}

// Create the globe view
- (ViewRef) loadSetup_view
{
    globeView = GlobeView_iOSRef(new GlobeView_iOS());
    globeView->continuousZoom = true;
    globeView->addWatcher(&viewWrapper);
    
    return globeView;
}

- (MaplyBaseInteractionLayer *) loadSetup_interactionLayer
{
    globeInteractLayer = [[WGInteractionLayer alloc] initWithGlobeView:globeView];
    globeInteractLayer.viewController = self;
    return globeInteractLayer;
}

// Put together all the random junk we need to draw
- (void) loadSetup
{
    [super loadSetup];
    
    // Wire up the gesture recognizers
    panDelegate = [WhirlyGlobePanDelegate panDelegateForView:wrapView globeView:globeView useCustomPanRecognizer:self.inScrollView];
    tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:wrapView globeView:globeView.get()];
    // These will activate the appropriate gesture
    self.panGesture = true;
    self.pinchGesture = true;
    self.zoomAroundPinch = true;
    self.rotateGesture = true;
    self.tiltGesture = false;

    self.selection = true;
    
    if(_doubleTapZoomGesture)
    {
        doubleTapDelegate = [WhirlyGlobeDoubleTapDelegate doubleTapDelegateForView:wrapView globeView:globeView.get()];
        doubleTapDelegate.minZoom = pinchDelegate.minHeight;
        doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
        doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
        doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
    }
    if(_twoFingerTapGesture)
    {
        twoFingerTapDelegate = [WhirlyGlobeTwoFingerTapDelegate twoFingerTapDelegateForView:wrapView globeView:globeView.get()];
        twoFingerTapDelegate.minZoom = pinchDelegate.minHeight;
        twoFingerTapDelegate.maxZoom = pinchDelegate.maxHeight;
        twoFingerTapDelegate.zoomTapFactor = _zoomTapFactor;
        twoFingerTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
        if (pinchDelegate)
            [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:twoFingerTapDelegate.gestureRecognizer];
    }
    if (_doubleTapDragGesture)
    {
        doubleTapDragDelegate = [WhirlyGlobeDoubleTapDragDelegate doubleTapDragDelegateForView:wrapView globeView:globeView.get()];
        doubleTapDragDelegate.minZoom = pinchDelegate.minHeight;
        doubleTapDragDelegate.maxZoom = pinchDelegate.maxHeight;
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        [panDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
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
    if (globeView)
        globeView->runViewUpdates();
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
	// Stop tracking notifications
    [self unregisterForEvents];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
}

// Register for interesting tap events and others
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnGlobe:) name:WhirlyGlobeTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOutsideGlobe:) name:WhirlyGlobeTapOutsideMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidStart:) name:kPanDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(panDidEnd:) name:kPanDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tiltDidStart:) name:kTiltDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tiltDidEnd:) name:kTiltDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pinchDidStart:) name:kPinchDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pinchDidEnd:) name:kPinchDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pinchDidStart:) name:kGlobeDoubleTapDragDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(pinchDidEnd:) name:kGlobeDoubleTapDragDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(rotateDidStart:) name:kRotateDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(rotateDidEnd:) name:kRotateDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(animationDidStart:) name:kWKViewAnimationStarted object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(animationDidEnd:) name:kWKViewAnimationEnded object:nil];
}

- (void)unregisterForEvents
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapOutsideMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPanDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPanDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kTiltDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kTiltDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPinchDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kPinchDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kGlobeDoubleTapDragDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kGlobeDoubleTapDragDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kRotateDelegateDidStart object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kRotateDelegateDidEnd object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWKViewAnimationStarted object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWKViewAnimationEnded object:nil];
}

#pragma mark - Properties

- (void)setKeepNorthUp:(bool)keepNorthUp
{
    panDelegate.northUp = keepNorthUp;
    pinchDelegate.northUp = keepNorthUp;

    if (keepNorthUp)
        self.rotateGesture = false;
    else
        self.rotateGesture = true;
}

- (bool)keepNorthUp
{
    return panDelegate.northUp;
}

- (bool)panGesture
{
    return panDelegate.gestureRecognizer.enabled;
}


- (void)setPanGesture:(bool)enabled
{
    panDelegate.gestureRecognizer.enabled = enabled;
    pinchDelegate.allowPan = enabled;
}

- (void)setZoomAroundPinch:(bool)zoomAroundPinch {
    _zoomAroundPinch = zoomAroundPinch;
    if (pinchDelegate) {
        pinchDelegate.zoomAroundPinch = self.zoomAroundPinch;
    }
}

- (void)setPinchGesture:(bool)pinchGesture
{
    if (pinchGesture)
    {
        if (!pinchDelegate)
        {
            pinchDelegate = [WhirlyGlobePinchDelegate pinchDelegateForView:wrapView globeView:globeView];
            pinchDelegate.zoomAroundPinch = self.zoomAroundPinch;
            pinchDelegate.doRotation = false;
            pinchDelegate.northUp = panDelegate.northUp;
            pinchDelegate.rotateDelegate = rotateDelegate;
            tiltDelegate.pinchDelegate = pinchDelegate;
            
            if (twoFingerTapDelegate)
                [twoFingerTapDelegate.gestureRecognizer requireGestureRecognizerToFail:pinchDelegate.gestureRecognizer];
        }
    } else {
        if (pinchDelegate)
        {
            [wrapView removeGestureRecognizer:pinchDelegate.gestureRecognizer];
            pinchDelegate = nil;
            tiltDelegate.pinchDelegate = nil;
        }
    }
}

- (bool)pinchGesture
{
    return pinchDelegate != nil;
}

- (void)setRotateGesture:(bool)rotateGesture
{
    if (rotateGesture)
    {
        if (!rotateDelegate)
        {
            rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:wrapView globeView:globeView.get()];
            rotateDelegate.rotateAroundCenter = true;
            pinchDelegate.rotateDelegate = rotateDelegate;
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:rotateDelegate.gestureRecognizer];
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
            pinchDelegate.rotateDelegate = nil;
            pinchDelegate.doRotation = false;
        }
    }
}

- (bool)rotateGesture
{
    return rotateDelegate != nil;
}

- (void)setTiltGesture:(bool)tiltGesture
{
    if (tiltGesture)
    {
        if (!tiltDelegate)
        {
            tiltDelegate = [WhirlyGlobeTiltDelegate tiltDelegateForView:wrapView globeView:globeView.get()];
            tiltDelegate.pinchDelegate = pinchDelegate;
            tiltDelegate.tiltCalcDelegate = tiltControlDelegate;
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDelegate.gestureRecognizer];
            [tiltDelegate.gestureRecognizer requireGestureRecognizerToFail:twoFingerTapDelegate.gestureRecognizer];
            [tiltDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
    } else {
        if (tiltDelegate)
        {
            [wrapView removeGestureRecognizer:tiltDelegate.gestureRecognizer];
            tiltDelegate = nil;
        }
    }
}

- (bool)tiltGesture
{
    return tiltDelegate != nil;
}

- (void)setAutoRotateInterval:(float)autoRotateInterval degrees:(float)autoRotateDegrees
{
    [globeInteractLayer setAutoRotateInterval:autoRotateInterval degrees:autoRotateDegrees];
}

- (float)height
{
    return globeView->getHeightAboveGlobe();
}

- (void)setHeight:(float)height
{
    globeView->setHeightAboveGlobe(height);
}

- (CGPoint)globeCenter
{
    // If it's not set, it's just the center
    if (globeCenter.x == -1000 || globeCenter.y == -1000) {
        CGRect bounds = self.view.bounds;
        CGPoint ret = {CGRectGetMidX(bounds),CGRectGetMidY(bounds)};
        return ret;
    }

    return globeCenter;
}

- (void)setGlobeCenter:(CGPoint)newGlobeCenter
{
    globeCenter = newGlobeCenter;
    
    if (globeView && self.view.frame.size.width > 0.0 && self.view.frame.size.height > 0.0) {
        double size = self.view.frame.size.width / 2.0;
        double offX = (globeCenter.x - self.view.frame.size.width/2.0)/size;
        double offY = (globeCenter.y - self.view.frame.size.height/2.0)/size;
        globeView->setCenterOffset(offX, offY, true);
    }
}

- (float)getZoomLimitsMin
{
	return pinchDelegate ? pinchDelegate.minHeight : FLT_MIN;
}

- (float)getZoomLimitsMax
{
	return pinchDelegate ? pinchDelegate.maxHeight : FLT_MIN;
}

- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight
{
    if (pinchDelegate)
    {
        *minHeight = pinchDelegate.minHeight;
        *maxHeight = pinchDelegate.maxHeight;
    }
}

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight
{
    if (pinchDelegate)
    {
        pinchDelegate.minHeight = minHeight;
        pinchDelegate.maxHeight = maxHeight;
        auto heightAboveGlobe = globeView->getHeightAboveGlobe();
        if (heightAboveGlobe < minHeight)
            globeView->setHeightAboveGlobe(minHeight);
        if (heightAboveGlobe > maxHeight)
            globeView->setHeightAboveGlobe(maxHeight);

        if (doubleTapDelegate)
        {
            doubleTapDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
        }
        if (doubleTapDragDelegate)
        {
            doubleTapDragDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDragDelegate.maxZoom = pinchDelegate.maxHeight;
        }
        if (twoFingerTapDelegate)
        {
            twoFingerTapDelegate.minZoom = pinchDelegate.minHeight;
            twoFingerTapDelegate.maxZoom = pinchDelegate.maxHeight;
        }
    }
}

- (void)setZoomTapFactor:(float)zoomTapFactor
{
    _zoomTapFactor = zoomTapFactor;
    
    if (doubleTapDelegate)
        doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
    if (twoFingerTapDelegate)
        twoFingerTapDelegate.zoomTapFactor = _zoomTapFactor;
}

- (void)setZoomTapAnimationDuration:(float)zoomAnimationDuration
{
    _zoomTapAnimationDuration = zoomAnimationDuration;
    
    if (doubleTapDelegate)
        doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
    if (twoFingerTapDelegate)
        twoFingerTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
}

- (void)setFarClipPlane:(double)farClipPlane
{
    globeView->setFarClippingPlane(farClipPlane);
}

- (void)setTiltMinHeight:(float)minHeight maxHeight:(float)maxHeight minTilt:(float)minTilt maxTilt:(float)maxTilt
{
    tiltControlDelegate = StandardTiltDelegateRef(new StandardTiltDelegate(globeView.get()));
    tiltControlDelegate->setContraints(minTilt, maxTilt, minHeight, maxHeight);
    if (pinchDelegate)
        pinchDelegate.tiltDelegate = tiltControlDelegate;
    if (doubleTapDelegate)
        doubleTapDelegate.tiltDelegate = tiltControlDelegate;
    if (twoFingerTapDelegate)
        twoFingerTapDelegate.tiltDelegate = tiltControlDelegate;
    if (tiltDelegate)
        tiltDelegate.tiltCalcDelegate = tiltControlDelegate;
}

/// Turn off varying tilt by height
- (void)clearTiltHeight
{
    if (pinchDelegate)
        pinchDelegate.tiltDelegate = nil;
    if (doubleTapDelegate)
        doubleTapDelegate.tiltDelegate = nil;
    if (twoFingerTapDelegate)
        twoFingerTapDelegate.tiltDelegate = nil;
    tiltControlDelegate = nil;
    globeView->setTilt(0.0);
}

- (float)tilt
{
    return globeView->getTilt();
}

- (void)setTilt:(float)newTilt
{
    globeView->setTilt(newTilt);
}

- (double)roll
{
    return globeView->getRoll();
}

- (void)setRoll:(double)newRoll
{
    globeView->setRoll(newRoll, true);
}

- (void)setDoubleTapZoomGesture:(bool)doubleTapZoomGesture
{
    _doubleTapZoomGesture = doubleTapZoomGesture;
    if (doubleTapZoomGesture)
    {
        if (!doubleTapDelegate)
        {
            doubleTapDelegate = [WhirlyGlobeDoubleTapDelegate doubleTapDelegateForView:wrapView globeView:globeView.get()];
            doubleTapDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
            doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
            doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
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
            twoFingerTapDelegate = [WhirlyGlobeTwoFingerTapDelegate twoFingerTapDelegateForView:wrapView globeView:globeView.get()];
            twoFingerTapDelegate.minZoom = pinchDelegate.minHeight;
            twoFingerTapDelegate.maxZoom = pinchDelegate.maxHeight;
            twoFingerTapDelegate.zoomTapFactor = _zoomTapFactor;
            twoFingerTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
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
            doubleTapDragDelegate = [WhirlyGlobeDoubleTapDragDelegate doubleTapDragDelegateForView:wrapView globeView:globeView.get()];
            doubleTapDragDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDragDelegate.maxZoom = pinchDelegate.maxHeight;
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

#pragma mark - Interaction

// Rotate to the given location over time
- (void)rotateToPoint:(GeoCoord)whereGeo time:(TimeInterval)howLong
{
    if (!renderControl)
        return;

    // If we were rotating from one point to another, stop
    globeView->cancelAnimation();
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = globeView->makeRotationToGeoCoord(whereGeo, panDelegate.northUp);
    
    // Rotate to the given position over time
    AnimateViewRotation *anim = new AnimateViewRotation(globeView.get(),newRotQuat,howLong);
    globeView->setDelegate(GlobeViewAnimationDelegateRef(anim));
}

- (void)rotateToPointD:(Point2d)whereGeo time:(TimeInterval)howLong
{
    if (!renderControl)
        return;

    // If we were rotating from one point to another, stop
    globeView->cancelAnimation();
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = globeView->makeRotationToGeoCoord(whereGeo, panDelegate.northUp);
    
    // Rotate to the given position over time
    AnimateViewRotation *anim = new AnimateViewRotation(globeView.get(),newRotQuat,howLong);
    globeView->setDelegate(GlobeViewAnimationDelegateRef(anim));
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(TimeInterval)howLong
{
    if (!renderControl)
        return;

    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return;
    }

    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:howLong];
}

// Figure out how to get the geolocation to the given point on the screen
- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(TimeInterval)howLong
{
    if (!renderControl)
        return false;
    
    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }

    globeView->cancelAnimation();
    
    // Figure out where that point lands on the globe
    Eigen::Matrix4d modelTrans = globeView->calcFullMatrix();
    Point3d whereLoc;
    Point2f loc2f(loc.x,loc.y);
    auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
    if (globeView->pointOnSphereFromScreen(loc2f, modelTrans, frameSizeScaled, whereLoc, true))
    {
        CoordSystemDisplayAdapter *coordAdapter = globeView->coordAdapter;
        Vector3d destPt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
        Eigen::Quaterniond endRot;
        endRot = QuatFromTwoVectors(destPt, whereLoc);
        Eigen::Quaterniond curRotQuat = globeView->getRotQuat();
        Eigen::Quaterniond newRotQuat = curRotQuat * endRot;
        
        if (panDelegate.northUp)
        {
            // We'd like to keep the north pole pointed up
            // So we look at where the north pole is going
            Vector3d northPole = (newRotQuat * Vector3d(0,0,1)).normalized();
            if (northPole.y() != 0.0)
            {
                // Then rotate it back on to the YZ axis
                // This will keep it upward
                float ang = atan(northPole.x()/northPole.y());
                // However, the pole might be down now
                // If so, rotate it back up
                if (northPole.y() < 0.0)
                    ang += M_PI;
                Eigen::AngleAxisd upRot(ang,destPt);
                newRotQuat = newRotQuat * upRot;
            }
        }
        
        // Rotate to the given position over time
        AnimateViewRotation *anim = new AnimateViewRotation(globeView.get(),newRotQuat,howLong);
        globeView->setDelegate(GlobeViewAnimationDelegateRef(anim));
        
        return true;
    } else
        return false;
}

- (bool)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight heading:(float)newHeading time:(TimeInterval)howLong
{
    if (!renderControl)
        return false;

    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }

    globeView->cancelAnimation();

    WhirlyGlobeViewControllerSimpleAnimationDelegate *anim = [[WhirlyGlobeViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = MaplyCoordinateDMakeWithMaplyCoordinate(newPos);
    anim.heading = newHeading;
    anim.height = newHeight;
    anim.tilt = [self tilt];
    
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

- (bool)animateToPositionD:(MaplyCoordinateD)newPos height:(double)newHeight heading:(double)newHeading time:(TimeInterval)howLong
{
    if (!renderControl)
        return false;

    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    
    globeView->cancelAnimation();
    
    WhirlyGlobeViewControllerSimpleAnimationDelegate *anim = [[WhirlyGlobeViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = newPos;
    anim.heading = newHeading;
    anim.height = newHeight;
    anim.tilt = [self tilt];
    
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

- (bool)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc height:(float)newHeight heading:(float)newHeading time:(TimeInterval)howLong {
    
    if (!renderControl)
        return false;

    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }
    
    globeView->cancelAnimation();

    // save current view state
    WhirlyGlobeViewControllerAnimationState *curState = [self getViewState];

    // temporarily change view state, without propagating updates, to find offset coordinate
    WhirlyGlobeViewControllerAnimationState *nextState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    nextState.heading = newHeading;
    nextState.tilt = self.tilt;
    nextState.roll = self.roll;
    nextState.pos = MaplyCoordinateDMakeWithMaplyCoordinate(newPos);
    nextState.height = newHeight;
    [self setViewStateInternal:nextState updateWatchers:false];
    
    // find offset coordinate
    MaplyCoordinate geoCoord;
    CGPoint invPoint = CGPointMake(self.view.frame.size.width/2+loc.x, self.view.frame.size.height/2+loc.y);
    if (![self geoPointFromScreen:invPoint geoCoord:&geoCoord])
    {
        [self setViewStateInternal:curState updateWatchers:false];
        return false;
    }
    
    // restore current view state
    [self setViewStateInternal:curState updateWatchers:false];
    
    // animate to offset coordinate
    WhirlyGlobeViewControllerSimpleAnimationDelegate *anim = [[WhirlyGlobeViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = MaplyCoordinateDMakeWithMaplyCoordinate(geoCoord);
    anim.heading = newHeading;
    anim.height = newHeight;
    anim.tilt = [self tilt];
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    if (!renderControl)
        return;

    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }
    
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:0.0];
    // If there's a pinch delegate, ask it to calculate the height.
    if (tiltControlDelegate) {
        self.tilt = tiltControlDelegate->tiltFromHeight(globeView->getHeightAboveGlobe());
    }
 }

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    if (!renderControl)
        return;

    if (isnan(newPos.x) || isnan(newPos.y) || isnan(height))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }

    [self setPosition:newPos];
    globeView->setHeightAboveGlobe(height);
}

- (void)setPositionD:(MaplyCoordinateD)newPos height:(double)height
{
    if (!renderControl)
        return;

    if (isnan(newPos.x) || isnan(newPos.y) || isnan(height))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }
    
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:0.0];
    globeView->setHeightAboveGlobe(height);
    // If there's a pinch delegate, ask it to calculate the height.
    if (tiltControlDelegate)
        self.tilt = tiltControlDelegate->tiltFromHeight(globeView->getHeightAboveGlobe());
}

- (void)setHeading:(float)heading
{
    if (!renderControl)
        return;

    if (isnan(heading))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid heading passed to setHeading:");
        return;
    }

    // Undo the current heading
    Point3d localPt = globeView->currentUp();
    auto oldRotQuat = globeView->getRotQuat();
    Vector3d northPole = (oldRotQuat * Vector3d(0,0,1)).normalized();
    Quaterniond posQuat = oldRotQuat;
    if (northPole.y() != 0.0)
    {
        // Then rotate it back on to the YZ axis
        // This will keep it upward
        float ang = atan(northPole.x()/northPole.y());
        // However, the pole might be down now
        // If so, rotate it back up
        if (northPole.y() < 0.0)
            ang += M_PI;
        Eigen::AngleAxisd upRot(ang,localPt);
        posQuat = posQuat * upRot;
    }

    Eigen::AngleAxisd rot(heading,localPt);
    Quaterniond newRotQuat = posQuat * rot;
    
    globeView->setRotQuat(newRotQuat);
}

- (float)heading
{
    if (!renderControl)
        return 0.0;

    float retHeading = 0.0;

    // Figure out where the north pole went
    Vector3d northPole = (globeView->getRotQuat() * Vector3d(0,0,1)).normalized();
    if (northPole.y() != 0.0)
        retHeading = atan2(-northPole.x(),northPole.y());
    
    return retHeading;
}

- (MaplyCoordinate)getPosition
{
    if (!renderControl)
        return {0.0, 0.0};

    GeoCoord geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographic(globeView->coordAdapter->displayToLocal(globeView->currentUp()));

	return {.x = geoCoord.lon(), .y = geoCoord.lat()};
}

- (MaplyCoordinateD)getPositionD
{
    if (!renderControl)
        return {0.0, 0.0};

    Point2d geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographicD(globeView->coordAdapter->displayToLocal(globeView->currentUp()));

	return {.x = geoCoord.x(), .y = geoCoord.y()};
}

- (double)getHeight
{
    if (!globeView)
        return 0.0;
    
	return globeView->getHeightAboveGlobe();
}

- (void)getPosition:(MaplyCoordinate *)pos height:(float *)height
{
    if (!renderControl) {
        *height = 0.0;
        return;
    }
    
    *height = globeView->getHeightAboveGlobe();
    Point3d localPt = globeView->currentUp();
    GeoCoord geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographic(globeView->coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.lon();  pos->y = geoCoord.lat();
}

- (void)getPositionD:(MaplyCoordinateD *)pos height:(double *)height
{
    if (!renderControl) {
        pos->x = 0.0;  pos->y = 0.0;  *height = 0.0;
        return;
    }

    *height = globeView->getHeightAboveGlobe();
    Point3d localPt = globeView->currentUp();
    Point2d geoCoord = globeView->coordAdapter->getCoordSystem()->localToGeographicD(globeView->coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSArray *)selectedObjs
{
    MaplyCoordinate coord;
    coord.x = msg.whereGeo.lon();
    coord.y = msg.whereGeo.lat();
    
    bool tappedOutside = msg.worldLoc == Point3f(0,0,0);

    if ([selectedObjs count] > 0 && self.selection)
    {
        // The user selected something, so let the delegate know
        if ([_delegate respondsToSelector:@selector(globeViewController:allSelect:atLoc:onScreen:)])
            [_delegate globeViewController:self allSelect:selectedObjs atLoc:coord onScreen:msg.touchLoc];
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
            
            if (_delegate && [_delegate respondsToSelector:@selector(globeViewController:didSelect:atLoc:onScreen:)])
                [_delegate globeViewController:self didSelect:selObj.selectedObj atLoc:coord onScreen:msg.touchLoc];
            else if (_delegate && [_delegate respondsToSelector:@selector(globeViewController:didSelect:)])
            {
                [_delegate globeViewController:self didSelect:selObj.selectedObj];
            }
        }
    } else {
        if (_delegate)
        {
            if (tappedOutside)
            {
                // User missed all objects and tapped outside the globe
                if ([_delegate respondsToSelector:@selector(globeViewControllerDidTapOutside:)])
                    [_delegate globeViewControllerDidTapOutside:self];
            } else {
                // The user didn't select anything, let the delegate know.
                if ([_delegate respondsToSelector:@selector(globeViewController:didTapAt:)])
                    [_delegate globeViewController:self didTapAt:coord];
            }
        }
        // Didn't select anything, so rotate
        if (_autoMoveToTap)
            [self rotateToPoint:msg.whereGeo time:1.0];
    }
}

// Called when the user taps on the globe.  We'll rotate to that position
- (void) tapOnGlobe:(NSNotification *)note
{
    WhirlyGlobeTapMessage *msg = note.object;
    
    // Ignore taps from other view controllers
    if (msg.view != wrapView)
        return;
    
    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [globeInteractLayer userDidTap:msg];
}

// Called when the user taps outside the globe.
- (void) tapOutsideGlobe:(NSNotification *)note
{
    WhirlyGlobeTapMessage *msg = note.object;
    
    // Ignore taps from other view controllers
    if (msg.view != wrapView)
        return;

    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [globeInteractLayer userDidTap:msg];
}

- (void) handleStartMoving:(bool)userMotion
{
    if (!isPanning && !isRotating && !isZooming && !isAnimating && !isTilting)
    {
        if ([_delegate respondsToSelector:@selector(globeViewControllerDidStartMoving:userMotion:)])
            [_delegate globeViewControllerDidStartMoving:self userMotion:userMotion];
    }
}

// Calculate the corners we'll be looking at with the given rotation
- (void)corners:(MaplyCoordinate *)corners forRot:(Eigen::Quaterniond)theRot viewMat:(Matrix4d)viewMat
{
    if (!renderControl)
        return;
    
    Point2f screenCorners[4];
    screenCorners[0] = Point2f(0.0, 0.0);
    screenCorners[1] = Point2f(renderControl->sceneRenderer->framebufferWidth,0.0);
    screenCorners[2] = Point2f(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    screenCorners[3] = Point2f(0.0, renderControl->sceneRenderer->framebufferHeight);
    
    Eigen::Matrix4d modelTrans;
    Eigen::Affine3d trans(Eigen::Translation3d(0,0,-globeView->calcEarthZOffset()));
    Eigen::Affine3d rot(theRot);
    Eigen::Matrix4d modelMat = (trans * rot).matrix();
    
    modelTrans = viewMat * modelMat;

    auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3d hit;
        if (globeView->pointOnSphereFromScreen(screenCorners[ii], modelTrans, frameSizeScaled, hit, true))
        {
            Point3d geoHit = renderControl->scene->getCoordAdapter()->displayToLocal(hit);
            corners[ii].x = geoHit.x();  corners[ii].y = geoHit.y();
        } else {
            corners[ii].x = MAXFLOAT;  corners[ii].y = MAXFLOAT;
        }
    }
}

// Convenience routine to handle the end of moving
- (void)handleStopMoving:(bool)userMotion
{
    if (isPanning || isRotating || isZooming || isAnimating || isTilting)
        return;
    
    if (![_delegate respondsToSelector:@selector(globeViewController:didStopMoving:userMotion:)])
        return;
    
    MaplyCoordinate corners[4];
    [self corners:corners forRot:globeView->getRotQuat() viewMat:globeView->calcViewMatrix()];

    [_delegate globeViewController:self didStopMoving:corners userMotion:userMotion];
}

// Called when the tilt delegate starts moving
- (void) tiltDidStart:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
    [self handleStartMoving:true];
    isTilting = true;
}

// Called when the tilt delegate stops moving
- (void) tiltDidEnd:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
    isTilting = false;
    [self handleStopMoving:true];
}

// Called when the pan delegate starts moving
- (void) panDidStart:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Pan started");

    [self handleStartMoving:true];
    isPanning = true;
}

// Called when the pan delegate stops moving
- (void) panDidEnd:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Pan ended");
    
    isPanning = false;
    [self handleStopMoving:true];
}

- (void) pinchDidStart:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Pinch started");
    
    [self handleStartMoving:true];
    isZooming = true;
}

- (void) pinchDidEnd:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Pinch ended");

    isZooming = false;
    [self handleStopMoving:true];
}

- (void) rotateDidStart:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Rotate started");
    
    [self handleStartMoving:true];
    isRotating = true;
}

- (void) rotateDidEnd:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Rotate ended");
    
    isRotating = false;
    [self handleStopMoving:true];
}

- (void) animationDidStart:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Animation started");

    [self handleStartMoving:false];
    isAnimating = true;
}

- (void) animationDidEnd:(NSNotification *)note
{
    if (note.object != globeView->tag)
        return;
    
//    NSLog(@"Animation ended");
    
    // Momentum animation is only kicked off by the pan delegate.
    bool userMotion = false;
    if (dynamic_cast<AnimateViewRotation *>(globeView->getDelegate().get()))
        userMotion = true;
    
    isAnimating = false;
    knownAnimateEndRot = false;
    [self handleStopMoving:userMotion];
}

// See if the given bounding box is all on sreen
- (bool)checkCoverage:(Mbr &)mbr globeView:(WhirlyGlobe::GlobeView *)theView height:(float)height
{
    theView->setHeightAboveGlobe(height, false);

    Point2fVector pts;
    mbr.asPoints(pts);
    CGRect frame = self.view.frame;
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        Point2f pt = pts[ii];
        MaplyCoordinate geoCoord;
        geoCoord.x = pt.x();  geoCoord.y = pt.y();
        CGPoint screenPt = [self pointOnScreenFromGeo:geoCoord globeView:theView];
        if (screenPt.x < 0 || screenPt.y < 0 || screenPt.x > frame.size.width || screenPt.y > frame.size.height)
            return false;
    }
    
    return true;
}

- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox pos:(MaplyCoordinate)pos
{
    GlobeView tempGlobe(*globeView.get());
    
    float oldHeight = globeView->getHeightAboveGlobe();
    Eigen::Quaterniond newRotQuat = tempGlobe.makeRotationToGeoCoord(GeoCoord(pos.x,pos.y), true);
    tempGlobe.setRotQuat(newRotQuat,false);

    Mbr mbr(Point2f(bbox.ll.x,bbox.ll.y),Point2f(bbox.ur.x,bbox.ur.y));
    
    double minHeight = tempGlobe.minHeightAboveGlobe();
    double maxHeight = tempGlobe.maxHeightAboveGlobe();
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,(double)pinchDelegate.minHeight);
        maxHeight = std::min(maxHeight,(double)pinchDelegate.maxHeight);
    }

    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr globeView:&tempGlobe height:minHeight];
    bool maxOnScreen = [self checkCoverage:mbr globeView:&tempGlobe height:maxHeight];
    if (!minOnScreen && !maxOnScreen)
    {
        tempGlobe.setHeightAboveGlobe(oldHeight,false);
        return oldHeight;
    }
    
    // Now for the binary search
    float minRange = 1e-5;
    do
    {
        float midHeight = (minHeight + maxHeight)/2.0;
        bool midOnScreen = [self checkCoverage:mbr globeView:&tempGlobe height:midHeight];
        
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
    
    return maxHeight;
}

- (CGPoint)pointOnScreenFromGeo:(MaplyCoordinate)geoCoord
{
    return [self pointOnScreenFromGeo:geoCoord globeView:globeView.get()];
}

- (CGPoint)pointOnScreenFromGeo:(MaplyCoordinate)geoCoord globeView:(GlobeView *)theView
{
    if (!renderControl)
        return CGPointMake(0.0, 0.0);
    
    Point3d pt = theView->coordAdapter->localToDisplay(theView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = theView->calcFullMatrix();
    
    auto screenPt = theView->pointOnScreenFromSphere(pt, &modelTrans, renderControl->sceneRenderer->getFramebufferSizeScaled());
    return CGPointMake(screenPt.x(),screenPt.y());
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
	CGPoint p;

	if (![self screenPointFromGeo:geoCoord screenPt:&p]) {
		return CGPointZero;
	}

	return p;
}

- (bool)screenPointFromGeo:(MaplyCoordinate)geoCoord screenPt:(CGPoint *)screenPt
{
    if (!renderControl)
        return false;
    
    Point3d pt = renderControl->visualView->coordAdapter->localToDisplay(renderControl->visualView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    Point3f pt3f(pt.x(),pt.y(),pt.z());
    
    Eigen::Matrix4d modelTrans4d = renderControl->visualView->calcModelMatrix();
    Eigen::Matrix4d viewTrans4d = renderControl->visualView->calcViewMatrix();
    Eigen::Matrix4d modelAndViewMat4d = viewTrans4d * modelTrans4d;
    Eigen::Matrix4f modelAndViewMat = Matrix4dToMatrix4f(modelAndViewMat4d);
    Eigen::Matrix4f modelAndViewNormalMat = modelAndViewMat.inverse().transpose();

    if (CheckPointAndNormFacing(pt3f,pt3f.normalized(),modelAndViewMat,modelAndViewNormalMat) < 0.0)
        return false;
    
    auto screenPt2f = globeView->pointOnScreenFromSphere(pt, &modelAndViewMat4d, renderControl->sceneRenderer->getFramebufferSizeScaled());
    screenPt->x = screenPt2f.x();  screenPt->y = screenPt2f.y();
    
    if (screenPt->x < 0 || screenPt->y < 0 || screenPt->x > renderControl->sceneRenderer->framebufferWidth || screenPt->y > renderControl->sceneRenderer->framebufferHeight)
        return false;
    
    return true;
}

- (bool)geoPointFromScreen:(CGPoint)screenPt geoCoord:(MaplyCoordinate *)retCoord
{
    if (!renderControl)
        return false;
    
	Point3d hit;
    Point2f screenPt2f(screenPt.x,screenPt.y);
	Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    if (globeView->pointOnSphereFromScreen(screenPt2f, theTransform, renderControl->sceneRenderer->getFramebufferSizeScaled(), hit, true))
    {
        GeoCoord geoCoord = renderControl->visualView->coordAdapter->getCoordSystem()->localToGeographic(renderControl->visualView->coordAdapter->displayToLocal(hit));
        retCoord->x = geoCoord.x();
        retCoord->y = geoCoord.y();
        
        return true;
	} else
        return false;
}

- (NSArray *)geocPointFromScreen:(CGPoint)screenPt
{
	double coords[3];

	if (![self geocPointFromScreen:screenPt geocCoord:coords]) {
		return nil;
	}

	return @[@(coords[0]), @(coords[1]), @(coords[2])];
}


- (bool)geocPointFromScreen:(CGPoint)screenPt geocCoord:(double *)retCoords
{
    if (!renderControl)
        return false;
    
	Point3d hit;
	Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
    if (globeView->pointOnSphereFromScreen(Point2f(screenPt.x,screenPt.y), theTransform, renderControl->sceneRenderer->getFramebufferSizeScaled(), hit, true))
    {
        Point3d geoC = renderControl->visualView->coordAdapter->getCoordSystem()->localToGeocentric(renderControl->visualView->coordAdapter->displayToLocal(hit));
        retCoords[0] = geoC.x();  retCoords[1] = geoC.y();  retCoords[2] = geoC.z();
        
        // Note: Obviously doing something stupid here
        if (isnan(retCoords[0]) || isnan(retCoords[1]) || isnan(retCoords[2]))
            return false;
        
        return true;
	} else
        return false;
}

- (CGSize)realWorldSizeFromScreenPt0:(CGPoint)pt0 pt1:(CGPoint)pt1
{
    CGSize size = CGSizeMake(-1.0, -1.0);
    
    if (!renderControl)
        return size;
    
    // Three points on the screen to give us two vectors
    Point2f screenPt[3];
    screenPt[0] = Point2f(pt0.x,pt0.y);
    screenPt[1] = Point2f(pt1.x,pt0.y);
    screenPt[2] = Point2f(pt0.x,pt1.y);
    Point3d hits[3];

    for (int ii=0;ii<3;ii++) {
        Point3d &hit = hits[ii];
        Eigen::Matrix4d theTransform = globeView->calcFullMatrix();
        if (globeView->pointOnSphereFromScreen(screenPt[ii], theTransform, renderControl->sceneRenderer->getFramebufferSizeScaled(), hit, true))
        {
            // Note: Obviously doing something stupid here
            if (isnan(hit.x()) || isnan(hit.y()) || isnan(hit.z()))
                return size;
            hit.normalize();
        } else
            return size;
    }
    
    double da = (hits[1] - hits[0]).norm() * EarthRadius;
    double db = (hits[2] - hits[0]).norm() * EarthRadius;
    size = CGSizeMake(da,db);
    
    return size;
}

// Note: Finish writing this
- (id)findObjectAtLocation:(CGPoint)screenPt
{
    if (!renderControl)
        return nil;
    
    // Look for the object, returns an ID
    SelectionManager *selectManager = (SelectionManager *)renderControl->scene->getManager(kWKSelectionManager);
    SimpleIdentity objId = selectManager->pickObject(Point2f(screenPt.x,screenPt.y), 10.0, globeView->makeViewState(renderControl->sceneRenderer.get()));
    
    if (objId != EmptyIdentity)
    {
        // Now ask the interaction layer for the right object
        return [renderControl->interactLayer getSelectableObject:objId];
    }
    
    return nil;
}

- (void)requirePanGestureRecognizerToFailForGesture:(UIGestureRecognizer *)other {
    if (panDelegate && panDelegate.gestureRecognizer)
        [other requireGestureRecognizerToFail:panDelegate.gestureRecognizer];
}


#pragma mark - WhirlyGlobeAnimationDelegate

// Called every frame from within the globe view
- (void)updateView:(GlobeView *)inGlobeView
{
    TimeInterval now = renderControl->scene->getCurrentTime();
    if (!animationDelegate)
    {
        globeView->cancelAnimation();
        return;
    }
    
    bool lastOne = false;
    if (now > animationDelegateEnd)
        lastOne = true;
    
    // Ask the delegate where we're supposed to be
    WhirlyGlobeViewControllerAnimationState *animState = [animationDelegate globeViewController:self stateForTime:now];
    
    [self setViewStateInternal:animState];
    
    if (lastOne)
    {
        globeView->cancelAnimation();
        if ([animationDelegate respondsToSelector:@selector(globeViewControllerDidFinishAnimation:)])
            [animationDelegate globeViewControllerDidFinishAnimation:self];
        animationDelegate = nil;
    }
}

- (void)animateWithDelegate:(NSObject<WhirlyGlobeViewControllerAnimationDelegate> *)inAnimationDelegate time:(TimeInterval)howLong
{
    TimeInterval now = renderControl->scene->getCurrentTime();
    animationDelegate = inAnimationDelegate;
    animationDelegateEnd = now+howLong;

    WhirlyGlobeViewControllerAnimationState *stateStart = [self getViewState];
    
    stateStart.heading = fmod(stateStart.heading + 2.0*M_PI, 2.0*M_PI);
    
    // Tell the delegate what we're up to
    [animationDelegate globeViewController:self startState:stateStart startTime:now endTime:animationDelegateEnd];
    
    WhirlyGlobeViewWrapper *delegate = new WhirlyGlobeViewWrapper();
    delegate->control = self;
    globeView->setDelegate(GlobeViewAnimationDelegateRef(delegate));
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *)animState
{
    [self setViewState:animState updateWatchers:true];
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *)animState updateWatchers:(bool)updateWatchers
{
    globeView->cancelAnimation();
    [self setViewStateInternal:animState updateWatchers:updateWatchers];
}

- (void)setViewStateInternal:(WhirlyGlobeViewControllerAnimationState *)animState
{
    [self setViewStateInternal:animState updateWatchers:true];
}

- (void)setViewStateInternal:(WhirlyGlobeViewControllerAnimationState *)animState updateWatchers:(bool)updateWatchers {
    if (!renderControl)
        return;
    
    Vector3d startLoc(0,0,1);
    
    if (animState.screenPos.x >= 0.0 && animState.screenPos.y >= 0.0)
    {
        Matrix4d heightTrans = Eigen::Affine3d(Eigen::Translation3d(0,0,-globeView->calcEarthZOffset())).matrix();
        Point3d hit;
        if (globeView->pointOnSphereFromScreen(Point2f(animState.screenPos.x,animState.screenPos.y), heightTrans, renderControl->sceneRenderer->getFramebufferSizeScaled(), hit, true))
        {
            startLoc = hit;
        }
    }
    
    // Start with a rotation from the clean start state to the location
    Point3d worldLoc = globeView->coordAdapter->localToDisplay(globeView->coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y)));
    Eigen::Quaterniond posRot = QuatFromTwoVectors(worldLoc, startLoc);
    
    // Orient with north up.  Either because we want that or we're about do do a heading
    Eigen::Quaterniond posRotNorth = posRot;
    if (panDelegate.northUp || animState.heading < MAXFLOAT)
    {
        // We'd like to keep the north pole pointed up
        // So we look at where the north pole is going
        Vector3d northPole = (posRot * Vector3d(0,0,1)).normalized();
        if (northPole.y() != 0.0)
        {
            // Then rotate it back on to the YZ axis
            // This will keep it upward
            float ang = atan(northPole.x()/northPole.y());
            // However, the pole might be down now
            // If so, rotate it back up
            if (northPole.y() < 0.0)
                ang += M_PI;
            Eigen::AngleAxisd upRot(ang,worldLoc);
            posRotNorth = posRot * upRot;
        }
    }
    
    // We can't have both northUp and a heading
    Eigen::Quaterniond finalQuat = posRotNorth;
    if (!panDelegate.northUp && animState.heading < MAXFLOAT)
    {
        Eigen::AngleAxisd headingRot(animState.heading,worldLoc);
        finalQuat = posRotNorth * headingRot;
    }
    
    // Set the height (easy)
    globeView->setHeightAboveGlobe(animState.height,false);
    
    // Set the tilt either directly or as a consequence of the height
    if (animState.tilt >= MAXFLOAT)
        globeView->setTilt(tiltControlDelegate->tiltFromHeight(globeView->getHeightAboveGlobe()));
    else
        globeView->setTilt(animState.tilt);
    globeView->setRoll(animState.roll, false);

    globeView->setRotQuat(finalQuat, updateWatchers);
    
    if (self.view.frame.size.width > 0.0 && self.view.frame.size.height > 0.0 &&
        animState.globeCenter.x != -1000 && animState.globeCenter.y != -1000) {
        double size = self.view.frame.size.width / 2.0;
        double offX = (animState.globeCenter.x - self.view.frame.size.width/2.0)/size;
        double offY = (animState.globeCenter.y - self.view.frame.size.height/2.0)/size;
        globeView->setCenterOffset(offX, offY, true);
    }
}

- (WhirlyGlobeViewControllerAnimationState *)getViewState
{
    // Figure out the current state
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    startQuat = globeView->getRotQuat();
    startUp = globeView->currentUp();
    state.heading = self.heading;
    state.tilt = self.tilt;
    state.roll = self.roll;
    MaplyCoordinateD pos;
    double height;
    [self getPositionD:&pos height:&height];
    state.pos = pos;
    state.height = height;
    state.globeCenter = [self globeCenter];
    
    return state;
}

- (WhirlyGlobeViewControllerAnimationState *)viewStateForLookAt:(MaplyCoordinate)coord tilt:(float)tilt heading:(float)heading altitude:(float)alt range:(float)range
{
    Vector3f north(0,0,1);
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter = globeView->coordAdapter;
    WhirlyKit::CoordSystem *coordSys = coordAdapter->getCoordSystem();
    Vector3f p0norm = coordAdapter->localToDisplay(coordSys->geographicToLocal(WhirlyKit::GeoCoord(coord.x,coord.y)));
    // Position we're looking at in display coords
    Vector3f p0 = p0norm * (1.0 + alt);
    
    Vector3f right = north.cross(p0norm);
    // This will happen near the poles
    if (right.squaredNorm() < 1e-5)
        right = Vector3f(1,0,0);
    Eigen::Affine3f tiltRot(Eigen::AngleAxisf(tilt,right));
    Vector3f p0normRange = p0norm * (range);
    Vector4f rVec4 = tiltRot.matrix() * Vector4f(p0normRange.x(), p0normRange.y(), p0normRange.z(),1.0);
    Vector3f rVec(rVec4.x(),rVec4.y(),rVec4.z());
    Vector3f p1 = p0 + rVec;
    Vector3f p1norm = p1.normalized();
    
    float dot = (-p1.normalized()).dot((p0-p1).normalized());
    
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    
    WhirlyKit::GeoCoord outGeoCoord = coordSys->localToGeographic(coordAdapter->displayToLocal(p1norm));
    state.pos = MaplyCoordinateDMake(outGeoCoord.lon(), outGeoCoord.lat());
    state.tilt = acosf(dot);
    
    if(isnan(state.tilt) || isnan(state.pos.x) || isnan(state.pos.y))
        return nil;
    
    state.height = sqrtf(p1.dot(p1)) - 1.0;
    state.heading = heading;
    
    return state;
}

- (void)applyConstraintsToViewState:(WhirlyGlobeViewControllerAnimationState *)viewState
{
    if (pinchDelegate)
    {
        if (viewState.height < pinchDelegate.minHeight)
            viewState.height = pinchDelegate.minHeight;
        if (viewState.height > pinchDelegate.maxHeight)
            viewState.height = pinchDelegate.maxHeight;
    }

    if (tiltControlDelegate)
    {
        viewState.tilt = tiltControlDelegate->tiltFromHeight(viewState.height);
    }
}

- (MaplyBoundingBox)getCurrentExtents
{
	MaplyBoundingBox box;

	if (![self getCurrentExtents:&box]) {
		return kMaplyNullBoundingBox;
	}

	return box;
}

- (bool) getCurrentExtents:(MaplyBoundingBox *)bbox
{
    CGRect frame = self.view.frame;
    
    CGPoint pt = CGPointMake(0,frame.size.height);
    
    bool resp = [self geoPointFromScreen:pt geoCoord:&(bbox->ll)];
    
    if (!resp)
    {
        // Left lower point is outside the globe
        return false;
    }
    
    pt = CGPointMake(frame.size.width,0);
    
    resp = [self geoPointFromScreen:pt geoCoord:&(bbox->ur)];
    if (!resp)
    {
        // Right upper point is outside the globe
        return false;
    }
    
    return true;
    
}

static const float LonAng = 2*M_PI/5.0;
static const float LatAng = M_PI/4.0;

// Can't represent the whole earth with -M_PI/+M_PI.  Have to fudge.
static const float FullExtentEps = 1e-5;

- (int)getUsableGeoBoundsForView:(MaplyBoundingBox *)bboxes visual:(bool)visualBoxes
{
    if (!renderControl)
        return 0;
    
    float extentEps = visualBoxes ? FullExtentEps : 0.0;
    
    Point2f screenCorners[4];
    screenCorners[0] = Point2f(0.0, 0.0);
    screenCorners[1] = Point2f(renderControl->sceneRenderer->framebufferWidth,0.0);
    screenCorners[2] = Point2f(renderControl->sceneRenderer->framebufferWidth,renderControl->sceneRenderer->framebufferHeight);
    screenCorners[3] = Point2f(0.0, renderControl->sceneRenderer->framebufferHeight);
    
    Eigen::Matrix4d modelTrans = globeView->calcFullMatrix();
    auto frameSizeScaled = renderControl->sceneRenderer->getFramebufferSizeScaled();

    Point3d corners[4];
    bool cornerValid[4];
    int numValid = 0;;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3d hit;
        if (globeView->pointOnSphereFromScreen(screenCorners[ii], modelTrans, frameSizeScaled, hit, true))
        {
            corners[ii] = renderControl->scene->getCoordAdapter()->displayToLocal(hit);
            cornerValid[ii] = true;
            numValid++;
        } else {
            cornerValid[ii] = false;
        }
    }
    
    // Current location the user is over
    Point3d localPt = globeView->currentUp();
    GeoCoord currentLoc = globeView->coordAdapter->getCoordSystem()->localToGeographic(globeView->coordAdapter->displayToLocal(localPt));

    // Toss in the current location
    std::vector<Mbr> mbrs(1);
    
    bool datelineSplit = false;

    // If no corners are visible, we'll just make up a hemisphere
    if (numValid == 0)
    {
        GeoCoord n,s,e,w;
        bool northOverflow = false, southOverflow = false;
        n.x() = currentLoc.x();  n.y() = currentLoc.y()+LatAng;
        if (n.y() > M_PI/2.0)
        {
            n.y() = M_PI/2.0;
            northOverflow = true;
        }
        s.x() = currentLoc.x();  s.y() = currentLoc.y()-LatAng;
        if (s.y() < -M_PI/2.0)
        {
            s.y() = -M_PI/2.0;
            southOverflow = true;
        }
        
        e.x() = currentLoc.x()+LonAng;  e.y() = currentLoc.y();
        w.x() = currentLoc.x()-LonAng;  w.y() = currentLoc.y();
        
        Mbr mbr;
        mbr.addPoint(Point2d(n.x(),n.y()));
        mbr.addPoint(Point2d(s.x(),s.y()));
        mbr.addPoint(Point2d(e.x(),e.y()));
        mbr.addPoint(Point2d(w.x(),w.y()));
        
        if (northOverflow)
        {
            mbrs.clear();
            if (visualBoxes)
            {
                mbrs.resize(2);
                mbrs[0].ll() = Point2f(-M_PI+extentEps,mbr.ll().y());
                mbrs[0].ur() = Point2f(0,M_PI/2.0);
                mbrs[1].ll() = Point2f(0,mbr.ll().y());
                mbrs[1].ur() = Point2f(M_PI-extentEps,M_PI/2.0);
            } else {
                mbrs.resize(1);
                mbrs[0].ll() = Point2f(-M_PI+extentEps,mbr.ll().y());
                mbrs[0].ur() = Point2f(M_PI-extentEps,M_PI/2.0);
            }
        } else if (southOverflow)
        {
            mbrs.clear();
            if (visualBoxes)
            {
                mbrs.resize(2);
                mbrs[0].ll() = Point2f(-M_PI+extentEps,-M_PI/2.0);
                mbrs[0].ur() = Point2f(0,mbr.ur().y());
                mbrs[1].ll() = Point2f(0,-M_PI/2.0);
                mbrs[1].ur() = Point2f(M_PI-extentEps,mbr.ur().y());
            } else {
                mbrs.resize(1);
                mbrs[0].ll() = Point2f(-M_PI+extentEps,-M_PI/2.0);
                mbrs[0].ur() = Point2f(M_PI-extentEps,mbr.ur().y());
            }
        } else {
            mbrs[0] = mbr;
        }
    } else {
        // Start with the four (or however many corners)
        for (unsigned int ii=0;ii<4;ii++)
            if (cornerValid[ii])
                mbrs[0].addPoint(Point2d(corners[ii].x(),corners[ii].y()));
        
        // See if the MBR is split across +180/-180
        if (mbrs[0].ur().x() - mbrs[0].ll().x() > M_PI)
        {
            // If so, reconstruct the MBRs appropriately
            mbrs.clear();
            mbrs.resize(2);
            datelineSplit = true;
            for (unsigned int ii=0;ii<4;ii++)
                if (cornerValid[ii])
                {
                    Point2d testPt = Point2d(corners[ii].x(),corners[ii].y());
                    if (testPt.x() < 0.0)
                        mbrs[1].addPoint(testPt);
                    else
                        mbrs[0].addPoint(testPt);
                }
            mbrs[0].addPoint(Point2d(M_PI,mbrs[1].ll().y()));
            mbrs[1].addPoint(Point2d(-M_PI,mbrs[0].ll().y()));
        }

        // Add midpoints along the edges
        for (unsigned int ii=0;ii<4;ii++)
        {
            int a = ii, b = (ii+1)%4;
            if (cornerValid[a] && cornerValid[b])
            {
                int numSamples = 8;
                for (unsigned int bi=1;bi<numSamples-1;bi++)
                {
                    Point2f screenPt(bi*(screenCorners[a].x()+screenCorners[b].x())/numSamples, bi*(screenCorners[a].y()+screenCorners[b].y())/numSamples);
                    Point3d hit;
                    if (globeView->pointOnSphereFromScreen(screenPt, modelTrans, frameSizeScaled, hit, true)) {
                        Point3d midPt3d = renderControl->scene->getCoordAdapter()->displayToLocal(hit);
                        if (mbrs.size() > 1)
                        {
                            if (midPt3d.x() < 0.0)
                                mbrs[1].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                            else
                                mbrs[0].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                        } else
                            mbrs[0].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                    }
                }
            }
        }
        
        if (numValid < 4)
        {
            // Look for intersection between globe and screen
            for (unsigned int ii=0;ii<4;ii++)
            {
                int a = ii, b = (ii+1)%4;
                if ((cornerValid[a] && !cornerValid[b]) ||
                    (!cornerValid[a] && cornerValid[b]))
                {
                    Point2f testPts[2];
                    if (cornerValid[a])
                    {
                        testPts[0] = screenCorners[a];
                        testPts[1] = screenCorners[b];
                    } else {
                        testPts[0] = screenCorners[b];
                        testPts[1] = screenCorners[a];
                    }

                    // Do a binary search for a few iterations
                    for (unsigned int bi=0;bi<8;bi++)
                    {
                        Point2f midPt((testPts[0].x()+testPts[1].x())/2, (testPts[0].y()+testPts[1].y())/2);
                        Point3d hit;
                        if (globeView->pointOnSphereFromScreen(midPt, modelTrans, frameSizeScaled, hit, true))
                        {
                            testPts[0] = midPt;
                        } else {
                            testPts[1] = midPt;
                        }
                    }
                    
                    // The first test point is valid, so let's convert that back
                    Point3d hit;
                    if (globeView->pointOnSphereFromScreen(testPts[0], modelTrans, frameSizeScaled, hit, true))
                    {
                        Point3d midPt3d = renderControl->scene->getCoordAdapter()->displayToLocal(hit);
                        if (mbrs.size() > 1)
                        {
                            if (midPt3d.x() < 0.0)
                                mbrs[1].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                            else
                                mbrs[0].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                        } else
                            mbrs[0].addPoint(Point2d(midPt3d.x(),midPt3d.y()));
                    }
                }
            }
        } else {
            // Check the poles
            Point3d poles[2];
            poles[0] = Point3d(0,0,1);
            poles[1] = Point3d(0,0,-1);
            
            Eigen::Matrix4d modelAndViewNormalMat = modelTrans.inverse().transpose();
            
            for (unsigned int ii=0;ii<2;ii++)
            {
                const Point3d &pt = poles[ii];
                if (CheckPointAndNormFacing(pt,pt.normalized(),modelTrans,modelAndViewNormalMat) < 0.0)
                    continue;
                
                Point2f screenPt = globeView->pointOnScreenFromSphere(pt, &modelTrans, frameSizeScaled);
            
                if (screenPt.x() < 0 || screenPt.y() < 0 || screenPt.x() > renderControl->sceneRenderer->framebufferWidth || screenPt.y() > renderControl->sceneRenderer->framebufferHeight)
                    continue;

                // Include the pole and just do the whole area
                switch (ii)
                {
                    case 0:
                    {
                        double minY = mbrs[0].ll().y();
                        if (mbrs.size() > 1)
                            minY = std::min((double)mbrs[1].ll().y(),minY);
                        
                        mbrs.clear();
                        if (visualBoxes)
                        {
                            mbrs.resize(2);
                            mbrs[0].ll() = Point2f(-M_PI+extentEps,minY);
                            mbrs[0].ur() = Point2f(0,M_PI/2.0);
                            mbrs[1].ll() = Point2f(0,minY);
                            mbrs[1].ur() = Point2f(M_PI-extentEps,M_PI/2.0);
                        } else {
                            mbrs.resize(1);
                            mbrs[0].ll() = Point2f(-M_PI+extentEps,minY);
                            mbrs[0].ur() = Point2f(M_PI-extentEps,M_PI/2.0);
                        }
                        datelineSplit = false;
                    }
                        break;
                    case 1:
                    {
                        double maxY = mbrs[0].ur().y();
                        if (mbrs.size() > 1)
                            maxY = std::max((double)mbrs[1].ur().y(),maxY);
                        
                        mbrs.clear();
                        if (visualBoxes)
                        {
                            mbrs.resize(2);
                            mbrs[0].ll() = Point2f(-M_PI+extentEps,-M_PI/2.0);
                            mbrs[0].ur() = Point2f(0,maxY);
                            mbrs[1].ll() = Point2f(0,-M_PI/2.0);
                            mbrs[1].ur() = Point2f(M_PI-extentEps,maxY);
                        } else {
                            mbrs.resize(1);
                            mbrs[0].ll() = Point2f(-M_PI+extentEps,-M_PI/2.0);
                            mbrs[0].ur() = Point2f(M_PI-extentEps,maxY);
                        }
                        datelineSplit = false;
                    }
                        break;
                }
            }
        }
    }
    
    // If the MBR is larger than M_PI, split it up
    // Has trouble displaying otherwise
    if (visualBoxes && mbrs.size() == 1 &&
        mbrs[0].ur().x() - mbrs[0].ll().x() > M_PI)
    {
        mbrs.push_back(mbrs[0]);
        mbrs[0].ur().x() = 0.0;
        mbrs[1].ll().x() = 0.0;
    }
    
    // For non-visual requests merge the MBRs back together if we split them
    if (!visualBoxes && mbrs.size() == 2)
    {
        Mbr mbr;
        mbr.ll() = Point2f(mbrs[0].ll().x()-2*M_PI,mbrs[0].ll().y());
        mbr.ur() = Point2f(mbrs[1].ur().x(),mbrs[1].ur().y());
        mbrs.clear();
        mbrs.push_back(mbr);
    }
    
    // Toss in the user's location, which is important for tilt
    if (datelineSplit && mbrs.size() == 2)
    {
        if (currentLoc.x() < 0.0)
            mbrs[1].addPoint(Point2d(currentLoc.lon(),currentLoc.lat()));
        else
            mbrs[0].addPoint(Point2d(currentLoc.lon(),currentLoc.lat()));
        
        // And make sure the Y's match up or this will be hard to put back together
        double minY = std::min(mbrs[0].ll().y(),mbrs[1].ll().y());
        double maxY = std::max(mbrs[0].ur().y(),mbrs[1].ur().y());
        mbrs[0].ll().y() = mbrs[1].ll().y() = minY;
        mbrs[0].ur().y() = mbrs[1].ur().y() = maxY;
    } else if (mbrs.size() == 1)
        mbrs[0].addPoint(Point2d(currentLoc.lon(),currentLoc.lat()));
    
    for (unsigned int ii=0;ii<mbrs.size();ii++)
    {
        const Mbr &mbr = mbrs[ii];
        MaplyBoundingBox *bbox = &bboxes[ii];
        bbox->ll.x = mbr.ll().x();  bbox->ll.y = mbr.ll().y();
        bbox->ur.x = mbr.ur().x();  bbox->ur.y = mbr.ur().y();
    }
    
    return (int)mbrs.size();
}

@end
