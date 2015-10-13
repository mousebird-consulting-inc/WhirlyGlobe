//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MaplyTestResult.h"

@class MaplyViewController;
@class WhirlyGlobeViewController;
@class MaplyTestCase;

typedef void (^TestCaseResult)(MaplyTestCase *testCase);


@interface MaplyTestCase : NSObject

- (void)start;

- (void)setUp;
- (void)tearDown;

@property (nonatomic, strong) UIView *testView;
@property (nonatomic, strong) NSString *name;
@property (nonatomic, copy) TestCaseResult resultBlock;
@property (nonatomic) NSInteger captureDelay;

@property (nonatomic, strong) WhirlyGlobeViewController *globeViewController;
@property (nonatomic, strong) MaplyViewController *mapViewController;

@property (nonatomic, readonly) MaplyTestResult *result;

@end
