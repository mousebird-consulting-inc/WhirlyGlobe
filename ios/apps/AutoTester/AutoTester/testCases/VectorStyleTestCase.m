//
//  VectorStyleTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 8/12/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import "VectorStyleTestCase.h"
#import "GeographyClassTestCase.h"

@implementation VectorStyleTestCase
{
    MaplyBaseViewController *baseViewC;
    GeographyClassTestCase *baseCase;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Vector Style Test";
        self.captureDelay = 5;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
        
    }
    
    return self;
}

- (void)overlayCountryFile:(NSString *)name ext:(NSString *)ext viewC:(MaplyBaseViewController *)viewC
{
    MaplyVectorStyleSimpleGenerator *simpleStyle = [[MaplyVectorStyleSimpleGenerator alloc] initWithViewC:viewC];

    NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:ext];
    if(path) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        NSDictionary *jsonDictionary = [NSJSONSerialization JSONObjectWithData:data
                                                                       options:0 error:nil];
        MaplyVectorObject *vecObj = [[MaplyVectorObject alloc] initWithGeoJSONDictionary:jsonDictionary];
        AddMaplyVectorsUsingStyle(@[vecObj],simpleStyle,viewC,MaplyThreadAny);
    }
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    baseViewC = globeVC;
    baseCase = [[GeographyClassTestCase alloc]init];
    [baseCase setUpWithGlobe:globeVC];
    
    [self overlayCountryFile:@"USA" ext:@"geojson" viewC:globeVC];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    baseViewC = mapVC;
    baseCase = [[GeographyClassTestCase alloc]init];
    [baseCase setUpWithMap:mapVC];
    
    [self overlayCountryFile:@"USA" ext:@"geojson" viewC:mapVC];
}

@end
