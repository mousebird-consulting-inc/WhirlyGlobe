//
//  WideVectorsTestCase.mm
//  AutoTester
//
//  Created by jmnavarro on 3/11/15.
//  Copyright 2015-2022 mousebird consulting.
//

#import "WideVectorsTestCase.h"
#import "SwiftBridge.h"

@interface NSDictionary(Stuff)
- (NSDictionary *_Nonnull) dictionaryByMergingWith:(NSDictionary *_Nullable)dict;
@end

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

- (NSArray *)addGeoJson:(NSString*)name
            dashPattern:(NSArray*)dashPattern
                  width:(CGFloat)width
                   edge:(double)edge
                 simple:(bool)simple
                  viewC:(MaplyBaseViewController *)baseViewC
{
    MaplyTexture *lineTexture = nil;
    if (dashPattern) {
        MaplyLinearTextureBuilder *lineTexBuilder = [[MaplyLinearTextureBuilder alloc] init];
        [lineTexBuilder setPattern:dashPattern];
        UIImage *lineImage = [lineTexBuilder makeImage];
        lineTexture = [baseViewC addTexture:lineImage
                                imageFormat:MaplyImageIntRGBA
                                  wrapFlags:MaplyImageWrapY
                                       mode:MaplyThreadCurrent];
    }

    NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:nil];
    if(path) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSON:data];
        if(vecObj) {
            [vecObj subdivideToGlobe:0.0001];

            //NSMutableDictionary *wideDesc = [NSMutableDictionary dictionaryWithDictionary:@{
            NSDictionary *wideDesc = @{
                kMaplyColor: [UIColor colorWithRed:1 green:0 blue:0 alpha:1.0],
                kMaplyFilled: @NO,
                kMaplyEnable: @YES,
                kMaplyFade: @0,
                kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
                kMaplyVecCentered: @YES,
                kMaplyVecTexture: lineTexture ? lineTexture : [NSNull null],
                kMaplyWideVecEdgeFalloff:@(edge),
                kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                kMaplyWideVecOffset: @(10.0),
                kMaplyWideVecMiterLimit: @(10.0),  // More than 10 degrees need a bevel join
                kMaplyVecWidth: @(width),
                kMaplyWideVecImpl: simple ? kMaplyWideVecImplPerf : kMaplyWideVecImpl,
            };

            MaplyComponentObject *obj1 = [baseViewC addWideVectors:@[vecObj] desc: wideDesc mode:MaplyThreadCurrent];
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

- (NSArray *)addGeoJson:(NSString*)name
            dashPattern:(NSArray*)dashPattern
                  width:(CGFloat)width
                  viewC:(MaplyBaseViewController *)baseViewC
{
    return [self addGeoJson:name dashPattern:dashPattern width:width edge:1.0 simple:false viewC:baseViewC];
}

- (NSArray *)addGeoJson:(NSString*)name viewC:(MaplyBaseViewController *)viewC
{
    return [self addGeoJson:name dashPattern:@[@8, @8] width:4 viewC:viewC];
//    return [self addGeoJson:name dashPattern:@[@8, @8] width:100 viewC:viewC];
}

- (NSArray *)addWideVectors:(MaplyVectorObject *)vecObj
                  baseViewC:(MaplyBaseViewController*)baseViewC
              dashedLineTex:(MaplyTexture*)dashedLineTex
              filledLineTex:(MaplyTexture*)filledLineTex
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
//																				   kMaplyWideVecMiterLimit: @(1.01),
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
//																				 kMaplyWideVecMiterLimit: @(1.01),
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

// Like `overlap:` but confirms that splitting vector colors within a single CO/info context
- (void)vecColors:(MaplyBaseViewController *)viewC {
    const auto cx = -60.0;
    const auto cy = 40.0;
    const auto cs = 0.1;
    NSMutableArray<MaplyVectorObject *> *objs = [NSMutableArray arrayWithCapacity:5];
    for (int i = 0; i < 5; ++i) {   // lines
        const MaplyCoordinate coords[] = {
            MaplyCoordinateMakeWithDegrees(cx - 4*cs + i*1*cs, cy + 3*cs),
            MaplyCoordinateMakeWithDegrees(cx + 2*cs + i*3*cs, cy - 2*cs),
            MaplyCoordinateMakeWithDegrees(cx + 8*cs - i*2*cs, cy - 6*cs),
        };

        const auto cc = 0.2f * i;
        UIColor *vecColor = [UIColor colorWithRed:0 green:cc blue:1.0f-cc alpha:0.5];
        NSDictionary *vecDesc = @{ kMaplyColor: vecColor };

        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:&coords[0]
                                                                        numCoords:sizeof(coords)/sizeof(coords[0])
                                                                       attributes:vecDesc];
        [vecObj subdivideToGlobe:0.0001];
        [objs addObject:vecObj];
    }

    NSDictionary *wideDesc = @{
        kMaplyColor: [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.2],
        kMaplyEnable: @YES,
        kMaplyFade: @0,
        kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
        kMaplyWideVecEdgeFalloff:@(5),
        kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
        kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
        kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
        kMaplyWideVecOffset: @(5),
        kMaplyWideVecMiterLimit: @(10.0),
        kMaplyVecWidth: @(10.0),
        //kMaplyWideVecImpl: kMaplyWideVecImplDefault,
        kMaplyWideVecImpl: kMaplyWideVecImplPerf,
    };

    [viewC addWideVectors:objs desc:wideDesc mode:MaplyThreadCurrent];

    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:wideDesc];
    for (int i = 0; i < 5; ++i) {
        const auto cc = 0.2f * i;
        UIColor *clColor = [UIColor colorWithRed:0 green:1.0f-cc blue:cc alpha:0.5];
        objs[i].attributes[kMaplyColor] = clColor;
    }
    [viewC addVectors:objs desc:desc mode:MaplyThreadCurrent];
}

- (void)overlap:(MaplyBaseViewController *)viewC {
    const auto cx = -90.0;
    const auto cy = 40.0;
    const auto cs = 0.1;
    for (int k = 0; k < 2; ++k)     // rows
    for (int j = 0; j < 2; ++j)     // cols
    {
        const int csep = 1;  // column separation
        const int rsep = 2;  // row, 0 to overlay for def/perf comparison
        for (int i = 0; i < 5; ++i) {   // lines
            const MaplyCoordinate coords[] = {
                MaplyCoordinateMakeWithDegrees(cx -  5*cs + i*1*cs + j*csep, cy +  5*cs - k*rsep),
                MaplyCoordinateMakeWithDegrees(cx +  0*cs + i*2*cs + j*csep, cy -  0*cs - k*rsep),
                MaplyCoordinateMakeWithDegrees(cx + 10*cs - i*2*cs + j*csep, cy - 10*cs - k*rsep),
            };

            const auto cc = 0.2f * i;
            UIColor *vecColor = [UIColor colorWithRed:0 green:cc blue:1.0f-cc alpha:0.5];
            UIColor *clColor = [UIColor colorWithRed:0 green:1.0f-cc blue:cc alpha:0.5];
            NSDictionary *vecDesc = @{
                kMaplyColor: k ? vecColor : [NSNull null],
            };

            MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:&coords[0]
                                                                            numCoords:sizeof(coords)/sizeof(coords[0])
                                                                           attributes:vecDesc];
            [vecObj subdivideToGlobe:0.0001];

            // Set 0: default implementation, color=blue
            // Set 1: performance implementation, color=red
            // Set 2: performance implementation, color=blue
            NSDictionary *wideDesc = @{
                kMaplyColor: j ? [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.2] :
                                 [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.2],
                kMaplyFilled: @NO,
                kMaplyEnable: @YES,
                kMaplyFade: @0,
                kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 1),
                kMaplyVecCentered: @YES,
                kMaplyVecTexture: [NSNull null],
                kMaplyWideVecEdgeFalloff:@(i),
                kMaplyWideVecJoinType: kMaplyWideVecMiterJoin,
                kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                kMaplyWideVecCoordType: kMaplyWideVecCoordTypeScreen,
                kMaplyWideVecOffset: @(2 * i),
                kMaplyWideVecMiterLimit: @(10.0),  // More than 10 degrees need a bevel join
                kMaplyVecWidth: @(20.0),
                kMaplyWideVecImpl: j ? kMaplyWideVecImplDefault : kMaplyWideVecImplPerf,
            };

            [viewC addWideVectors:@[vecObj] desc:wideDesc mode:MaplyThreadCurrent];

            // Add a centerline for visualizing offsets
            NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:wideDesc];
            if (k) {
                vecObj.attributes[kMaplyColor] = clColor;
            } else {
                desc[kMaplyColor] = [UIColor magentaColor];
            }
            [viewC addVectors:@[vecObj] desc:desc mode:MaplyThreadCurrent];
        }
    }
}

- (void) exprs:(MaplyBaseViewController *)viewC withLoader:(MaplyQuadLoaderBase*)loader perf:(bool)perf {

    const MaplyCoordinate coords[] = {
        MaplyCoordinateMakeWithDegrees(-100, 60 + (perf?0:2)),
        MaplyCoordinateMakeWithDegrees(-110, 61 + (perf?0:2)),
        MaplyCoordinateMakeWithDegrees(-120, 62 + (perf?0:2)),
    };

    MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithLineString:&coords[0]
                                                                    numCoords:sizeof(coords)/sizeof(coords[0])
                                                                   attributes:nil];
    [vecObj subdivideToGlobe:0.0001];

    NSMutableArray<MaplyVectorObject *> *objs = [NSMutableArray array];
    [objs addObject:vecObj];

    NSDictionary *desc = @{
        kMaplyColor: [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:1.0],
        kMaplyEnable: @YES,
        kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault + 2),
    };

    // Note that for GeographyClass, this loader only does zoom levels 0-6
    const int slot = [loader getZoomSlot];

    UIColor *c1 = [UIColor colorWithRed:1.0 green:0.0 blue:1.0 alpha:0.8];
    UIColor *c2 = [UIColor colorWithRed:0.0 green:0.0 blue:1.0 alpha:0.8];
    NSDictionary *wideDesc = [desc dictionaryByMergingWith:@{
        kMaplyDrawPriority:       @(kMaplyVectorDrawPriorityDefault + 1),
        kMaplyWideVecEdgeFalloff: @(1),
        kMaplyZoomSlot:           @(slot),
        kMaplyVecWidth:           @{@"stops":@[@[@2,@1],@[@6,@20]]},
        kMaplyWideVecOffset:      @{@"stops":@[@[@2,@-20],@[@6,@20]]},
        kMaplyOpacity:            @{@"stops":@[@[@2,@0.2],@[@6,@0.9]]},
        kMaplyColor:              @{@"stops":@[@[@2,c1],@[@6,c2]]},
        kMaplyShader:             perf ? kMaplyShaderWideVectorPerformance : kMaplyShaderWideVectorExp,
        kMaplyWideVecImpl:        perf ? kMaplyWideVecImplPerf : kMaplyWideVecImplDefault,
    }];

    NSMutableArray<MaplyComponentObject*> *cos = [NSMutableArray new];
    if (MaplyComponentObject *co = [viewC addVectors:objs desc:desc mode:MaplyThreadCurrent])
    {
        [cos addObject:co];
    }
    if (MaplyComponentObject *co = [viewC addWideVectors:objs desc:wideDesc mode:MaplyThreadCurrent])
    {
        [cos addObject:co];
    }
}

- (void)wideLineTest:(MaplyBaseViewController *)viewC
{
    [self addGeoJson:@"sawtooth.geojson" dashPattern:nil width:50.0 edge:20.0 simple:false viewC:viewC];
    [self addGeoJson:@"moving-lawn.geojson" viewC:viewC];
    [self addGeoJson:@"spiral.geojson" viewC:viewC];
    [self addGeoJson:@"square.geojson" dashPattern:@[@2, @2] width:10.0 viewC:viewC];
    [self addGeoJson:@"track.geojson" viewC:viewC];
//    [self addGeoJson:@"uturn2.geojson" dashPattern:@[@16, @16] width:40 viewC:viewC];

    [self addGeoJson:@"USA.geojson" viewC:viewC];

//    [self addGeoJson:@"testJson.json" viewC:viewC];
    
    //    [self addGeoJson:@"straight.geojson"];
    //    [self addGeoJson:@"uturn.geojson"];
    
    [self overlap:viewC];
    
    [self vecColors:viewC];

    // Dynamic properties require a zoom slot, which may not be set up yet
    __weak MaplyQuadLoaderBase *weakLoader = [baseCase getLoader];
    [weakLoader addPostInitBlock:^{
        if (__strong MaplyQuadLoaderBase *loader = weakLoader)
        {
            [self exprs:viewC withLoader:loader perf:false];
            [self exprs:viewC withLoader:loader perf:true];
        }
    }];
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
