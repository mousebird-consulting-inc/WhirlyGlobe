//
//  VectorsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 29/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "VectorsTestCase.h"
#import "GeographyClassTestCase.h"
#import "MaplyBaseViewController.h"
#include <stdlib.h>

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
				NSString *vecName = [[wgVecObj attributes] objectForKey:@"ADMIN"];
				wgVecObj.userObject = vecName;
				[self.compList addObject:wgVecObj];
				[baseVC addVectors:[NSArray arrayWithObject:wgVecObj] desc:vectorDict];
				if ([vecName isEqualToString:@"Spain"]) {
					self.selectedCountry = wgVecObj;
				}
			}
		}
		if (self.selectedCountry != nil) {
			//[self handleSelection:baseVC selected:self.selectedCountry];
		}
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
	GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
	[baseView setUpWithGlobe:globeVC];
	//Overlay Countries
	[self overlayCountries:(MaplyBaseViewController*)globeVC];
	return true;
}


- (BOOL)setUpWithMap:(MaplyViewController *)mapVC
{	
	GeographyClassTestCase *baseView = [[GeographyClassTestCase alloc]init];
	[baseView setUpWithMap:mapVC];
	[self overlayCountries:(MaplyBaseViewController*)mapVC];
	
	return true;
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
