/*
 *  WhirlyGlobeViewController.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2015 mousebird consulting
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
#import "WhirlyGlobeViewController.h"
#import "WhirlyGlobeViewController_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeViewControllerAnimationState

- (id)init
{
    self = [super init];
    _heading = MAXFLOAT;
    _height = 1.0;
    _tilt = MAXFLOAT;
    _pos.x = _pos.y = 0.0;
    _screenPos = {-1,-1};
    
    return self;
}

+ (WhirlyGlobeViewControllerAnimationState *)Interpolate:(double)t from:(WhirlyGlobeViewControllerAnimationState *)stateA to:(WhirlyGlobeViewControllerAnimationState *)stateB
{
    WhirlyGlobeViewControllerAnimationState *newState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    
    newState.heading = (stateB.heading-stateA.heading)*t + stateA.heading;
    newState.height = (stateB.height-stateA.height)*t + stateA.height;
    newState.tilt = (stateB.tilt-stateA.tilt)*t + stateA.tilt;
    newState.pos = MaplyCoordinateMake((stateB.pos.x-stateA.pos.x)*t + stateA.pos.x,(stateB.pos.y-stateA.pos.y)*t + stateA.pos.y);
    if (stateA.screenPos.x >= 0.0 && stateA.screenPos.y >= 0.0 &&
        stateB.screenPos.x >= 0.0 && stateB.screenPos.y >= 0.0)
    {
        newState.screenPos = CGPointMake((stateB.screenPos.x - stateA.screenPos.x)*t + stateA.screenPos.x,(stateB.screenPos.y - stateA.screenPos.y)*t + stateA.screenPos.y);
    } else
        newState.screenPos = stateB.screenPos;
    
    return newState;
}

@end

@implementation WhirlyGlobeViewControllerSimpleAnimationDelegate
{
    WhirlyGlobeViewControllerAnimationState *startState;
    WhirlyGlobeViewControllerAnimationState *endState;
    NSTimeInterval startTime,endTime;
}

- (id)initWithState:(WhirlyGlobeViewControllerAnimationState *)inEndState
{
    self = [super init];
    endState = inEndState;
    
    return self;
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC startState:(WhirlyGlobeViewControllerAnimationState *)inStartState startTime:(NSTimeInterval)inStartTime endTime:(NSTimeInterval)inEndTime
{
    startState = inStartState;
    if (!endState)
    {
        endState = [[WhirlyGlobeViewControllerAnimationState alloc] init];
        endState.heading = _heading;
        endState.height = _height;
        endState.tilt = _tilt;
        endState.pos = _loc;
    }
    startTime = inStartTime;
    endTime = inEndTime;
}

- (WhirlyGlobeViewControllerAnimationState *)globeViewController:(WhirlyGlobeViewController *)viewC stateForTime:(NSTimeInterval)currentTime
{
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    double t = (currentTime-startTime)/(endTime-startTime);
    if (t < 0.0)  t = 0.0;
    if (t > 1.0)  t = 1.0;
    state.heading = (endState.heading - startState.heading)*t + startState.heading;
    state.height = (endState.height - startState.height)*t + startState.height;
    state.tilt = (endState.tilt - startState.tilt)*t + startState.tilt;
    MaplyCoordinate pos;
    pos.x = (endState.pos.x - startState.pos.x)*t + startState.pos.x;
    pos.y = (endState.pos.y - startState.pos.y)*t + startState.pos.y;
    state.pos = pos;
    
    return state;
}

- (void)globeViewControllerDidFinishAnimation:(WhirlyGlobeViewController *)viewC
{
}

@end

@interface WhirlyGlobeViewController() <WGInteractionLayerDelegate,WhirlyGlobeAnimationDelegate>
@end

@implementation WhirlyGlobeViewController
{
    bool isPanning,isRotating,isZooming,isAnimating,isTilting;
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
    
    return self;
}

// Tear down layers and layer thread
- (void) clear
{
    [super clear];
    
    globeScene = NULL;
    globeView = nil;
    
    pinchDelegate = nil;
    panDelegate = nil;
    tiltDelegate = nil;
    tapDelegate = nil;
    rotateDelegate = nil;
    animateRotation = nil;    

    doubleTapDelegate = nil;
    twoFingerTapDelegate = nil;
    doubleTapDragDelegate = nil;
    
    delegateRespondsToViewUpdate = false;
}

- (void) dealloc
{
    if (globeScene)
        [self clear];
}

- (void)setDelegate:(NSObject<WhirlyGlobeViewControllerDelegate> *)delegate
{
    _delegate = delegate;
    delegateRespondsToViewUpdate = [_delegate respondsToSelector:@selector(globeViewController:didMove:)];
}

// Called by the globe view when something changes
- (void)viewUpdated:(WhirlyKitView *)view
{
    if (delegateRespondsToViewUpdate)
    {
        MaplyCoordinate corners[4];
        [self corners:corners forRot:globeView.rotQuat viewMat:[globeView calcViewMatrix]];
        [_delegate globeViewController:self didMove:corners];
    }
}

// Create the globe view
- (WhirlyKitView *) loadSetup_view
{
	globeView = [[WhirlyGlobeView alloc] init];
    globeView.continuousZoom = true;
    [globeView addWatcherDelegate:self];
    
    return globeView;
}

- (Scene *) loadSetup_scene
{
    globeScene = new WhirlyGlobe::GlobeScene(globeView.coordAdapter,4);
    sceneRenderer.theView = globeView;
    
    return globeScene;
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
    panDelegate = [PanDelegateFixed panDelegateForView:glView globeView:globeView];
    tapDelegate = [WhirlyGlobeTapDelegate tapDelegateForView:glView globeView:globeView];
    // These will activate the appropriate gesture
    self.pinchGesture = true;
    self.rotateGesture = true;
    self.tiltGesture = false;
        
    self.selection = true;
    
    if(_doubleTapZoomGesture)
    {
        doubleTapDelegate = [WhirlyGlobeDoubleTapDelegate doubleTapDelegateForView:glView globeView:globeView];
        doubleTapDelegate.minZoom = pinchDelegate.minHeight;
        doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
        doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
        doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
    }
    if(_twoFingerTapGesture)
    {
        twoFingerTapDelegate = [WhirlyGlobeTwoFingerTapDelegate twoFingerTapDelegateForView:glView globeView:globeView];
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
        doubleTapDragDelegate = [WhirlyGlobeDoubleTapDragDelegate doubleTapDragDelegateForView:glView globeView:globeView];
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
    [globeView runViewUpdates];
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
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sphericalEarthLayerLoaded:) name:kWhirlyGlobeSphericalEarthLoaded object:nil];
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
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(animationWillEnd:) name:kAnimateViewMomentum object:nil];
}

- (void)unregisterForEvents
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapOutsideMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWhirlyGlobeSphericalEarthLoaded object:nil];
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
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kAnimateViewMomentum object:nil];
}

/// Add a spherical earth layer with the given set of base images
- (WGViewControllerLayer *)addSphericalEarthLayerWithImageSet:(NSString *)name
{
    WGViewControllerLayer *newLayer = [[WGSphericalEarthWithTexGroup alloc] initWithWithLayerThread:baseLayerThread scene:globeScene texGroup:name];
    newLayer.drawPriority = layerDrawPriority++ + kMaplyImageLayerDrawPriorityDefault;
    if (!newLayer)
        return nil;
    
    [userLayers addObject:newLayer];
    
    return newLayer;
}

// Called when the earth layer finishes loading
- (void)sphericalEarthLayerLoaded:(NSNotification *)note
{
    WhirlyGlobeSphericalEarthLayer *layer = note.object;
    
    // Look for the matching layer
    if ([_delegate respondsToSelector:@selector(globeViewController:layerDidLoad:)])
        for (WGViewControllerLayer *userLayer in userLayers)
        {
            if ([userLayer isKindOfClass:[WGSphericalEarthWithTexGroup class]])
            {
                WGSphericalEarthWithTexGroup *userSphLayer = (WGSphericalEarthWithTexGroup *)userLayer;
                if (userSphLayer.earthLayer == layer)
                {
                    [_delegate globeViewController:self layerDidLoad:userLayer];
                    break;
                }
            }
        }
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

- (void)setPinchGesture:(bool)pinchGesture
{
    if (pinchGesture)
    {
        if (!pinchDelegate)
        {
            pinchDelegate = [WGPinchDelegateFixed pinchDelegateForView:glView globeView:globeView];
            pinchDelegate.zoomAroundPinch = true;
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
            [glView removeGestureRecognizer:pinchDelegate.gestureRecognizer];
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
            rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:globeView];
            rotateDelegate.rotateAroundCenter = true;
            pinchDelegate.rotateDelegate = rotateDelegate;
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:rotateDelegate.gestureRecognizer];
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
            tiltDelegate = [TiltDelegate tiltDelegateForView:glView globeView:globeView];
            tiltDelegate.pinchDelegate = pinchDelegate;
            tiltDelegate.tiltCalcDelegate = tiltControlDelegate;
            [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDelegate.gestureRecognizer];
            [tiltDelegate.gestureRecognizer requireGestureRecognizerToFail:twoFingerTapDelegate.gestureRecognizer];
            [tiltDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDragDelegate.gestureRecognizer];
        }
    } else {
        if (tiltDelegate)
        {
            [glView removeGestureRecognizer:tiltDelegate.gestureRecognizer];
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
    return globeView.heightAboveGlobe;
}

- (void)setHeight:(float)height
{
    globeView.heightAboveGlobe = height;
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
        if (globeView.heightAboveGlobe < minHeight)
            globeView.heightAboveGlobe = minHeight;
        if (globeView.heightAboveGlobe > maxHeight)
            globeView.heightAboveGlobe = maxHeight;

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
    [globeView setFarClippingPlane:farClipPlane];
}

- (void)setTiltMinHeight:(float)minHeight maxHeight:(float)maxHeight minTilt:(float)minTilt maxTilt:(float)maxTilt
{
    tiltControlDelegate = [[WGStandardTiltDelegate alloc] initWithGlobeView:globeView];
    [tiltControlDelegate setMinTilt:minTilt maxTilt:maxTilt minHeight:minHeight maxHeight:maxHeight];
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
    globeView.tilt = 0.0;
}

- (float)tilt
{
    return globeView.tilt;
}

- (void)setTilt:(float)newTilt
{
    globeView.tilt = newTilt;
}

- (void)setDoubleTapZoomGesture:(bool)doubleTapZoomGesture
{
    _doubleTapZoomGesture = doubleTapZoomGesture;
    if (doubleTapZoomGesture)
    {
        if (!doubleTapDelegate)
        {
            doubleTapDelegate = [WhirlyGlobeDoubleTapDelegate doubleTapDelegateForView:glView globeView:globeView];
            doubleTapDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
            doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
            doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
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
            twoFingerTapDelegate = [WhirlyGlobeTwoFingerTapDelegate twoFingerTapDelegateForView:glView globeView:globeView];
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
            doubleTapDragDelegate = [WhirlyGlobeDoubleTapDragDelegate doubleTapDragDelegateForView:glView globeView:globeView];
            doubleTapDragDelegate.minZoom = pinchDelegate.minHeight;
            doubleTapDragDelegate.maxZoom = pinchDelegate.maxHeight;
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

// Rotate to the given location over time
- (void)rotateToPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // If we were rotating from one point to another, stop
    [globeView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = [globeView makeRotationToGeoCoord:whereGeo keepNorthUp:panDelegate.northUp];
    
    // Rotate to the given position over time
    animateRotation = [[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:howLong];
    globeView.delegate = animateRotation;        
}

- (void)rotateToPointD:(Point2d)whereGeo time:(NSTimeInterval)howLong
{
    // If we were rotating from one point to another, stop
    [globeView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = [globeView makeRotationToGeoCoordD:whereGeo keepNorthUp:panDelegate.northUp];
    
    // Rotate to the given position over time
    animateRotation = [[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:howLong];
    globeView.delegate = animateRotation;
}

// External facing version of rotateToPoint
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong
{
    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return;
    }

    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:howLong];
}

// Figure out how to get the geolocation to the given point on the screen
- (bool)animateToPosition:(WGCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }

    [globeView cancelAnimation];
    
    // Figure out where that point lands on the globe
    Eigen::Matrix4d modelTrans = [globeView calcFullMatrix];
    Point3d whereLoc;
    if ([globeView pointOnSphereFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc normalized:true])
    {
        CoordSystemDisplayAdapter *coordAdapter = globeView.coordAdapter;
        Vector3d destPt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
        Eigen::Quaterniond endRot;
        endRot = QuatFromTwoVectors(destPt, whereLoc);
        Eigen::Quaterniond curRotQuat = globeView.rotQuat;
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
        animateRotation = [[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:howLong];
        globeView.delegate = animateRotation;
        
        return true;
    } else
        return false;
}

- (bool)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight heading:(float)newHeading time:(NSTimeInterval)howLong
{
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(newHeight))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to animationToPosition:");
        return false;
    }

    [globeView cancelAnimation];

    WhirlyGlobeViewControllerSimpleAnimationDelegate *anim = [[WhirlyGlobeViewControllerSimpleAnimationDelegate alloc] init];
    anim.loc = newPos;
    anim.heading = newHeading;
    anim.height = newHeight;
    anim.tilt = [self tilt];
    
    [self animateWithDelegate:anim time:howLong];
    
    return true;
}

// External facing set position
- (void)setPosition:(WGCoordinate)newPos
{
    if (isnan(newPos.x) || isnan(newPos.y))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }
    
    // Note: This might conceivably be a problem, though I'm not sure how.
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:0.0];
    // If there's a pinch delegate, ask it to calculate the height.
    if (tiltControlDelegate)
        self.tilt = [tiltControlDelegate tiltFromHeight:globeView.heightAboveGlobe];
 }

- (void)setPosition:(WGCoordinate)newPos height:(float)height
{
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(height))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }

    [self setPosition:newPos];
    globeView.heightAboveGlobe = height;
}

- (void)setPositionD:(MaplyCoordinateD)newPos height:(double)height
{
    if (isnan(newPos.x) || isnan(newPos.y) || isnan(height))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid location passed to setPosition:");
        return;
    }
    
    // Note: This might conceivably be a problem, though I'm not sure how.
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:0.0];
    globeView.heightAboveGlobe = height;
    // If there's a pinch delegate, ask it to calculate the height.
    if (tiltControlDelegate)
        self.tilt = [tiltControlDelegate tiltFromHeight:globeView.heightAboveGlobe];
}

- (void)setHeading:(float)heading
{
    if (isnan(heading))
    {
        NSLog(@"WhirlyGlobeViewController: Invalid heading passed to setHeading:");
        return;
    }

    // Undo the current heading
    Point3d localPt = [globeView currentUp];
    Vector3d northPole = (globeView.rotQuat * Vector3d(0,0,1)).normalized();
    Quaterniond posQuat = globeView.rotQuat;
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
    
    globeView.rotQuat = newRotQuat;
}

- (float)heading
{
    float retHeading = 0.0;

    // Figure out where the north pole went
    Vector3d northPole = (globeView.rotQuat * Vector3d(0,0,1)).normalized();
    if (northPole.y() != 0.0)
        retHeading = atan2(-northPole.x(),northPole.y());
    
    return retHeading;
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    *height = globeView.heightAboveGlobe;
    Point3d localPt = [globeView currentUp];
    GeoCoord geoCoord = globeView.coordAdapter->getCoordSystem()->localToGeographic(globeView.coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.lon();  pos->y = geoCoord.lat();
}

- (void)getPositionD:(MaplyCoordinateD *)pos height:(double *)height
{
    *height = globeView.heightAboveGlobe;
    Point3d localPt = [globeView currentUp];
    Point2d geoCoord = globeView.coordAdapter->getCoordSystem()->localToGeographicD(globeView.coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSArray *)selectedObjs
{
    WGCoordinate coord;
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
    if (msg.view != glView)
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
    if (msg.view != glView)
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
    CGPoint screenCorners[4];
    screenCorners[0] = CGPointMake(0.0, 0.0);
    screenCorners[1] = CGPointMake(sceneRenderer.framebufferWidth,0.0);
    screenCorners[2] = CGPointMake(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    screenCorners[3] = CGPointMake(0.0, sceneRenderer.framebufferHeight);
    
    Eigen::Matrix4d modelTrans;
    // Note: Pulled this calculation out of the globe view.
    Eigen::Affine3d trans(Eigen::Translation3d(0,0,-[globeView calcEarthZOffset]));
    Eigen::Affine3d rot(theRot);
    Eigen::Matrix4d modelMat = (trans * rot).matrix();
    
    modelTrans = viewMat * modelMat;

    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3d hit;
        if ([globeView pointOnSphereFromScreen:screenCorners[ii] transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight) hit:&hit normalized:true])
        {
            Point3d geoHit = scene->getCoordAdapter()->displayToLocal(hit);
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
    [self corners:corners forRot:globeView.rotQuat viewMat:[globeView calcViewMatrix]];

    [_delegate globeViewController:self didStopMoving:corners userMotion:userMotion];
}

// Called when the tilt delegate starts moving
- (void) tiltDidStart:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
    [self handleStartMoving:true];
    isTilting = true;
}

// Called when the tilt delegate stops moving
- (void) tiltDidEnd:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
    isTilting = false;
    [self handleStopMoving:true];
}

// Called when the pan delegate starts moving
- (void) panDidStart:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Pan started");

    [self handleStartMoving:true];
    isPanning = true;
}

// Called when the pan delegate stops moving
- (void) panDidEnd:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Pan ended");
    
    isPanning = false;
    [self handleStopMoving:true];
}

- (void) pinchDidStart:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Pinch started");
    
    [self handleStartMoving:true];
    isZooming = true;
}

- (void) pinchDidEnd:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Pinch ended");

    isZooming = false;
    [self handleStopMoving:true];
}

- (void) rotateDidStart:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Rotate started");
    
    [self handleStartMoving:true];
    isRotating = true;
}

- (void) rotateDidEnd:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Rotate ended");
    
    isRotating = false;
    [self handleStopMoving:true];
}

- (void) animationDidStart:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Animation started");

    [self handleStartMoving:false];
    isAnimating = true;
}

- (void) animationDidEnd:(NSNotification *)note
{
    if (note.object != globeView)
        return;
    
//    NSLog(@"Animation ended");
    
    // Momentum animation is only kicked off by the pan delegate.
    bool userMotion = false;
    if ([globeView.delegate isKindOfClass:[AnimateViewMomentum class]])
        userMotion = true;
    
    isAnimating = false;
    knownAnimateEndRot = false;
    [self handleStopMoving:userMotion];
}

- (void) animationWillEnd:(NSNotification *)note
{
    AnimateViewMomentumMessage *info = note.object;
    if (![info isKindOfClass:[AnimateViewMomentumMessage class]])
        return;

//    NSLog(@"Animation will end");

    knownAnimateEndRot = true;
    animateEndRot = info.rot;

    if (!isRotating && !isZooming)
    {
        if ([_delegate respondsToSelector:@selector(globeViewController:willStopMoving:userMotion:)])
        {
            MaplyCoordinate corners[4];
            if (knownAnimateEndRot)
            {
                [self corners:corners forRot:animateEndRot viewMat:[globeView calcViewMatrix]];
                [_delegate globeViewController:self willStopMoving:corners userMotion:true];
            }
        }
    }
}

// See if the given bounding box is all on sreen
- (bool)checkCoverage:(Mbr &)mbr globeView:(WhirlyGlobeView *)theView height:(float)height
{
    [globeView setHeightAboveGlobe:height updateWatchers:false];

    std::vector<Point2f> pts;
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
    WhirlyGlobeView *tempGlobe = [[WhirlyGlobeView alloc] initWithGlobeView:globeView];
    
    float oldHeight = globeView.heightAboveGlobe;
    Eigen::Quaterniond newRotQuat = [tempGlobe makeRotationToGeoCoord:GeoCoord(pos.x,pos.y) keepNorthUp:YES];
    [tempGlobe setRotQuat:newRotQuat updateWatchers:false];

    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    
    float minHeight = tempGlobe.minHeightAboveGlobe;
    float maxHeight = tempGlobe.maxHeightAboveGlobe;
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minHeight);
        maxHeight = std::min(maxHeight,pinchDelegate.maxHeight);
    }

    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr globeView:tempGlobe height:minHeight];
    bool maxOnScreen = [self checkCoverage:mbr globeView:tempGlobe height:maxHeight];
    if (!minOnScreen && !maxOnScreen)
    {
        [tempGlobe setHeightAboveGlobe:oldHeight updateWatchers:false];
        return oldHeight;
    }
    
    // Now for the binary search
    // Note: I'd rather make a copy of the view first
    float minRange = 1e-5;
    do
    {
        float midHeight = (minHeight + maxHeight)/2.0;
        bool midOnScreen = [self checkCoverage:mbr globeView:tempGlobe height:midHeight];
        
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

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    Point3d pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = [visualView calcFullMatrix];
    return [globeView pointOnScreenFromSphere:pt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
}

- (bool)screenPointFromGeo:(MaplyCoordinate)geoCoord screenPt:(CGPoint *)screenPt
{
    Point3d pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    Point3f pt3f(pt.x(),pt.y(),pt.z());
    
    Eigen::Matrix4d modelTrans4d = [visualView calcModelMatrix];
    Eigen::Matrix4d viewTrans4d = [visualView calcViewMatrix];
    Eigen::Matrix4d modelAndViewMat4d = viewTrans4d * modelTrans4d;
    Eigen::Matrix4f modelAndViewMat = Matrix4dToMatrix4f(modelAndViewMat4d);
    Eigen::Matrix4f modelAndViewNormalMat = modelAndViewMat.inverse().transpose();

    if (CheckPointAndNormFacing(pt3f,pt3f.normalized(),modelAndViewMat,modelAndViewNormalMat) < 0.0)
        return false;
    
    *screenPt =  [globeView pointOnScreenFromSphere:pt transform:&modelAndViewMat4d frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
    
    if (screenPt->x < 0 || screenPt->y < 0 || screenPt->x > sceneRenderer.framebufferWidth || screenPt->y > sceneRenderer.framebufferHeight)
        return false;
    
    return true;
}

- (bool)geoPointFromScreen:(CGPoint)screenPt geoCoord:(MaplyCoordinate *)retCoord
{
	Point3d hit;
	Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
    if ([globeView pointOnSphereFromScreen:screenPt transform:&theTransform frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
    {
        GeoCoord geoCoord = visualView.coordAdapter->getCoordSystem()->localToGeographic(visualView.coordAdapter->displayToLocal(hit));
        retCoord->x = geoCoord.x();
        retCoord->y = geoCoord.y();
        
        return true;
	} else
        return false;
}

- (bool)geocPointFromScreen:(CGPoint)screenPt geocCoord:(double *)retCoords
{
	Point3d hit;
	Eigen::Matrix4d theTransform = [globeView calcFullMatrix];
    if ([globeView pointOnSphereFromScreen:screenPt transform:&theTransform frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
    {
        Point3d geoC = visualView.coordAdapter->getCoordSystem()->localToGeocentric(visualView.coordAdapter->displayToLocal(hit));
        retCoords[0] = geoC.x();  retCoords[1] = geoC.y();  retCoords[2] = geoC.z();
        
        // Note: Obviously doing something stupid here
        if (isnan(retCoords[0]) || isnan(retCoords[1]) || isnan(retCoords[2]))
            return false;
        
        return true;
	} else
        return false;
}

// Note: Finish writing this
- (id)findObjectAtLocation:(CGPoint)screenPt
{
    // Look for the object, returns an ID
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    SimpleIdentity objId = selectManager->pickObject(Point2f(screenPt.x,screenPt.y), 10.0, globeView);
    
    if (objId != EmptyIdentity)
    {
        // Now ask the interaction layer for the right object
        return [interactLayer getSelectableObject:objId];
    }
    
    return nil;
}

#pragma mark - WhirlyGlobeAnimationDelegate

// Called every frame from within the globe view
- (void)updateView:(WhirlyGlobeView *)inGlobeView
{
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    if (!animationDelegate)
    {
        [globeView cancelAnimation];
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
        [globeView cancelAnimation];
        if ([animationDelegate respondsToSelector:@selector(globeViewControllerDidFinishAnimation:)])
            [animationDelegate globeViewControllerDidFinishAnimation:self];
        animationDelegate = nil;
    }
}

- (void)animateWithDelegate:(NSObject<WhirlyGlobeViewControllerAnimationDelegate> *)inAnimationDelegate time:(NSTimeInterval)howLong
{
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    animationDelegate = inAnimationDelegate;
    animationDelegateEnd = now+howLong;

    WhirlyGlobeViewControllerAnimationState *stateStart = [self getViewState];
    
    // Tell the delegate what we're up to
    [animationDelegate globeViewController:self startState:stateStart startTime:now endTime:animationDelegateEnd];
    
    globeView.delegate = self;
}

- (void)setViewState:(WhirlyGlobeViewControllerAnimationState *)animState
{
    [globeView cancelAnimation];
    [self setViewStateInternal:animState];
}

- (void)setViewStateInternal:(WhirlyGlobeViewControllerAnimationState *)animState
{
    Vector3d startLoc(0,0,1);
    
    if (animState.screenPos.x >= 0.0 && animState.screenPos.y >= 0.0)
    {
        Matrix4d heightTrans = Eigen::Affine3d(Eigen::Translation3d(0,0,-[globeView calcEarthZOffset])).matrix();
        Point3d hit;
        if ([globeView pointOnSphereFromScreen:animState.screenPos transform:&heightTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&hit normalized:true])
        {
            startLoc = hit;
        }
    }
    
    // Start with a rotation from the clean start state to the location
    Point3d worldLoc = globeView.coordAdapter->localToDisplay(globeView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y)));
    Eigen::Quaterniond posRot = QuatFromTwoVectors(worldLoc, startLoc);
    
    // Orient with north up.  Either because we want that or we're about do do a heading
    Eigen::Quaterniond posRotNorth = posRot;
    if (panDelegate.northUp || animState.heading != MAXFLOAT)
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
    if (!panDelegate.northUp && animState.heading != MAXFLOAT)
    {
        Eigen::AngleAxisd headingRot(animState.heading,worldLoc);
        finalQuat = posRotNorth * headingRot;
    }
    
    // Set the height (easy)
    [globeView setHeightAboveGlobe:animState.height updateWatchers:false];
    
    // Set the tilt either directly or as a consequence of the height
    if (animState.tilt == MAXFLOAT)
        globeView.tilt = [tiltControlDelegate tiltFromHeight:globeView.heightAboveGlobe];
    else
        globeView.tilt = animState.tilt;
    
    globeView.rotQuat = finalQuat;
}

- (WhirlyGlobeViewControllerAnimationState *)getViewState
{
    // Figure out the current state
    WhirlyGlobeViewControllerAnimationState *state = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    startQuat = globeView.rotQuat;
    startUp = [globeView currentUp];
    state.heading = self.heading;
    state.tilt = self.tilt;
    MaplyCoordinate pos;
    float height;
    [self getPosition:&pos height:&height];
    state.pos = pos;
    state.height = height;
    
    return state;
}

- (WhirlyGlobeViewControllerAnimationState *)viewStateForLookAt:(MaplyCoordinate)coord tilt:(float)tilt heading:(float)heading altitude:(float)alt range:(float)range
{
    Vector3f north(0,0,1);
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter = globeView.coordAdapter;
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
    state.pos = MaplyCoordinateMake(outGeoCoord.lon(), outGeoCoord.lat());
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
        viewState.tilt = [tiltControlDelegate tiltFromHeight:viewState.height];
    }
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

// Note: Should base this on the 
static const float LonAng = 2*M_PI/5.0;
static const float LatAng = M_PI/4.0;

// Can't represent the whole earth with -M_PI/+M_PI.  Have to fudge.
static const float FullExtentEps = 1e-5;

- (int)getUsableGeoBoundsForView:(MaplyBoundingBox *)bboxes visual:(bool)visualBoxes
{
    float extentEps = visualBoxes ? FullExtentEps : 0.0;
    
    CGPoint screenCorners[4];
    screenCorners[0] = CGPointMake(0.0, 0.0);
    screenCorners[1] = CGPointMake(sceneRenderer.framebufferWidth,0.0);
    screenCorners[2] = CGPointMake(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight);
    screenCorners[3] = CGPointMake(0.0, sceneRenderer.framebufferHeight);
    
    Eigen::Matrix4d modelTrans = [globeView calcFullMatrix];

    Point3d corners[4];
    bool cornerValid[4];
    int numValid = 0;;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3d hit;
        if ([globeView pointOnSphereFromScreen:screenCorners[ii] transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight) hit:&hit normalized:true])
        {
            corners[ii] = scene->getCoordAdapter()->displayToLocal(hit);
            cornerValid[ii] = true;
            numValid++;
        } else {
            cornerValid[ii] = false;
        }
    }
    
    // Current location the user is over
    Point3d localPt = [globeView currentUp];
    GeoCoord currentLoc = globeView.coordAdapter->getCoordSystem()->localToGeographic(globeView.coordAdapter->displayToLocal(localPt));

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
                    CGPoint screenPt = CGPointMake(bi*(screenCorners[a].x+screenCorners[b].x)/numSamples, bi*(screenCorners[a].y+screenCorners[b].y)/numSamples);
                    Point3d hit;
                    if ([globeView pointOnSphereFromScreen:screenPt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight) hit:&hit normalized:true]) {
                        Point3d midPt3d = scene->getCoordAdapter()->displayToLocal(hit);
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
                    CGPoint testPts[2];
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
                        CGPoint midPt = CGPointMake((testPts[0].x+testPts[1].x)/2, (testPts[0].y+testPts[1].y)/2);
                        Point3d hit;
                        if ([globeView pointOnSphereFromScreen:midPt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight) hit:&hit normalized:true])
                        {
                            testPts[0] = midPt;
                        } else {
                            testPts[1] = midPt;
                        }
                    }
                    
                    // The first test point is valid, so let's convert that back
                    Point3d hit;
                    if ([globeView pointOnSphereFromScreen:testPts[0] transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth,sceneRenderer.framebufferHeight) hit:&hit normalized:true])
                    {
                        Point3d midPt3d = scene->getCoordAdapter()->displayToLocal(hit);
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
                
                CGPoint screenPt = [globeView pointOnScreenFromSphere:pt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
            
                if (screenPt.x < 0 || screenPt.y < 0 || screenPt.x > sceneRenderer.framebufferWidth || screenPt.y > sceneRenderer.framebufferHeight)
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
    
    return mbrs.size();
}

@end
