
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

@interface WhirlyGlobeViewController() <WGInteractionLayerDelegate>
@end

@implementation WhirlyGlobeViewController
{
}

- (id) init
{
    self = [super init];
    if (!self)
        return nil;
    
    _autoMoveToTap = true;
    _zoomInOnDoubleTap = false;
    return self;
}

// Tear down layers and layer thread
- (void) clear
{
    [super clear];
    
    globeScene = NULL;
    globeView = nil;
    touchDelegate = nil;
    pinchDelegate = nil;
    panDelegate = nil;
    tapDelegate = nil;
    rotateDelegate = nil;
    animateRotation = nil;
    animateZoom = nil;
}

- (void) dealloc
{
    if (globeScene)
        [self clear];
}

// Create the globe view
- (WhirlyKitView *) loadSetup_view
{
    globeView = [[WhirlyGlobeView alloc] init];
    globeView.continuousZoom = true;
    
    return globeView;
}

- (Scene *) loadSetup_scene
{
    globeScene = new WhirlyGlobe::GlobeScene(4);
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
    touchDelegate = [TouchDelegateFixed touchDelegateForView:glView globeView:globeView];
    
    
    // These will activate the appropriate gesture
    self.pinchGesture = true;
    self.rotateGesture = true;
    
    self.selection = true;
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
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:WhirlyGlobeTapOutsideMsg object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kWhirlyGlobeSphericalEarthLoaded object:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
}

// Register for interesting tap events and others
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnGlobe:) name:WhirlyGlobeTapMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOutsideGlobe:) name:WhirlyGlobeTapOutsideMsg object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sphericalEarthLayerLoaded:) name:kWhirlyGlobeSphericalEarthLoaded object:nil];
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
            pinchDelegate = [WGPinchDelegateFixed pinchDelegateForView:glView globeView:globeView];
    } else {
        if (pinchDelegate)
        {
            UIPinchGestureRecognizer *pinchRecog = nil;
            for (UIGestureRecognizer *recog in glView.gestureRecognizers)
                if ([recog isKindOfClass:[UIPinchGestureRecognizer class]])
                    pinchRecog = (UIPinchGestureRecognizer *)recog;
            [glView removeGestureRecognizer:pinchRecog];
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
            rotateDelegate = [WhirlyGlobeRotateDelegate rotateDelegateForView:glView globeView:globeView];
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
    }
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

#pragma mark - Interaction




//// Zoom to the given location over time
//- (void)animateZoomToHeight: (float) heightAboveGlobe onPosition:(WGCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
//{
//
//    [globeView cancelAnimation];
//
//    // Figure out where that points lands on the globe
//    Eigen::Matrix4d modelTrans = [globeView calcFullMatrix];
//    Point3d whereLoc;
//    if ([globeView pointOnSphereFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc normalized:true])
//    {
//        CoordSystemDisplayAdapter *coordAdapter = globeView.coordAdapter;
//        Vector3d destPt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(newPos.x,newPos.y)));
//        Eigen::Quaterniond endRot;
//        endRot = QuatFromTwoVectors(destPt, whereLoc);
//        Eigen::Quaterniond curRotQuat = globeView.rotQuat;
//        Eigen::Quaterniond newRotQuat = curRotQuat * endRot;
//
//        if (panDelegate.northUp)
//        {
//            // We'd like to keep the north pole pointed up
//            // So we look at where the north pole is going
//            Vector3d northPole = (newRotQuat * Vector3d(0,0,1)).normalized();
//            if (northPole.y() != 0.0)
//            {
//                // Then rotate it back on to the YZ axis
//                // This will keep it upward
//                float ang = atan(northPole.x()/northPole.y());
//                // However, the pole might be down now
//                // If so, rotate it back up
//                if (northPole.y() < 0.0)
//                    ang += M_PI;
//                Eigen::AngleAxisd upRot(ang,destPt);
//                newRotQuat = newRotQuat * upRot;
//            }
//        }
//
//
//        // Zoom and rotate to the given position over time
//        animateZoom = [[AnimateZoom alloc] initWithView:globeView zoomHeightAboveGlobe:heightAboveGlobe rot:newRotQuat howLong:howLong];
//
//        globeView.delegate = animateZoom;
//
//    }
//}



// External facing version of zoomOnHeight
- (void)animateZoomHeight: (float)heightAboveGlobe ToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong
{
    [self zoomOnHeight:heightAboveGlobe toPoint:GeoCoord(newPos.x,newPos.y) time:howLong];
}

// Zoom to the given location over time
- (void)zoomOnHeight: (float)heightAboveGlobe toPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // If we were rotating from one point to another, stop
    [globeView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = [globeView makeRotationToGeoCoord:whereGeo keepNorthUp:YES];
    
    
    // Zoom and rotate to the given position over time
    animateZoom = [[AnimateZoom alloc] initWithView:globeView zoomHeightAboveGlobe:heightAboveGlobe rot:newRotQuat howLong:howLong];
    
    globeView.delegate = animateZoom;
    
}



// External facing version of rotateToPoint
- (void)animateToPosition:(WGCoordinate)newPos time:(NSTimeInterval)howLong
{
    [self rotateToPoint:GeoCoord(newPos.x,newPos.y) time:howLong];
}

// Rotate to the given location over time
- (void)rotateToPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // If we were rotating from one point to another, stop
    [globeView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaterniond newRotQuat = [globeView makeRotationToGeoCoord:whereGeo keepNorthUp:YES];
    
    
    // Rotate to the given position over time
    animateRotation = [[AnimateViewRotation alloc] initWithView:globeView rot:newRotQuat howLong:howLong];
    globeView.delegate = animateRotation;
}






// Figure out how to get the geolocation to the given point on the screen
- (void)animateToPosition:(WGCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
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
    }
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
    Point3d localPt = [globeView currentUp];
    Eigen::AngleAxisd rot(heading,localPt);
    Quaterniond newRotQuat = globeView.rotQuat * rot;
    
    globeView.rotQuat = newRotQuat;
}

- (float)heading
{
    float retHeading = 0.0;
    
    // Figure out where the north pole went
    Vector3d northPole = (globeView.rotQuat * Vector3d(0,0,1)).normalized();
    if (northPole.y() != 0.0)
        retHeading = atan(northPole.x()/northPole.y());
    
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
        
        //if the user used double tap
        if(msg.noOfTaps==2 && _zoomInOnDoubleTap)
        {
            
            float howMuchToZoom = msg.heightAboveSurface*0.5;
            [self zoomOnHeight:howMuchToZoom toPoint:msg.whereGeo time:0.3];
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

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    Point3d pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4d modelTrans = [visualView calcFullMatrix];
    return [globeView pointOnScreenFromSphere:pt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
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

- (void)setSunDirection:(MaplyCoordinate3d)sunDir
{
    
}

- (void)clearSunDirection
{
    
}

@end
