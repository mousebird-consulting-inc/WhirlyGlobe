//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting.
//

#import <UIKit/UIKit.h>
#import "WhirlyGlobeComponent.h"
#import "MaplyComponent.h"

@class MaplyViewController;
@class WhirlyGlobeViewController;
@class MaplyTestCase;
@class MaplyCoordinateSystem;

typedef void (^TestCaseResult)(MaplyTestCase * _Nonnull testCase);

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseImplementations) {
	MaplyTestCaseImplementationGlobe = 1 << 1,
	MaplyTestCaseImplementationMap   = 1 << 2,
};

@interface MaplyTestCase : NSObject <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate>

- (void)startGlobe:(UINavigationController * __nonnull)nav;
- (void)startMap:(UINavigationController * __nonnull)nav;

- (void)setUpWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (void)setUpWithMap:(MaplyViewController * _Nonnull)mapVC;

- (void)stop;
- (MaplyCoordinateSystem * _Nullable)customCoordSystem;

@property (nonatomic, strong) NSString * _Nonnull name;

@property (nonatomic) MaplyTestCaseImplementations implementations;

@property (nonatomic, weak) MaplyBaseViewController * _Nullable baseViewController;
@property (nonatomic, strong) WhirlyGlobeViewController *_Nullable globeViewController;
@property (nonatomic, strong) MaplyViewController * _Nullable mapViewController;

@end
