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
    [super clear];
    
    mapScene = NULL;
    mapView = nil;

    mapInteractLayer = nil;
    
    tapDelegate = nil;
    panDelegate = nil;
    pinchDelegate = nil;
    
    if (coordAdapter)
        delete coordAdapter;
    coordAdapter = NULL;    
}

- (void)dealloc
{
    [self clear];
}

- (WhirlyKitView *) loadSetup_view
{
    coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord(-180.0,-90.0), GeoCoord(180.0,90.0));
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
    pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
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

// Internal animation handler
- (void)animateToPoint:(GeoCoord)whereGeo time:(NSTimeInterval)howLong
{
    // note: fill this in
}

// External facing version of rotateToPoint
- (void)animateToPosition:(MaplyCoordinate)newPos time:(NSTimeInterval)howLong
{
    // Note: fill this in
}

// External facing set position
- (void)setPosition:(MaplyCoordinate)newPos
{
    // Note: fill this in
}

- (void)setPosition:(MaplyCoordinate)newPos height:(float)height
{
    // Note: fill this in
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
        // The user didn't select anything, let the delegate know.
        if (delegate && [delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
        {
            MaplyCoordinate coord;
            coord.x = msg.whereGeo.lon();
            coord.y = msg.whereGeo.lat();
            [delegate maplyViewController:self didTapAt:coord];
        }
        [self animateToPoint:msg.whereGeo time:1.0];
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
