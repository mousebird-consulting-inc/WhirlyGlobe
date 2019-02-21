//
//  CartoDBLayer.h
//  HelloEarth
//
//  Created by Steve Gifford on 11/18/14.
//  Copyright © 2014-2017 mousebird consulting.
//

#import <Foundation/Foundation.h>
#import <WhirlyGlobeComponent.h>

/** CartoDB Interpreter fetches tiles of GeoJSON data from their NY building data set.
 
    It implements MaplyTileInfoNew to provide URLs and min/max zoom.
    IT also implements MaplyLoaderInterpreter to convert that data into geometry.
  */
@interface CartoDBInterpreter : NSObject <MaplyLoaderInterpreter,MaplyTileInfoNew>

@property (nonatomic,assign) int minZoom,maxZoom;

@property (nonatomic,weak) MaplyQuadPagingLoader *loader;

// Create with the search string we'll use
- (id)initWithSearch:(NSString *)inSearch;

@end
