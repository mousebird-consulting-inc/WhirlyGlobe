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

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@interface MaplyViewController () <MaplyInteractionLayerDelegate>
@end

@implementation MaplyViewController
{
    // Flat view for 2D mode
    MaplyFlatView * flatView;
    // Scroll view for tethered mode
    UIScrollView * __weak scrollView;
    // Content scale for scroll view mode
    float scale;
    bool scheduledToDraw;
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
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
    
    [layerThread addThingToDelete:coordAdapter];
    
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
    coordAdapter = new SphericalMercatorDisplayAdapter(0.0, GeoCoord::CoordFromDegrees(-180.0,-90.0), GeoCoord::CoordFromDegrees(180.0,90.0));
    
    if (scrollView)
    {
        flatView = [[MaplyFlatView alloc] initWithCoordAdapter:coordAdapter];
        [self setupFlatView];
        mapView = flatView;
    } else {
        mapView = [[MaplyView alloc] initWithCoordAdapter:coordAdapter];
        mapView.continuousZoom = true;
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
    if (!_tetheredMode)
    {
        // Wire up the gesture recognizers
        tapDelegate = [MaplyTapDelegate tapDelegateForView:glView mapView:mapView];
        panDelegate = [MaplyPanDelegate panDelegateForView:glView mapView:mapView];
        pinchDelegate = [MaplyPinchDelegate pinchDelegateForView:glView mapView:mapView];
        pinchDelegate.minZoom = [mapView minHeightAboveSurface];
        pinchDelegate.maxZoom = [mapView maxHeightAboveSurface];
        rotateDelegate = [MaplyRotateDelegate rotateDelegateForView:glView mapView:mapView];
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
    
    if (flatView)
    {
        [flatView setExtents:Mbr(Point2f(ll.x,ll.y),Point2f(ur.x,ur.y))];
    }
    
    if (panDelegate)
        [panDelegate setBounds:bounds];
    if (pinchDelegate)
        [pinchDelegate setBounds:bounds];
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
- (void)animateToPosition:(MaplyCoordinate)newPos onScreen:(CGPoint)loc time:(NSTimeInterval)howLong
{
    if (_tetheredMode)
        return;
    
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
    }
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
}

- (void)getPosition:(WGCoordinate *)pos height:(float *)height
{
    Point3d loc = mapView.loc;
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

// Called back on the main thread after the interaction thread does the selection
- (void)handleSelection:(MaplyTapMessage *)msg didSelect:(NSObject *)selectedObj
{
    if (selectedObj && self.selection)
    {
        // The user selected something, so let the delegate know
        if (_delegate && [_delegate respondsToSelector:@selector(maplyViewController:didSelect:)])
            [_delegate maplyViewController:self didSelect:selectedObj];
    } else {
        MaplyCoordinate coord;
        coord.x = msg.whereGeo.lon();
        coord.y = msg.whereGeo.lat();
        // The user didn't select anything, let the delegate know.
        if (_delegate && [_delegate respondsToSelector:@selector(maplyViewController:didTapAt:)])
        {
            [_delegate maplyViewController:self didTapAt:coord];
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
