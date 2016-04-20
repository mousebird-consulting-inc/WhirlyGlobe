//
//  MapjamSource.h
//  AutoTester
//
//  Created by Steve Gifford on 4/18/16.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobeComponent.h>
#import "MapboxVectorTiles.h"

@interface MapjamSource : NSObject<MaplyPagingDelegate>

@property (nonatomic,assign) int minZoom,maxZoom;

// From the style sheet
@property (nonatomic) UIColor *backgroundColor;

/** @brief Initialize with the base URL, API key and location of the style
 */
- (id)initWithBase:(NSString *)inBaseURL apiKey:(NSString *)apiKey styleJSON:(NSData *)styleJSON viewC:(MaplyBaseViewController *)viewC;

@end
