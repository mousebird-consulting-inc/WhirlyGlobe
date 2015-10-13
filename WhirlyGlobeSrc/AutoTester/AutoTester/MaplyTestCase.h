//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmWork on 13/10/15.
//  Copyright © 2015 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>


typedef void (^TestCaseResult)(BOOL failed);


@interface MaplyTestCase : NSObject

- (void)runTest;

@property (nonatomic, strong) NSString *name;
@property (nonatomic, copy) TestCaseResult resultBlock;

@end
