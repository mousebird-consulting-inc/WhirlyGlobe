//
//  WideVectorGlobeTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 7/21/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "WideVectorGlobeTestCase.h"
#import "MaplyBaseViewController.h"
#import "MaplyTextureBuilder.h"
#import "MaplyScreenLabel.h"
#import "GeographyClassTestCase.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation WideVectorGlobeTestCase

- (instancetype)init
{
    if (self = [super init]) {
        self.captureDelay = 20;
        self.name = @"Wide Vector Backface";
        self.implementations = MaplyTestCaseImplementationGlobe | MaplyTestCaseImplementationMap;
    }
    return self;
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
        NSDictionary *jsonDictionary = [NSJSONSerialization JSONObjectWithData:data
                                                                       options:0 error:nil];
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSONDictionary:jsonDictionary];
        if(vecObj) {
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
    return [self addGeoJson:name dashPattern:@[@8, @8] width:4 viewC:viewC];
    //    return [self addGeoJson:name dashPattern:@[@8, @8] width:20 viewC:viewC];
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
    
    
    return @[lines,screenLines];
}

- (void)wideLineTest:(MaplyBaseViewController *)viewC
{
    [self addGeoJson:@"USA.geojson" viewC:viewC];
}


- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC{
    
    GeographyClassTestCase * baseLayer = [[GeographyClassTestCase alloc]init];
    [baseLayer setUpWithGlobe:globeVC];
    [self wideLineTest:globeVC];
    [globeVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:0.1];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC{
    GeographyClassTestCase * baseLayer = [[GeographyClassTestCase alloc]init];
    [baseLayer setUpWithMap:mapVC];
    [self wideLineTest:mapVC];
    [mapVC animateToPosition:MaplyCoordinateMakeWithDegrees(-122.4192, 37.7793) time:0.1];
}


@end
