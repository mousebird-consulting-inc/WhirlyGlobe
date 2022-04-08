//
//  FindHeightTestCase.m
//  AutoTester
//
//  Created by Ranen Ghosh on 4/7/16.
//  Copyright 2016-2022 mousebird consulting.
//

#import "FindHeightTestCase.h"
#import "SwiftBridge.h"

@implementation FindHeightTestCase {
    MaplyBaseViewController *_baseVC;
    CartoDBLightTestCase *baseView;
    bool stopped;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Find Height";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        stopped = false;
    }
    return self;
}

- (void)setupWithBaseVC:(MaplyBaseViewController *)vc {
}

- (void)addBoundingBox:(MaplyBoundingBox)bbox baseVC:(MaplyBaseViewController *)baseVC
{
    // Add a visible bounding box
    MaplyCoordinate coords[5];
    coords[0] = MaplyCoordinateMake(bbox.ll.x,bbox.ll.y);
    coords[1] = MaplyCoordinateMake(bbox.ur.x,bbox.ll.y);
    coords[2] = MaplyCoordinateMake(bbox.ur.x,bbox.ur.y);
    coords[3] = MaplyCoordinateMake(bbox.ll.x,bbox.ur.y);
    coords[4] = MaplyCoordinateMake(bbox.ll.x,bbox.ll.y);
    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:coords numCoords:5 attributes:nil];
    [baseVC addVectors:@[vecObj] desc:@{kMaplyColor: [UIColor redColor]}];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    stopped = false;
    baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithGlobe:globeVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)globeVC];
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5 heading:0 time:1.0];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        if (self->stopped) {
            return;
        }
        const MaplyBoundingBox bbox = { MaplyCoordinateMakeWithDegrees(7.05090689853, 47.7675500593),
                                        MaplyCoordinateMakeWithDegrees(8.06813647023, 49.0562323851) };
        [self addBoundingBox:bbox baseVC:globeVC];

        const CGPoint margin = CGPointMake(20, 50);

        const MaplyCoordinate center = MaplyCoordinateMakeWithDegrees((7.05090689853+8.06813647023)/2, (47.7675500593+49.0562323851)/2);
        const double height = [globeVC findHeightToViewBounds:bbox
                                                          pos:center
                                                      marginX:-margin.x
                                                      marginY:-margin.y];
        if (height > 0)
        {
            [globeVC animateToPosition:center height:height heading:0 time:3.0];
        }
        NSLog(@"height = %f",height);
        
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 4.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            if (self->stopped) {
                return;
            }
            // Consider the bottom/right quarter of the screen to test offset and size
            const CGRect wholeFrame = globeVC.view.frame;
            const CGRect frame = CGRectIntersection(wholeFrame,
                                    CGRectOffset(wholeFrame, wholeFrame.size.width / 2, wholeFrame.size.height / 2));
            MaplyCoordinate newCenter = {0,0};
            const double height = [globeVC findHeightToViewBounds:bbox
                                                              pos:center
                                                            frame:frame
                                                           newPos:&newCenter
                                                          marginX:-margin.x
                                                          marginY:-margin.y];
            if (height > 0)
            {
                [globeVC animateToPosition:newCenter height:height heading:0 time:1.0];
            }
            NSLog(@"center = %f/%f height = %f", 180*newCenter.y/M_PI, 180*newCenter.x/M_PI, height);
        });
    });
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    stopped = false;
    baseView = [[CartoDBLightTestCase alloc] init];
    [baseView setUpWithMap:mapVC];
    [self setupWithBaseVC:(MaplyBaseViewController *)mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-98.58, 39.83) height:1.5 time:1.0];

    dispatch_after( dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        if (self->stopped) {
            return;
        }
        MaplyBoundingBox bbox = { MaplyCoordinateMakeWithDegrees(7.05090689853, 47.7675500593),
                                  MaplyCoordinateMakeWithDegrees(8.06813647023, 49.0562323851) };
        [self addBoundingBox:bbox baseVC:mapVC];

        const CGPoint margin = CGPointMake(20, 50);

        const MaplyCoordinate center = MaplyCoordinateMakeWithDegrees((7.05090689853+8.06813647023)/2, (47.7675500593+49.0562323851)/2);
        const double height = [mapVC findHeightToViewBounds:bbox
                                                        pos:center
                                                    marginX:-margin.x
                                                    marginY:-margin.y];
        if (height > 0)
        {
            [mapVC animateToPosition:center height:height heading:mapVC.heading time:3.0];
        }
        NSLog(@"height = %f",height);
        
        // Then, repeat but fit it into just part of the view
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 4.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            if (self->stopped) {
                return;
            }
            // Consider the bottom/right quarter of the screen to test offset and size
            const CGRect wholeFrame = mapVC.view.frame;
            const CGRect frame = CGRectIntersection(wholeFrame,
                                    CGRectOffset(wholeFrame, wholeFrame.size.width / 2,
                                                             wholeFrame.size.height / 2));
            MaplyCoordinate newCenter = {0,0};
            const double height = [mapVC findHeightToViewBounds:bbox
                                                            pos:center
                                                          frame:frame
                                                         newPos:&newCenter
                                                        marginX:-margin.x
                                                        marginY:-margin.y];
            if (height > 0)
            {
                [mapVC animateToPosition:newCenter height:height heading:mapVC.heading time:1.0];
            }
            NSLog(@"center = %f/%f height = %f", 180*newCenter.y/M_PI, 180*newCenter.x/M_PI, height);
        });
    });
}

- (void)stop {
    stopped = true;
    [super stop];
}

@end
