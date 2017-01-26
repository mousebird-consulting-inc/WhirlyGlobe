//
//  CartoDBLayer.h
//  HelloEarth
//
//  Created by Steve Gifford on 11/18/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobeComponent.h>

@interface CartoDBLayer : NSObject <MaplyPagingDelegate>
{
	NSString *search;
	NSOperationQueue *opQueue;
}

@property (nonatomic,assign) int minZoom,maxZoom;

// Create with the search string we'll use
- (id)initWithSearch:(NSString *)search;

@end
