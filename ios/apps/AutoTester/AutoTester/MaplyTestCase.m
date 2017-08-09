//
//  MaplyTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import "MaplyTestCase.h"
#import "WhirlyGlobeComponent.h"
#import "MaplyComponent.h"


@implementation MaplyTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.state = MaplyTestCaseStateReady;
		self.interactive = false;
		self.pendingDownload = 0;
	}

	return self;
}

- (NSArray * _Nullable)remoteResources {
	return nil;
}


- (void)start
{
	if (self.state != MaplyTestCaseStateDownloading && self.state != MaplyTestCaseStateError) {
		self.state = MaplyTestCaseStateRunning;

		dispatch_group_t lock = dispatch_group_create();

		if (self.options & MaplyTestCaseOptionGlobe) {
			if (!self.interactive)
				dispatch_group_enter(lock);

			[self runGlobeTestWithLock:lock];
		}

		if (self.options & MaplyTestCaseOptionMap) {
			if (!self.interactive)
				dispatch_group_enter(lock);

			[self runMapTestWithLock:lock];
		}

		if (!self.interactive)
			dispatch_group_notify(lock,dispatch_get_main_queue(),^{
				self.state = MaplyTestCaseStateReady;

				if (self.resultBlock) {
					self.resultBlock(self);
				}
			});
	}
}

- (void)runGlobeTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.globeViewController = [[WhirlyGlobeViewController alloc] init];
    self.baseViewController = self.globeViewController;
	[self.testView addSubview:self.globeViewController.view];
	self.globeViewController.view.backgroundColor = [UIColor blueColor];
	self.globeViewController.view.frame = self.testView.bounds;
	self.globeViewController.clearColor = [UIColor blackColor];
	self.globeViewController.frameInterval = 2;
    self.globeViewController.delegate = self;

	// setup test case specifics
	[self setUpWithGlobe:self.globeViewController];

	if (!self.interactive)
	{
		[self runTestWithGlobe:self.globeViewController result: ^(BOOL passed) {
			if (!passed || self.captureDelay == -1) {
				[self finishGlobeTestWithActualImage:nil passed:passed];
				dispatch_group_leave(lock);
			}
			else {
				__weak MaplyTestCase *weak_self = self;

				// TODO don't use main queue. The capture is blocking!
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, self.captureDelay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{

					NSString *snapshot = [weak_self captureScreenFromVC:self.globeViewController];

					[weak_self finishGlobeTestWithActualImage:snapshot
													   passed:passed && (_globeResult.actualImageFile != nil)];
					dispatch_group_leave(lock);
				});
			}
		}];
	}
}

- (MaplyCoordinateSystem * _Nullable)customCoordSystem
{
	return nil;
}

- (void)runMapTestWithLock:(dispatch_group_t)lock
{
	// create and prepare the controller
	self.mapViewController = [[MaplyViewController alloc] initWithMapType:MaplyMapTypeFlat];
    self.mapViewController.viewWrap = true;
    self.baseViewController = self.mapViewController;
    self.mapViewController.delegate = self;
	
	MaplyCoordinateSystem *coordSys = [self customCoordSystem];
	if (coordSys)
		self.mapViewController.coordSys = coordSys;

	[self.testView addSubview:self.mapViewController.view];
	self.mapViewController.view.frame = self.testView.bounds;
	self.mapViewController.clearColor = [UIColor blackColor];
	self.mapViewController.frameInterval = 2;

	// setup test case specifics
	[self setUpWithMap:self.mapViewController];

	if (!self.interactive)
	{
		[self runTestWithMap:self.mapViewController result: ^(BOOL passed) {
			if (!passed || self.captureDelay == -1) {
				[self finishMapTestWithActualImage:nil passed:passed];
				dispatch_group_leave(lock);
			}
			else {
				__weak MaplyTestCase *weak_self = self;

				// TODO don't use main queue. The capture is blocking!
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, self.captureDelay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{

					NSString *snapshot = [weak_self captureScreenFromVC:self.mapViewController];

					[weak_self finishMapTestWithActualImage:snapshot
													 passed:passed && (_mapResult.actualImageFile != nil)];
					dispatch_group_leave(lock);
				});
			}
		}];
	}
}


- (void)finishGlobeTestWithActualImage:(NSString *)actualImage passed:(BOOL)passed
{
	_globeResult = [[MaplyTestResult alloc] initWithTestName:self.name
													baseline:[self baselineGlobeImageFile]
													  actual:actualImage
													  passed:passed];

	[self tearDownWithGlobe:self.globeViewController];
	[self removeGlobeController];
}

-(void) removeGlobeController {
    [self.globeViewController teardown];
    
	[self.globeViewController.view removeFromSuperview];
	self.globeViewController = nil;

}
- (void)finishMapTestWithActualImage:(NSString *)actualImage passed:(BOOL)passed
{
	_mapResult = [[MaplyTestResult alloc] initWithTestName:self.name
												  baseline:[self baselineMapImageFile]
													actual:actualImage
													passed:passed];

	[self tearDownWithMap:self.mapViewController];
	[self removeMapController];
}
- (void) removeMapController {
    [self.mapViewController teardown];
    
	[self.mapViewController.view removeFromSuperview];
	self.mapViewController = nil;
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController *)globeVC
{
}

- (void)tearDownWithMap:(MaplyViewController *)mapVC
{
}

- (NSString *)baselineGlobeImageFile
{
	NSString *name = [NSString stringWithFormat:@"%@-globe", [self testClassName]];
	return [[NSBundle mainBundle] pathForResource:name ofType:@"png"];
}

- (NSString *)baselineMapImageFile
{
	NSString *name = [NSString stringWithFormat:@"%@-map", [self testClassName]];
	return [[NSBundle mainBundle] pathForResource:name ofType:@"png"];
}

- (NSString *)testClassName
{
	NSString *className = NSStringFromClass(self.class);
	if ([className hasPrefix:@"AutoTester."]) {
		className = [className componentsSeparatedByString:@"."][1];
	}

	return className;
}

- (void)runTestWithGlobe:(WhirlyGlobeViewController *)globeVC result:(void (^)(BOOL passed))block
{
	block(YES);
}

- (void)runTestWithMap:(MaplyViewController *)globeVC result:(void (^)(BOOL passed))block
{
	block(YES);
}

- (NSString *)captureScreenFromVC:(MaplyBaseViewController *)vc
{
	UIImage *screenshot = vc.snapshot;

	if (screenshot) {
		NSString *dir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];
		//NSLog(@"------>%@", dir);
		NSData *imgData = UIImagePNGRepresentation(screenshot);

		NSString *guid = [[NSProcessInfo processInfo] globallyUniqueString] ;
		NSString *fileName = [NSString stringWithFormat:@"%@-%@", [self testClassName], guid];
		dir = [dir stringByAppendingPathComponent:@"results"];
		fileName = [dir stringByAppendingPathComponent:fileName];

		[imgData writeToFile:fileName atomically:YES];

		return fileName;
	}

	NSLog(@"An error happened capturing the screen");
	return nil;
}

- (void)setSelected:(BOOL)selected
{
	if (self.state == MaplyTestCaseStateReady || self.state == MaplyTestCaseStateSelected){
		self.state = MaplyTestCaseStateSelected;
		[[NSUserDefaults standardUserDefaults] setBool:selected forKey:_name];
	}
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

- (void)maplyViewController:(MaplyViewController *__nonnull)viewC didSelect:(NSObject *__nonnull)selectedObj atLoc:(WGCoordinate)coord onScreen:(CGPoint)screenPt
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
            title = (NSString *)vecObj.userObject;
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
