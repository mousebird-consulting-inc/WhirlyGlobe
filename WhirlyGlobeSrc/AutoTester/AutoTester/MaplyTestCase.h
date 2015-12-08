//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MaplyTestResult.h"

@class MaplyViewController;
@class WhirlyGlobeViewController;
@class MaplyTestCase;

typedef void (^TestCaseResult)(MaplyTestCase * _Nonnull testCase);

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseOptions) {
	MaplyTestCaseOptionNone  = 0 << 0,
	MaplyTestCaseOptionGlobe = 1 << 1,
	MaplyTestCaseOptionMap   = 1 << 2,
};


@interface MaplyTestCase : NSObject

- (void)start;

// these prototypes are necessary for Swift
- (BOOL)setUpWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (void)tearDownWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;

- (BOOL)setUpWithMap:(MaplyViewController * _Nonnull)mapVC;
- (void)tearDownWithMap:(MaplyViewController * _Nonnull)mapVC;

@property (nonatomic, strong) UIView * _Nullable testView;
@property (nonatomic, strong) NSString * _Nonnull name;
@property (nonatomic, copy) TestCaseResult _Nullable resultBlock;
@property (nonatomic) NSInteger captureDelay;

@property (nonatomic) MaplyTestCaseOptions options;

@property (nonatomic) BOOL selected;
@property (nonatomic) BOOL running;

@property (nonatomic, strong) WhirlyGlobeViewController *_Nullable globeViewController;
@property (nonatomic, strong) MaplyViewController * _Nullable mapViewController;

@property (nonatomic, readonly) MaplyTestResult * _Nullable globeResult;
@property (nonatomic, readonly) MaplyTestResult * _Nullable mapResult;

@end
