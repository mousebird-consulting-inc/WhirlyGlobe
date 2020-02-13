//
//  WideVectorsTestCase.m
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import "WideVectorsTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyTextureBuilder.h"
#import "MaplyScreenLabel.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"
#import "AutoTester-Swift.h"

@implementation WideVectorsTestCase
{
    GeographyClassTestCase * baseCase;
}

- (instancetype)init
{
	if (self = [super init]) {
		self.name = @"Wide Vectors";
		self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
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
            MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithShapeFile:@"tl_2013_06075_roads"];
            if (vecObj) {
                [self addWideVectors:vecObj baseViewC:baseViewC
                       dashedLineTex:dashedLineTex
                       filledLineTex:filledLineTex];
            }
		});
}

- (NSArray *)addGeoJson:(NSString*)name dashPattern:(NSArray*)dashPattern width:(CGFloat)width viewC:(MaplyBaseViewController *)baseViewC
{
    MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
    [lineTexBuilder setPattern:dashPattern];
    UIImage *lineImage = [lineTexBuilder makeImage];
    MaplyTexture *lineTexture = [baseViewC addTexture:lineImage
                                          imageFormat:MaplyImageIntRGBA
                                            wrapFlags:MaplyImageWrapY
                                                 mode:MaplyThreadCurrent];
    
    NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:nil];
    if(path) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSON:data];
        if(vecObj) {
            [vecObj subdivideToGlobe:0.0001];
            MaplyComponentObject *obj1 = [baseViewC addWideVectors:@[vecObj]
                                 desc: @{kMaplyColor: [UIColor colorWithRed:1 green:0 blue:0 alpha:1.0],
                                         kMaplyFilled: @NO,
                                         kMaplyEnable: @YES,
                                         kMaplyFade: @0,
                                         kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
                                         kMaplyVecCentered: @YES,
                                         kMaplyVecTexture: lineTexture,
                                         kMaplyWideVecEdgeFalloff: @(1.0),
                                         kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                                         kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                                         // More than 10 degrees need a bevel join
                                         kMaplyWideVecMiterLimit: @(10),
                                         kMaplyVecWidth: @(width)}
                                 mode:MaplyThreadCurrent];
            MaplyComponentObject *obj2 = [baseViewC addVectors:@[vecObj]
                             desc: @{kMaplyColor: [UIColor blackColor],
                                     kMaplyFilled: @NO,
                                     kMaplyEnable: @YES,
                                     kMaplyFade: @0,
                                     kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault),
                                     kMaplyVecCentered: @YES,
                                     kMaplyVecWidth: @(1)}
                             mode:MaplyThreadCurrent];

            return @[obj1,obj2];
        }
    }
    
    return nil;
}

- (NSArray *)addGeoJson:(NSString*)name viewC:(MaplyBaseViewController *)viewC
{
//    return [self addGeoJson:name dashPattern:@[@8, @8] width:4 viewC:viewC];
    return [self addGeoJson:name dashPattern:@[@8, @8] width:20 viewC:viewC];
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

- (void)wideLineTest:(MaplyBaseViewController *)viewC
{
    [self addGeoJson:@"sawtooth.geojson" viewC:viewC];
    [self addGeoJson:@"moving-lawn.geojson" viewC:viewC];
    [self addGeoJson:@"spiral.geojson" viewC:viewC];
    [self addGeoJson:@"square.geojson" viewC:viewC];
    [self addGeoJson:@"track.geojson" viewC:viewC];
    [self addGeoJson:@"uturn2.geojson" dashPattern:@[@16, @16] width:40 viewC:viewC];
    
    [self addGeoJson:@"USA.geojson" viewC:viewC];

//    [self addGeoJson:@"testJson.json" viewC:viewC];
    
    //    [self addGeoJson:@"straight.geojson"];
    //    [self addGeoJson:@"uturn.geojson"];
}


- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC{
	
	baseCase = [[GeographyClassTestCase alloc]init];
	[baseCase setUpWithGlobe:globeVC];
	[self wideLineTest:globeVC];
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:0.1];

}

- (void)setUpWithMap:(MaplyViewController *)mapVC{
	baseCase = [[GeographyClassTestCase alloc]init];
	[baseCase setUpWithMap:mapVC];
	[self wideLineTest:mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:0.1];
    [self loadShapeFile:mapVC];
}

@end
