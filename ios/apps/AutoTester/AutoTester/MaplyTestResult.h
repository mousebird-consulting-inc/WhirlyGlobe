//
//  TestResult.h
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MaplyTestResult : NSObject

@property (nonatomic, readonly) BOOL passed;
@property (nonatomic, strong, readonly) NSString * _Nonnull testName;
@property (nonatomic, strong, readonly) NSString * _Nonnull baselineImageFile;
@property (nonatomic, strong, readonly) NSString * _Nullable actualImageFile;

- (instancetype _Nonnull)initWithTestName:(NSString * _Nonnull)testName
								 baseline:(NSString * _Nonnull)baseline
								   actual:(NSString * _Nullable)actual
								   passed:(BOOL)passed;

@end
