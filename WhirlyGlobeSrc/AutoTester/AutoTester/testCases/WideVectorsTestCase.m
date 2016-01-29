//
//  WideVectorsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import "WideVectorsTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyTextureBuilder.h"
#import "MaplyScreenLabel.h"
#import "GeographyClassTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation WideVectorsTestCase

- (instancetype)init
{
	if (self = [super init]) {
		self.captureDelay = 20;
		self.name = @"Wide Vectors";
	}
	return self;
}


- (void) loadShapeFile: (MaplyBaseViewController*) baseViewC
{
	MaplyTexture *dashedLineTex,*filledLineTex;

	MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
	[lineTexBuilder setPattern:@[@(4),@(4)]];
	UIImage *dashedLineImage = [lineTexBuilder makeImage];
	dashedLineTex = [baseViewC addTexture:dashedLineImage
									 desc:@{kMaplyTexMinFilter: kMaplyMinFilterLinear,
											kMaplyTexMagFilter: kMaplyMinFilterLinear,
											kMaplyTexWrapX: @true,
											kMaplyTexWrapY: @true,
											kMaplyTexFormat: @(MaplyImageIntRGBA)}
									 mode:MaplyThreadCurrent];
	
	lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
	[lineTexBuilder setPattern:@[@(32)]];
	UIImage *lineImage = [lineTexBuilder makeImage];
	filledLineTex = [baseViewC addTexture:lineImage
									 desc:@{kMaplyTexMinFilter: kMaplyMinFilterLinear,
											kMaplyTexMagFilter: kMaplyMinFilterLinear,
											kMaplyTexWrapX: @true,
											kMaplyTexWrapY: @true,
											kMaplyTexFormat: @(MaplyImageIntRGBA)}
									 mode:MaplyThreadCurrent];
	
	
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
		^{
			// Add the vectors at three different levels

			MaplyVectorDatabase *vecDb = [[MaplyVectorDatabase alloc] initWithShape:@"tl_2013_06075_roads"];
			if (vecDb) {
				MaplyVectorObject *vecObj = [vecDb fetchAllVectors];
				if (vecObj) {
					[self addWideVectors:vecObj baseViewC:baseViewC
						   dashedLineTex:dashedLineTex
						   filledLineTex:filledLineTex];
				}
			}
		});
}


- (NSArray *)addWideVectors:(MaplyVectorObject *)vecObj baseViewC: (MaplyBaseViewController*) baseViewC dashedLineTex: (MaplyTexture*) dashedLineTex filledLineTex: (MaplyTexture*) filledLineTex
{
	UIColor *color = [UIColor blueColor];
	float fade = 0.25;
	MaplyComponentObject *lines = [baseViewC addVectors:@[vecObj] desc:@{kMaplyColor: color,
																		 kMaplyVecWidth: @(4.0),
																		 kMaplyFade: @(fade),
																		 kMaplyVecCentered: @(true),
																		 kMaplyMaxVis: @(10.0),
																		 kMaplyMinVis: @(0.00032424763776361942)
																		 }];
	
	MaplyComponentObject *screenLines = [baseViewC addWideVectors:@[vecObj] desc:@{kMaplyColor: [UIColor colorWithRed:0.5 green:0.0 blue:0.0 alpha:0.5],
																				   kMaplyFade: @(fade),
																				   kMaplyVecWidth: @(3.0),
																				   kMaplyVecTexture: filledLineTex,
																				   kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
																				   kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
																				   kMaplyWideVecMiterLimit: @(1.01),
																				   kMaplyWideVecTexRepeatLen: @(8),
																				   kMaplyMaxVis: @(0.00032424763776361942),
																				   kMaplyMinVis: @(0.00011049506429117173)
																				   }];
	
	MaplyComponentObject *realLines = [baseViewC addWideVectors:@[vecObj] desc:@{kMaplyColor: color,
																				 kMaplyFade: @(fade),
																				 kMaplyVecTexture: dashedLineTex,
																				 // 8m in display coordinates
																				 kMaplyVecWidth: @(10.0/6371000),
																				 kMaplyWideVecCoordType: kMaplyWideVecCoordTypeReal,
																				 kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
																				 kMaplyWideVecMiterLimit: @(1.01),
																				 // Repeat every 10m
																				 kMaplyWideVecTexRepeatLen: @(10/6371000.f),
																				 kMaplyMaxVis: @(0.00011049506429117173),
																				 kMaplyMinVis: @(0.0)
																				 }];
	
	// Look for some labels
	MaplyComponentObject *labelObj = nil;
	NSMutableArray *labels = [NSMutableArray array];
	for (MaplyVectorObject *road in [vecObj splitVectors])
	{
		MaplyCoordinate middle;
		double rot;
		// Note: We should get this from the view controller
		MaplyCoordinateSystem *coordSys = [[MaplySphericalMercator alloc] initWebStandard];
		[road linearMiddle:&middle rot:&rot displayCoordSys:coordSys];
		NSDictionary *attrs = road.attributes;
		
		NSString *name = attrs[@"FULLNAME"];
		
		if (name)
		{
			MaplyScreenLabel *label = [[MaplyScreenLabel alloc] init];
			label.loc = middle;
			label.text = name;
			label.layoutImportance = 1.0;
			label.rotation = rot + M_PI/2.0;
			label.keepUpright = true;
			label.layoutImportance = kMaplyLayoutBelow;
			[labels addObject:label];
		}
	}
	labelObj = [baseViewC addScreenLabels:labels desc:
				@{kMaplyTextOutlineSize: @(1.0),
				  kMaplyTextOutlineColor: [UIColor blackColor],
				  kMaplyFont: [UIFont systemFontOfSize:18.0],
				  kMaplyDrawPriority: @(200)
				  }];
	
	if ([baseViewC isKindOfClass:[WhirlyGlobeViewController class]]) {
		[(WhirlyGlobeViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.3 heading:0.8 time:0.1];
		[(WhirlyGlobeViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.1 heading:0.8 time:0.1];
		[(WhirlyGlobeViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.005 heading:0.8 time:0.1];
	}
	else {
		if ([baseViewC isKindOfClass:[MaplyViewController class]]) {
			[(MaplyViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.3 time:0.1];
			[(MaplyViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.1 time:0.1];
			[(MaplyViewController*)baseViewC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) height:0.005 time:0.1];
			
		}
	}
	
	return @[lines,screenLines,realLines,labelObj];
}

- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC{
	
	GeographyClassTestCase * baseLayer = [[GeographyClassTestCase alloc]init];
	[baseLayer setUpWithGlobe:globeVC];
	[self loadShapeFile:(MaplyBaseViewController*)globeVC];
	
	return true;
}

- (BOOL)setUpWithMap:(MaplyViewController *)mapVC{
	GeographyClassTestCase * baseLayer = [[GeographyClassTestCase alloc]init];
	[baseLayer setUpWithMap:mapVC];
	[self loadShapeFile:(MaplyBaseViewController*)mapVC];

	return true;
}

@end
