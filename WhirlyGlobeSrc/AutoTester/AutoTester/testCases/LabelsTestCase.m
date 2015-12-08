//
//  LabelsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 2/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "LabelsTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyLabel.h"
#import "VectorsTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation LabelsTestCase


- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 3;
		self.name = @"Labels";
	}
	return self;
}

- (void) insertLabels: (NSMutableArray*) arrayComp theView: (MaplyBaseViewController*) theView {
	
	CGSize size = CGSizeMake(0, 0.05);
	for (int i=0; i < arrayComp.count; i++){
		MaplyVectorObject* object = arrayComp[i];
		MaplyLabel *label = [[MaplyLabel alloc] init];
		label.loc = object.center;
		label.size = size;
		if (object.userObject == nil) {
			label.text = @"Label";
			label.userObject = object.userObject;
		}
		else {
			label.text = object.userObject;
			label.userObject = object.userObject;
		}
		if (i % 2 == 0) {
			[theView addLabels:@[label]
						  desc:@{
							kMaplyFont: [UIFont boldSystemFontOfSize:24.0],
							kMaplyTextOutlineColor: [UIColor whiteColor],
							kMaplyTextOutlineSize: @2.0,
							kMaplyColor: [UIColor whiteColor]
						}];
		}
		else {
			[theView addLabels:@[label]
						  desc:@{
							kMaplyFont: [UIFont boldSystemFontOfSize:24.0],
							kMaplyShadowColor: [UIColor whiteColor],
							kMaplyShadowSize: @2.0,
							kMaplyColor: [UIColor whiteColor]
						}];
		}
	}
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC {
	VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
	[baseView setUpWithGlobe:globeVC];
	[self insertLabels: baseView.compList theView:(MaplyBaseViewController*)globeVC];
	[globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
	return true;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC {
	VectorsTestCase * baseView = [[VectorsTestCase alloc]init];
	[baseView setUpWithMap:mapVC];
	[self insertLabels: baseView.compList theView:(MaplyBaseViewController*)mapVC];
	[mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-3.6704803, 40.5023056) time:1.0];
	return true;
}
@end
