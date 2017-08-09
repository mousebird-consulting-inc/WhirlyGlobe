//
//  VectorsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
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
		self.captureDelay = 5;
		self.compList = [[NSMutableArray alloc] init];
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
                    wgVecObj.userObject = vecName;
                    wgVecObj.selectable = true;
                    [self.compList addObject:wgVecObj];
                    [baseVC addVectors:[NSArray arrayWithObject:wgVecObj] desc:vectorDict];
                    [baseVC addSelectionVectors:[NSArray arrayWithObject:wgVecObj]];
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

- (void) tearDownWithMap:(MaplyViewController *)mapVC {
    for (MaplyComponentObject __strong *comp in self.compList){
        [mapVC removeObject: comp];
        comp = nil;
    }
    [self.compList removeAllObjects ];
    self.baseView = nil;
    
}

- (void)tearDownWithGlobe:(WhirlyGlobeViewController *)globeVC{
    
    for (MaplyComponentObject __strong *comp in self.compList){
        [globeVC removeObject: comp];
        comp = nil;
    }
    [self.compList removeAllObjects ];
    self.baseView = nil;
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
