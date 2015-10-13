//
//  TestResult.h
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MaplyTestResult : NSObject

@property (nonatomic) BOOL passed;
@property (nonatomic, strong) NSString *testName;
@property (nonatomic, strong) NSString *baselineImageFile;
@property (nonatomic, strong) NSString *actualImageFile;

@end
