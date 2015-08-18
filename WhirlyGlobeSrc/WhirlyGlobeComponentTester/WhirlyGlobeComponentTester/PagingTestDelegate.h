//
//  PagingTestDelegate.h
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 8/17/15.
//  Copyright (c) 2015 mousebird consulting. All rights reserved.
//

#import <WhirlyGlobeComponent.h>

/** @details Generates lots of random markers for testing.
  */
@interface PagingTestDelegate : NSObject<MaplyPagingDelegate>

@property (nonatomic,strong) MaplyCoordinateSystem *coordSys;

@end
