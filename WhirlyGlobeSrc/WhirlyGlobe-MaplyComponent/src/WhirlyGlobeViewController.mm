/*
 *  WhirlyGlobeViewController.mm
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011-2012 mousebird consulting
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

@synthesize delegate;

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
}

- (void) dealloc
{
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
    // These will activate the appropriate gesture
    self.pinchGesture = true;
    self.rotateGesture = true;
        
    selection = true;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    [self registerForEvents];
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
    WGViewControllerLayer *newLayer = [[WGSphericalEarthWithTexGroup alloc] initWithWithLayerThread:layerThread scene:globeScene texGroup:name];
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
    if ([delegate respondsToSelector:@selector(globeViewController:layerDidLoad:)])
        for (WGViewControllerLayer *userLayer in userLayers)
        {
            if ([userLayer isKindOfClass:[WGSphericalEarthWithTexGroup class]])
            {
                WGSphericalEarthWithTexGroup *userSphLayer = (WGSphericalEarthWithTexGroup *)userLayer;
                if (userSphLayer.earthLayer == layer)
                {
                    [delegate globeViewController:self layerDidLoad:userLayer];
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

#pragma mark - Interaction

// Rotate to the given location over time
- (void)rotateToPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // If we were rotating from one point to another, stop
    [globeView cancelAnimation];
    
    // Construct a quaternion to rotate from where we are to where
    //  the user tapped
    Eigen::Quaternionf newRotQuat = [globeView makeRotationToGeoCoord:whereGeo keepNorthUp:YES];
    
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
- (void)animateToPosition:(WGCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    [globeView cancelAnimation];
    
    // Figure out where that points lands on the globe
    Eigen::Matrix4f modelTrans = [globeView calcFullMatrix];
    Point3f whereLoc;
    if ([globeView pointOnSphereFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc])
    {
        CoordSystemDisplayAdapter *coordAdapter = globeView.coordAdapter;
        Vector3f destPt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
        Eigen::Quaternionf endRot;
        endRot = QuatFromTwoVectors(destPt, whereLoc);
        Eigen::Quaternionf curRotQuat = globeView.rotQuat;
        Eigen::Quaternionf newRotQuat = curRotQuat * endRot;
        
        if (panDelegate.northUp)
        {
            // We'd like to keep the north pole pointed up
            // So we look at where the north pole is going
            Vector3f northPole = (newRotQuat * Vector3f(0,0,1)).normalized();
            if (northPole.y() != 0.0)
            {
                // Then rotate it back on to the YZ axis
                // This will keep it upward
                float ang = atanf(northPole.x()/northPole.y());
                // However, the pole might be down now
                // If so, rotate it back up
                if (northPole.y() < 0.0)
                    ang += M_PI;
                Eigen::AngleAxisf upRot(ang,destPt);
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
}

- (void)setPosition:(WGCoordinate)newPos height:(float)height
{
    [self setPosition:newPos];
    globeView.heightAboveGlobe = height;
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    *height = globeView.heightAboveGlobe;
    Point3f localPt = [globeView currentUp];
    GeoCoord geoCoord = globeView.coordAdapter->getCoordSystem()->localToGeographic(globeView.coordAdapter->displayToLocal(localPt));
    pos->x = geoCoord.lon();  pos->y = geoCoord.lat();
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(WhirlyGlobeTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    if (selectedObj && selection)
    {
        // The user selected something, so let the delegate know
        if (delegate && [delegate respondsToSelector:@selector(globeViewController:didSelect:)])
            [delegate globeViewController:self didSelect:selectedObj];
    } else {
        // The user didn't select anything, let the delegate know.
        if (delegate && [delegate respondsToSelector:@selector(globeViewController:didTapAt:)])
        {
            WGCoordinate coord;
            coord.x = msg.whereGeo.lon();
            coord.y = msg.whereGeo.lat();
            [delegate globeViewController:self didTapAt:coord];
        }
        // Didn't select anything, so rotate
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
    if (selection && delegate && [delegate respondsToSelector:@selector(globeViewControllerDidTapOutside:)])
        [delegate globeViewControllerDidTapOutside:self];
}

- (CGPoint)screenPointFromGeo:(MaplyCoordinate)geoCoord
{
    Point3f pt = visualView.coordAdapter->localToDisplay(visualView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(geoCoord.x,geoCoord.y)));
    
    Eigen::Matrix4f modelTrans = [visualView calcFullMatrix];
    return [globeView pointOnScreenFromSphere:pt transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor)];
}

@end
