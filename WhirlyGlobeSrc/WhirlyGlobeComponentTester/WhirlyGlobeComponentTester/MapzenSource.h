//
//  MapzenSource.h
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 11/20/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobeComponent.h>

@interface MapzenSource : NSObject<MaplyPagingDelegate>

@property (nonatomic,assign) int minZoom,maxZoom;

- (id)initWithBase:(NSString *)baseURL layers:(NSArray *)layers;

@end
