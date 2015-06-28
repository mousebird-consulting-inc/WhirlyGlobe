//
//  CartoDBLayer.h
//  HelloEarth
//
//  Created by Chris Lamb on 6/28/15.
//  Copyright (c) 2015 com.SantaCruzNewspaperTaxi. All rights reserved.
//

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