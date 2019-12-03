//
//  VectorsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "VectorsTestCase.h"
#import "GeographyClassTestCase.h"
#import "MaplyBaseViewController.h"
#include <stdlib.h>
#import "MaplyViewController.h"
#import "WhirlyGlobeViewController.h"

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
			kMaplyColor: [UIColor whiteColor],
			kMaplySelectable: @(true),
			kMaplyVecWidth: @(4.0)};
		NSArray * paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"geojson" inDirectory:nil];
		for (NSString* fileName  in paths) {
			NSData *jsonData = [NSData dataWithContentsOfFile:fileName];
			if (jsonData) {
				MaplyVectorObject *wgVecObj = [MaplyVectorObject VectorObjectFromGeoJSON:jsonData];
                if (wgVecObj)
                {
                    NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
                    wgVecObj.attributes[@"title"] = vecName;
                    wgVecObj.selectable = true;
                    [self.vecList addObject:wgVecObj];
                    MaplyComponentObject *compObj = [baseVC addVectors:@[wgVecObj] desc:vectorDict];
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
	 self.baseView = [[GeographyClassTestCase alloc]init];
	[self.baseView setUpWithGlobe:globeVC];
	//Overlay Countries
	[self overlayCountries:(MaplyBaseViewController*)globeVC];
}


- (void)setUpWithMap:(MaplyViewController *)mapVC
{	
	 self.baseView = [[GeographyClassTestCase alloc]init];
	[self.baseView setUpWithMap:mapVC];
	[self overlayCountries:(MaplyBaseViewController*)mapVC];
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
			annotate.subTitle = (NSString *)theVector.attributes[@"title"];
			[viewC addAnnotation:annotate forPoint:location offset:CGPointZero];
		}
	}
}

- (void) stop
{
    [self.baseView stop];
    if (_compObjs) {
        [self.baseViewController removeObjects:_compObjs];
        [_compObjs removeAllObjects];
    }
}

@end
