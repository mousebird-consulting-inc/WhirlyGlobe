//
//  GeoJSONSource.h
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-18.
//  Copyright © 2016 mousebird consulting. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MaplyComponent.h"
//#import "GSNParser.h"

#define GEOJSON_MAX_POINTS 4096

@interface GeoJSONSource : NSObject

- (id)initWithViewC:(MaplyBaseViewController *)baseVC GeoJSONURL:(NSURL *)geoJSONURL sldURL:(NSURL *)sldURL;

- (void)startParseWithCompletion:(nonnull void (^)()) completionBlock;

- (void)startParse;

@property (nonatomic, readonly) bool loaded;
@property (nonatomic, assign) bool enabled;

@end
