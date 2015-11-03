//
//  MapzenSource.h
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 11/20/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobeComponent.h>
#import "MaplyMapnikVectorTiles.h"

typedef enum {MapzenSourceGeoJSON, MapzenSourcePBF } MapzenSourceType;

/** @brief Mapzen Source type.  Handles fetching from Mapzen.
    @details Implements a paging delegate that can fetch Mapzen vector tile data.
  */
@interface MapzenSource : NSObject<MaplyPagingDelegate>

@property (nonatomic,assign) int minZoom,maxZoom;

// From the style sheet
@property (nonatomic) UIColor *backgroundColor;

/** @brief Initialize with the base URL and the layers we want to fetch.
  */
- (id)initWithBase:(NSString *)inBaseURL layers:(NSArray *)inLayers apiKey:(NSString *)apiKey sourceType:(MapzenSourceType)inType styleData:(NSData *)styleData styleType:(MapnikStyleType) styleType viewC:(MaplyBaseViewController *)viewC;

@end
