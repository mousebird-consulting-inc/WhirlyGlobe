//
//  ShapefileTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 6/25/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "ShapefileTestCase.h"
#import "GeographyClassTestCase.h"

@implementation ShapefileTestCase
{
    MaplyBaseViewController *baseViewC;
    MaplyTestCase *baseCase;
    MaplyComponentObject *compObj;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Shapefile";
        self.captureDelay = 5;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    
    return self;
}

- (void)overlayShapefile
{
    MaplyVectorDatabase *vecDb = [[MaplyVectorDatabase alloc] initWithShape:@"ne_10m_roads"];
    MaplyVectorObject *vecObj = [vecDb fetchAllVectors];
    
    compObj = [baseViewC addVectors:@[vecObj] desc:@{kMaplyColor: [UIColor whiteColor]}];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    baseViewC = globeVC;
    baseCase = [[GeographyClassTestCase alloc]init];
    [baseCase setUpWithGlobe:globeVC];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       [self overlayShapefile];
                   });
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseViewC = mapVC;
    baseCase = [[GeographyClassTestCase alloc]init];
    [baseCase setUpWithMap:mapVC];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       [self overlayShapefile];
                   });
}

- (void) tearDownWithMap:(MaplyViewController *)mapVC {
    [baseViewC removeObject:compObj];
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController *)globeVC{
    [baseViewC removeObject:compObj];
}
- (void) handleSelection:(MaplyBaseViewController *)viewC
                selected:(NSObject *)selectedObj
{
    // ensure it's a MaplyVectorObject. It should be one of our outlines.
    if ([selectedObj isKindOfClass:[MaplyVectorObject class]]) {
        MaplyVectorObject *theVector = (MaplyVectorObject *)selectedObj;
        MaplyCoordinate location;
        
        if ([theVector centroid:&location]) {
            MaplyAnnotation *annotate = [[MaplyAnnotation alloc]init];
            annotate.title = @"Selected";
            annotate.subTitle = (NSString *)theVector.userObject;
            [viewC addAnnotation:annotate forPoint:location offset:CGPointZero];
        }
    }
}

@end
