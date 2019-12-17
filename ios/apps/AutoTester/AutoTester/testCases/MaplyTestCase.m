//
//  MaplyTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "MaplyTestCase.h"
#import "WhirlyGlobeComponent.h"
#import "MaplyComponent.h"


@implementation MaplyTestCase

- (void)startGlobe:(UINavigationController *)nav
{
    // create and prepare the controller
    self.globeViewController = [[WhirlyGlobeViewController alloc] init];
    // Note: Debugging
//    self.globeViewController.useOpenGLES = true;
    self.baseViewController = self.globeViewController;
    [nav pushViewController:self.baseViewController animated:YES];
    self.globeViewController.view.backgroundColor = [UIColor blackColor];
    self.globeViewController.clearColor = [UIColor blackColor];
    self.globeViewController.frameInterval = 2;
    self.globeViewController.delegate = self;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        // setup test case specifics
        [self setUpWithGlobe:self.globeViewController];
    });
}

- (void)startMap:(UINavigationController *)nav
{
    // create and prepare the controller
    self.mapViewController = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
    self.mapViewController.viewWrap = true;
    self.baseViewController = self.mapViewController;
    self.mapViewController.delegate = self;
    
    MaplyCoordinateSystem *coordSys = [self customCoordSystem];
    if (coordSys)
        self.mapViewController.coordSys = coordSys;

    [nav pushViewController:self.baseViewController animated:YES];
    self.mapViewController.view.backgroundColor = [UIColor blackColor];
    self.mapViewController.clearColor = [UIColor blackColor];
    self.mapViewController.frameInterval = 2;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        // setup test case specifics
        [self setUpWithMap:self.mapViewController];
    });
}

- (MaplyCoordinateSystem * _Nullable)customCoordSystem
{
	return nil;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
}

- (void)stop
{
    if (self.globeViewController) {
        [self.globeViewController teardown];
        
        [self.globeViewController.view removeFromSuperview];
        self.globeViewController = nil;
    }
    if (self.mapViewController) {
        [self.mapViewController teardown];
        
        [self.mapViewController.view removeFromSuperview];
        self.mapViewController = nil;
    }
}

- (NSString *)testClassName
{
	NSString *className = NSStringFromClass(self.class);
	if ([className hasPrefix:@"AutoTester."]) {
		className = [className componentsSeparatedByString:@"."][1];
	}

	return className;
}

- (void)setName:(NSString *)name
{
	_name = name;
}

- (void)globeViewController:(WhirlyGlobeViewController *__nonnull)viewC allSelect:(NSArray *__nonnull)selectedObjs atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt
{
    [self handleSelection:selectedObjs];
}

- (void)globeViewController:(WhirlyGlobeViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    [self.baseViewController clearAnnotations];
}

- (void)globeViewControllerDidTapOutside:(WhirlyGlobeViewController *)viewC
{
    [self.baseViewController clearAnnotations];
}

-(void)maplyViewController:(MaplyViewController *)viewC didSelect:(NSObject *)selectedObj
{
    [self handleSelection:selectedObj];
}

- (void)maplyViewController:(MaplyViewController *__nonnull)viewC didSelect:(NSObject *__nonnull)selectedObj atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt
{
    [self handleSelection:selectedObj];
}

- (void)maplyViewController:(MaplyViewController *)viewC allSelect:(NSArray *)selectedObjs atLoc:(MaplyCoordinate)coord onScreen:(CGPoint)screenPt
{
    [self handleSelection:selectedObjs];
}

- (void)maplyViewController:(MaplyViewController *)viewC didTapAt:(MaplyCoordinate)coord
{
    [self.baseViewController clearAnnotations];
}

- (void)handleSelection:(id)selectedObjs
{
    // If we've currently got a selected view, get rid of it
    //    if (selectedViewTrack)
    //    {
    //        [baseViewC removeViewTrackForView:selectedViewTrack.view];
    //        selectedViewTrack = nil;
    //    }
    [self.baseViewController clearAnnotations];
    
    bool isMultiple = [selectedObjs isKindOfClass:[NSArray class]] && [(NSArray *)selectedObjs count] > 1;
    
    NSString *title = nil,*subTitle = nil;
    CGPoint offset = CGPointZero;
    MaplyCoordinate loc = MaplyCoordinateMakeWithDegrees(0,0);
    if (isMultiple)
    {
        NSArray *selArr = selectedObjs;
        if ([selArr count] == 0)
            return;
        
        MaplySelectedObject *firstObj = [selArr objectAtIndex:0];
        // Only screen objects will be clustered
        if ([firstObj.selectedObj isKindOfClass:[MaplyScreenMarker class]])
        {
            MaplyScreenMarker *marker = firstObj.selectedObj;
            loc = marker.loc;
        } else if ([firstObj.selectedObj isKindOfClass:[MaplyScreenLabel class]])
        {
            MaplyScreenLabel *label = firstObj.selectedObj;
            loc = label.loc;
        } else
            return;
        
        title = @"Cluster";
        subTitle = [NSString stringWithFormat:@"%tu objects",[selArr count]];
    } else {
        id selectedObj = nil;
        if ([selectedObjs isKindOfClass:[NSArray class]])
        {
            NSArray *selArr = selectedObjs;
            if ([selArr count] == 0)
                return;
            selectedObj = [(MaplySelectedObject *)[selArr objectAtIndex:0] selectedObj];
        } else
            selectedObj = selectedObjs;
        
        if ([selectedObj isKindOfClass:[MaplyMarker class]])
        {
            MaplyMarker *marker = (MaplyMarker *)selectedObj;
            loc = marker.loc;
            title = (NSString *)marker.userObject;
            subTitle = @"Marker";
        } else if ([selectedObj isKindOfClass:[MaplyScreenMarker class]])
        {
            MaplyScreenMarker *screenMarker = (MaplyScreenMarker *)selectedObj;
            loc = screenMarker.loc;
            title = (NSString *)screenMarker.userObject;
            subTitle = @"Screen Marker";
            offset = CGPointMake(0.0, -8.0);
        } else if ([selectedObj isKindOfClass:[MaplyLabel class]])
        {
            MaplyLabel *label = (MaplyLabel *)selectedObj;
            loc = label.loc;
            title = (NSString *)label.userObject;
            subTitle = @"Label";
        } else if ([selectedObj isKindOfClass:[MaplyScreenLabel class]])
        {
            MaplyScreenLabel *screenLabel = (MaplyScreenLabel *)selectedObj;
            loc = screenLabel.loc;
            title = (NSString *)screenLabel.userObject;
            subTitle = @"Screen Label";
            offset = CGPointMake(0.0, -6.0);
        } else if ([selectedObj isKindOfClass:[MaplyVectorObject class]])
        {
            MaplyVectorObject *vecObj = (MaplyVectorObject *)selectedObj;
            loc = [vecObj centroid];
            title = (NSString *)vecObj.attributes[@"title"];
            subTitle = @"Vector";
        } else if ([selectedObj isKindOfClass:[MaplyShapeSphere class]])
        {
            MaplyShapeSphere *sphere = (MaplyShapeSphere *)selectedObj;
            loc = sphere.center;
            title = @"Shape";
            subTitle = @"Sphere";
        } else if ([selectedObj isKindOfClass:[MaplyShapeCylinder class]])
        {
            MaplyShapeCylinder *cyl = (MaplyShapeCylinder *)selectedObj;
            loc = cyl.baseCenter;
            title = @"Shape";
            subTitle = @"Cylinder";
        } else if ([selectedObj isKindOfClass:[MaplyShapeGreatCircle class]])
        {
            MaplyShapeGreatCircle *gc = (MaplyShapeGreatCircle *)selectedObj;
            loc = gc.startPt;
            title = @"Shape";
            subTitle = @"Great Circle";
        } else if ([selectedObj isKindOfClass:[MaplyShapeExtruded class]])
        {
            MaplyShapeExtruded *ex = (MaplyShapeExtruded *)selectedObj;
            loc = ex.center;
            title = @"Shape";
            subTitle = @"Extruded";
        } else if ([selectedObj isKindOfClass:[MaplyGeomModelInstance class]]) {
            MaplyGeomModelInstance *modelInst = (MaplyGeomModelInstance *)selectedObj;
            loc = MaplyCoordinateMake(modelInst.center.x,modelInst.center.y);
            title = @"Model";
            subTitle = @"Instance";
        } else
        {
            // Don't know what it is
            return;
        }
    }
    
    // Build the selection view and hand it over to the globe to track
    //    selectedViewTrack = [[MaplyViewTracker alloc] init];
    //    selectedViewTrack.loc = loc;
    //    selectedViewTrack.view = [self makeSelectionView:[NSString stringWithFormat:@"%@: %@",title,subTitle]];
    //    [baseViewC addViewTracker:selectedViewTrack];
    //
    //    [self performSelector:@selector(moveViewTracker:) withObject:selectedViewTrack afterDelay:1.0];
    if (title)
    {
        MaplyAnnotation *annotate = [[MaplyAnnotation alloc] init];
        annotate.title = title;
        annotate.subTitle = subTitle;
        [self.baseViewController clearAnnotations];
        [self.baseViewController addAnnotation:annotate forPoint:loc offset:offset];
    }
}


@end
