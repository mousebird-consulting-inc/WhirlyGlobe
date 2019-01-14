//
//  GeoJSONSource.h
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-18.
//  Copyright © 2016-2017 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MaplyBaseViewController.h"

#define GEOJSON_MAX_POINTS 4096

@interface GeoJSONSource : NSObject

- (id _Nullable)initWithViewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)baseVC GeoJSONURL:(NSURL * _Nonnull)geoJSONURL sldURL:(NSURL * _Nonnull)sldURL relativeDrawPriority:(int)relativeDrawPriority ;

- (void)startParseWithCompletion:(nonnull void (^)(void)) completionBlock;

- (void)startParse;

@property (nonatomic, readonly) bool loaded;
@property (nonatomic, assign) bool enabled;

@end
