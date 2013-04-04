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
#import "MaplyQuadEarthWithMBTiles_private.h"
#import "MaplyQuadEarthWithRemoteTiles_private.h"
#import "MaplyInteractionLayer_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@interface MaplyViewController () <MaplyInteractionLayerDelegate>
@end

@implementation MaplyViewController

@synthesize delegate;

// Tear down layers and layer thread
- (void) clear
{
    [layerThread addThingToDelete:coordAdapter];

    [super clear];
    
    mapScene = NULL;
    mapView = nil;

    mapInteractLayer = nil;
    
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;

    coordAdapter = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (WhirlyKitView *) loadSetup_view
{
    coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
    mapView = [[MaplyView alloc] initWithCoordAdapater:coordAdapter];
    
    mapView.continuousZoom = true;

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
    
    // Wire up the gesture recognizers
    tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
    panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
    boundLL = MaplyCoordinateMakeWithDegrees(-180.0,-90.0);
    boundUR = MaplyCoordinateMakeWithDegrees(180.0,90.0);
    pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
    pinchDelegate.minZoom = [mapView minHeightAboveSurface];
    pinchDelegate.maxZoom = [mapView maxHeightAboveSurface];
    [self setViewExtentsLL:boundLL ur:boundUR];
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
    [[NSNotificationCenter defaultCenter] removeObserver:self name:MaplyTapMsg object:nil];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

// Register for interesting tap events and others
// Note: Fill this in
- (void)registerForEvents
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapOnMap:) name:MaplyTapMsg object:nil];
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
    [panDelegate setBounds:bounds];
    [pinchDelegate setBounds:bounds];
}

// Internal animation handler
- (void)animateToPoint:(Point3f)newLoc time:(NSTimeInterval)howLong
{
    [mapView cancelAnimation];

    MaplyAnimateViewTranslation *animTrans = [[MaplyAnimateViewTranslation alloc] initWithView:mapView translate:newLoc howLong:howLong];
    curAnimation = animTrans;
    mapView.delegate = animTrans;
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong
{
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
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    // Figure out where the point lands on the map
    Eigen::Matrix4f modelTrans = [mapView calcFullMatrix];
    Point3f whereLoc;
    if ([mapView pointOnPlaneFromScreen:loc transform:&modelTrans frameSize:Point2f(sceneRenderer.framebufferWidth/glView.contentScaleFactor,sceneRenderer.framebufferHeight/glView.contentScaleFactor) hit:&whereLoc clip:true])
    {
        Point3f diffLoc(whereLoc.x()-mapView.loc.x(),whereLoc.y()-mapView.loc.y(),0.0);
        Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
        loc.x() -= diffLoc.x();
        loc.y() -= diffLoc.y();
        loc.z() = mapView.loc.z();
        [self animateToPoint:loc time:howLong];
    }
}

// This version takes a height as well
- (void)animateToPosition:(MaplyCoordinate)newPos height:(float)newHeight time:(NSTimeInterval)howLong
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

    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
    loc.z() = newHeight;
    
    [self animateToPoint:loc time:howLong];
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    Point3f loc = mapView.loc;
    [self setPosition:newPos height:loc.z()];
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
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
    
    Point3f loc = mapView.coordAdapter->localToDisplay(mapView.coordAdapter->getCoordSystem()->geographicToLocal(GeoCoord(newPos.x,newPos.y)));
    loc.z() = height;
    mapView.loc = loc;
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    Point3f loc = mapView.loc;
    GeoCoord geoCoord = mapView.coordAdapter->getCoordSystem()->localToGeographic(loc);
    pos->x = geoCoord.x();  pos->y = geoCoord.y();
    *height = loc.z();
}

/// Return the min and max heights above the globe for zooming
- (void)getZoomLimitsMin:(float *)minHeight max:(float *)maxHeight
{
    if (pinchDelegate)
    {
        *minHeight = pinchDelegate.minZoom;
        *minHeight = pinchDelegate.maxZoom;
    }
}

/// Set the min and max heights above the globe for zooming
- (void)setZoomLimitsMin:(float)minHeight max:(float)maxHeight
{
    if (pinchDelegate)
    {
        pinchDelegate.minZoom = minHeight;
        pinchDelegate.maxZoom = maxHeight;
        Point3f loc = mapView.loc;
        if (mapView.heightAboveSurface < minHeight)
            mapView.loc = Point3f(loc.x(),loc.y(),minHeight);
        if (mapView.heightAboveSurface > maxHeight)
            mapView.loc = Point3f(loc.x(),loc.y(),maxHeight);
    }
}

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    if (selectedObj && selection)
    {
        // The user selected something, so let the delegate know
        if (delegate && [delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
            [delegate maplyViewController:self didSelect:selectedObj];
    } else {
        MaplyCoordinate coord;
        coord.x = msg.whereGeo.lon();
        coord.y = msg.whereGeo.lat();
        // The user didn't select anything, let the delegate know.
        if (delegate && [delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
        {
            [delegate maplyViewController:self didTapAt:coord];
        }
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

@end
