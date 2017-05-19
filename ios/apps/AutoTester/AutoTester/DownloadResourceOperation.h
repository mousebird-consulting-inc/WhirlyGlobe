//
//  DownloadResourceOperation.h
//  AutoTester
//
//  Created by jmnavarro on 13/5/16.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MaplyTestCase.h"

@interface DownloadResourceOperation : NSOperation

- (id) initWithUrl:(NSURL *)url test:(MaplyTestCase*)test;

@end
