//
//  VectorsTestCase.mm
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright 2015-2022 mousebird consulting.
//

#import "VectorsTestCase.h"
#import "SwiftBridge.h"

#include <stdlib.h>

@interface VectorsTestCase()

@property (strong, nonatomic) MaplyVectorObject * selectedCountry;
@end

@implementation VectorsTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Vectors";
		self.compObjs = [[NSMutableArray alloc] init];
        self.vecList = [[NSMutableArray alloc] init];
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
	}
	
	return self;
}


- (void) overlayCountries: (MaplyBaseViewController*) baseVC
{
    NSDictionary *vectorDict = @{
        kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
        kMaplyColor: [UIColor redColor],
        kMaplySelectable: @(true),
        kMaplyFade: @(0.2),
    };
    NSDictionary *wideDict = @{
        kMaplyWideVecImpl: kMaplyWideVecImplPerf,
        kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault),
        kMaplyVecCloseAreals: @(false),
        kMaplyColor: [UIColor whiteColor],
        kMaplySelectable: @(false),
        kMaplyFade: @(0.2),
        kMaplyVecWidth: @(3.0),
    };
    NSArray * paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"geojson" inDirectory:nil];
    for (NSString* fileName  in paths) {
        // We only want the three letter countries
        NSString *baseName = [[fileName lastPathComponent] stringByDeletingPathExtension];
        if ([baseName length] != 3)
            continue;
        NSData *jsonData = [NSData dataWithContentsOfFile:fileName];
        if (jsonData) {
            MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];
            if (wgVecObj)
            {
                NSLog(@"Loading vector %@",fileName);
                NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
                wgVecObj.attributes[@"title"] = vecName;
                wgVecObj.selectable = true;
                [self.vecList addObject:wgVecObj];
                MaplyComponentObject *compObj = [baseVC addVectors:@[wgVecObj] desc:vectorDict];
                if (compObj) {
                    [self.compObjs addObject:compObj];
                }
                compObj = [baseVC addWideVectors:@[wgVecObj] desc:wideDict];
                if (compObj) {
                    [self.compObjs addObject:compObj];
                }
//                    [baseVC addSelectionVectors:[NSArray arrayWithObject:wgVecObj]];
                if ([vecName isEqualToString:@"Spain"]) {
                    self.selectedCountry = wgVecObj;
                }
            }
        }
    }
    if (self.selectedCountry != nil) {
        //[self handleSelection:baseVC selected:self.selectedCountry];
    }
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	 self.baseCase = [[GeographyClassTestCase alloc]init];
	[self.baseCase setUpWithGlobe:globeVC];
	//Overlay Countries
	[self overlayCountries:(MaplyBaseViewController*)globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{	
	 self.baseCase = [[GeographyClassTestCase alloc]init];
	[self.baseCase setUpWithMap:mapVC];
	[self overlayCountries:(MaplyBaseViewController*)mapVC];
}

- (void)handleSelection:(NSArray<NSObject *>*)selectedObjs
                  atLoc:(MaplyCoordinate)coord
               onScreen:(CGPoint)screenPt {
    [self.baseViewController clearAnnotations];
    
    CGPoint offset = CGPointMake(0,0);
    for (__strong NSObject* obj in selectedObjs) {
        if ([obj isKindOfClass:[MaplySelectedObject class]]) {
            obj = ((MaplySelectedObject *)obj).selectedObj;
        }
        // ensure it's a MaplyVectorObject. It should be one of our outlines.
        if ([obj isKindOfClass:[MaplyVectorObject class]]) {
            MaplyVectorObject *theVector = (MaplyVectorObject *)obj;

            MaplyCoordinate location;
            if (![theVector centroid:&location]) {
                location = theVector.center;
            }

            MaplyAnnotation *annotate = [[MaplyAnnotation alloc]init];
            annotate.title = [NSString stringWithFormat:@"Selected (at x=%.0f y=%.0f)", screenPt.x, screenPt.y];;
            annotate.subTitle = (NSString *)theVector.attributes[@"title"];
            [self.baseViewController addAnnotation:annotate
                                          forPoint:location
                                            offset:offset];
            offset.x += 10;
        }
    }
}

- (void) stop
{
    [self.baseCase stop];
    if (_compObjs) {
        [self.baseViewController removeObjects:_compObjs];
        [_compObjs removeAllObjects];
    }
}

@end
