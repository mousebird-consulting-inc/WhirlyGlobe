//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MaplyTestResult.h"
#import "WhirlyGlobeComponent.h"


@class MaplyViewController;
@class WhirlyGlobeViewController;
@class MaplyTestCase;
@class MaplyCoordinateSystem;

typedef void (^TestCaseResult)(MaplyTestCase * _Nonnull testCase);

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseOptions) {
	MaplyTestCaseOptionGlobe = 1 << 1,
	MaplyTestCaseOptionMap   = 1 << 2,
};

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseImplementations) {
	MaplyTestCaseImplementationGlobe = 1 << 1,
	MaplyTestCaseImplementationMap   = 1 << 2,
};

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseState) {
	MaplyTestCaseStateDownloading,
	MaplyTestCaseStateReady,
	MaplyTestCaseStateSelected,
	MaplyTestCaseStateError,
	MaplyTestCaseStateRunning,
};

@interface MaplyTestCase : NSObject <WhirlyGlobeViewControllerDelegate,MaplyViewControllerDelegate>

- (void)start;

// these prototypes are necessary for Swift
- (void)setUpWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (NSArray * _Nullable)remoteResources;

- (void)setUpWithMap:(MaplyViewController * _Nonnull)mapVC;
- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC;
- (void) removeGlobeController;
- (void) removeMapController;
- (MaplyCoordinateSystem * _Nullable)customCoordSystem;

@property (nonatomic, strong) UIView * _Nullable testView;
@property (nonatomic, strong) NSString * _Nonnull name;
@property (nonatomic, copy) TestCaseResult _Nullable resultBlock;
@property (nonatomic) NSInteger captureDelay;

@property (nonatomic) MaplyTestCaseOptions options;
@property (nonatomic) MaplyTestCaseImplementations implementations;

@property (nonatomic) MaplyTestCaseState state;
@property (nonatomic) BOOL interactive;

@property (nonatomic) NSInteger pendingDownload;

@property (nonatomic, copy, nullable) void (^updateProgress)(BOOL enableIndicator);


@property (nonatomic, weak) MaplyBaseViewController * _Nullable baseViewController;
@property (nonatomic, strong) WhirlyGlobeViewController *_Nullable globeViewController;
@property (nonatomic, strong) MaplyViewController * _Nullable mapViewController;

@property (nonatomic, readonly) MaplyTestResult * _Nullable globeResult;
@property (nonatomic, readonly) MaplyTestResult * _Nullable mapResult;

@end
