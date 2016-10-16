//
//  ActiveObjectTestCase.m
//  AutoTester
//
//  Created by Steve Gifford on 9/9/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import "ActiveObjectTestCase.h"
#import "CartoDBLightTestCase.h"

@interface SimpleActiveObject : MaplyActiveObject
@end

@implementation SimpleActiveObject

- (nonnull instancetype)initWithViewController:(MaplyBaseViewController *__nonnull)viewC
{
    self = [super initWithViewController:viewC];
    
    return self;
}

- (bool)hasUpdate
{
    return true;
}

- (void)updateForFrame:(id)frameInfo
{
    // Do your per frame updates here
}

@end

@implementation ActiveObjectTestCase
{
    SimpleActiveObject *activeObject;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.name = @"Active Object Test Case";
        self.captureDelay = 5;
        self.implementations = MaplyTestCaseImplementationMap | MaplyTestCaseImplementationGlobe;
    }
    
    return self;
}

- (void)setupActiveObject:(MaplyBaseViewController *)viewC
{
    activeObject = [[SimpleActiveObject alloc] initWithViewController:viewC];
    
    [viewC addActiveObject:activeObject];
}

- (void)setUpWithGlobe:(WhirlyGlobeViewController *)globeVC
{
    CartoDBLightTestCase *baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithGlobe:globeVC];
    
    [self setupActiveObject:globeVC];
}

- (void)setUpWithMap:(MaplyViewController *)mapVC
{
    CartoDBLightTestCase *baseCase = [[CartoDBLightTestCase alloc] init];
    [baseCase setUpWithMap:mapVC];

    [self setupActiveObject:mapVC];
}
@end
