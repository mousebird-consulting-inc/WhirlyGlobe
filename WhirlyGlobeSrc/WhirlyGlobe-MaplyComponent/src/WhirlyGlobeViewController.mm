/*
 *  WhirlyGlobeViewController.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2013 mousebird consulting
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
    
    return self;
}

@end

@interface WhirlyGlobeViewController() <WGInteractionLayerDelegate,WhirlyGlobeAnimationDelegate>
@end

@implementation WhirlyGlobeViewController
{
    bool isPanning,isRotating,isZooming,isAnimating;
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
        
    self.selection = true;
    
    if(_doubleTapZoomGesture)
    {
        doubleTapDelegate = [WhirlyGlobeDoubleTapDelegate doubleTapDelegateForView:glView globeView:globeView];
        doubleTapDelegate.minZoom = pinchDelegate.minHeight;
        doubleTapDelegate.maxZoom = pinchDelegate.maxHeight;
        doubleTapDelegate.zoomTapFactor = _zoomTapFactor;
        doubleTapDelegate.zoomAnimationDuration = _zoomTapAnimationDuration;
        [tapDelegate.gestureRecognizer requireGestureRecognizerToFail:doubleTapDelegate.gestureRecognizer];
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
        }
    } else {
        if (pinchDelegate)
        {
            [glView removeGestureRecognizer:pinchDelegate.gestureRecognizer];
            pinchDelegate = nil;
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

- (void)setTiltMinHeight:(float)minHeight maxHeight:(float)maxHeight minTilt:(float)minTilt maxTilt:(float)maxTilt
{
    if (pinchDelegate)
        [pinchDelegate setMinTilt:minTilt maxTilt:maxTilt minHeight:minHeight maxHeight:maxHeight];
}

/// Turn off varying tilt by height
- (void)clearTiltHeight
{
    if (pinchDelegate)
        [pinchDelegate clearTiltZoom];
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

// External facing version of rotateToPoint
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong
{
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:howLong];
}

// Figure out how to get the geolocation to the given point on the screen
- (bool)animateToPosition:(WGCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    [globeView cancelAnimation];
    
    // Figure out where that points lands on the globe
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

// External facing set position
- (void)setPosition:(WGCoordinate)newPos
{
    // Note: This might conceivably be a problem, though I'm not sure how.
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:0.0];
    // If there's a pinch delegate, ask it to calculate the height.
    if (pinchDelegate)
    {
        self.tilt = [pinchDelegate calcTilt];
    }
}

- (void)setPosition:(WGCoordinate)newPos height:(float)height
{
    [self setPosition:newPos];
    globeView.heightAboveGlobe = height;
}

- (void)setHeading:(float)heading
{
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

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    WGCoordinate coord;
    coord.x = msg.whereGeo.lon();
    coord.y = msg.whereGeo.lat();

    if (selectedObj && self.selection)
    {
        // The user selected something, so let the delegate know
        if (_delegate && [_delegate respondsToSelector:@selector(globeViewController:didSelect:atLoc:onScreen:)])
            [_delegate globeViewController:self didSelect:selectedObj atLoc:coord onScreen:msg.touchLoc];
        else if (_delegate && [_delegate respondsToSelector:@selector(globeViewController:didSelect:)])
            [_delegate globeViewController:self didSelect:selectedObj];
    } else {
        // The user didn't select anything, let the delegate know.
        if (_delegate && [_delegate respondsToSelector:@selector(globeViewController:didTapAt:)])
        {
            [_delegate globeViewController:self didTapAt:coord];
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
    
    // Hand this over to the interaction layer to look for a selection
    // If there is no selection, it will call us back in the main thread
    [globeInteractLayer userDidTap:msg];
}

// Called when the user taps outside the globe.
- (void) tapOutsideGlobe:(NSNotification *)note
{
    if (self.selection && _delegate && [_delegate respondsToSelector:@selector(globeViewControllerDidTapOutside:)])
        [_delegate globeViewControllerDidTapOutside:self];
}

- (void) handleStartMoving:(bool)userMotion
{
    if (!isPanning && !isRotating && !isZooming && !isAnimating)
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
    if (isPanning || isRotating || isZooming || isAnimating)
        return;
    
    if (![_delegate respondsToSelector:@selector(globeViewController:didStopMoving:userMotion:)])
        return;
    
    MaplyCoordinate corners[4];
    [self corners:corners forRot:globeView.rotQuat viewMat:[globeView calcViewMatrix]];

    [_delegate globeViewController:self didStopMoving:corners userMotion:userMotion];
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
    float oldHeight = globeView.heightAboveGlobe;

    Eigen::Quaterniond oldRotQuat = globeView.rotQuat;
    Eigen::Quaterniond newRotQuat = [globeView makeRotationToGeoCoord:GeoCoord(pos.x,pos.y) keepNorthUp:YES];
    [globeView setRotQuat:newRotQuat updateWatchers:false];

    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    
    float minHeight = globeView.minHeightAboveGlobe;
    float maxHeight = globeView.maxHeightAboveGlobe;
    if (pinchDelegate)
    {
        minHeight = std::max(minHeight,pinchDelegate.minHeight);
        maxHeight = std::min(maxHeight,pinchDelegate.maxHeight);
    }

    // Check that we can at least see it
    bool minOnScreen = [self checkCoverage:mbr globeView:globeView height:minHeight];
    bool maxOnScreen = [self checkCoverage:mbr globeView:globeView height:maxHeight];
    if (!minOnScreen && !maxOnScreen)
    {
        [globeView setHeightAboveGlobe:oldHeight updateWatchers:false];
        return oldHeight;
    }
    
    // Now for the binary search
    // Note: I'd rather make a copy of the view first
    float minRange = 1e-5;
    do
    {
        float midHeight = (minHeight + maxHeight)/2.0;
        bool midOnScreen = [self checkCoverage:mbr globeView:globeView height:midHeight];
        
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
    
    [globeView setHeightAboveGlobe:oldHeight updateWatchers:false];
    [globeView setRotQuat:oldRotQuat updateWatchers:false];
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
    
    // Start with a rotation from the clean start state to the location
    Point3d worldLoc = globeView.coordAdapter->localToDisplay(globeView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(animState.pos.x,animState.pos.y)));
    Eigen::Quaterniond posRot = QuatFromTwoVectors(worldLoc, Vector3d(0,0,1));
    
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
        globeView.tilt = [pinchDelegate calcTilt];
    else
        globeView.tilt = animState.tilt;
    
    globeView.rotQuat = finalQuat;
    
    if (lastOne)
    {
        [globeView cancelAnimation];
        animationDelegate = nil;
    }
}

- (void)animateWithDelegate:(NSObject<WhirlyGlobeViewControllerAnimationDelegate> *)inAnimationDelegate time:(NSTimeInterval)howLong
{
    NSTimeInterval now = CFAbsoluteTimeGetCurrent();
    animationDelegate = inAnimationDelegate;
    animationDelegateEnd = now+howLong;

    // Figure out the current state
    WhirlyGlobeViewControllerAnimationState *stateStart = [[WhirlyGlobeViewControllerAnimationState alloc] init];
    startQuat = globeView.rotQuat;
    startUp = [globeView currentUp];
    stateStart.heading = self.heading;
    stateStart.tilt = self.tilt;
    MaplyCoordinate pos;
    float height;
    [self getPosition:&pos height:&height];
    stateStart.pos = pos;
    stateStart.height = height;
    
    // Tell the delegate what we're up to
    [animationDelegate globeViewController:self startState:stateStart startTime:now endTime:animationDelegateEnd];
    
    globeView.delegate = self;
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


@end
