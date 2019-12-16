//
//  GeoJSONSource.h
//  AutoTester
//
//  Created by Ranen Ghosh on 2016-11-18.
//  Copyright © 2016-2019 mousebird consulting.
//

#import <UIKit/UIKit.h>
#import "control/MaplyBaseViewController.h"

#define GEOJSON_MAX_POINTS 4096

/**
 This class will read GeoJSON via URL with an associated Styled Layer Descriptor via URL.  It will then
  parse both of them and apply the SLD style to the GeoJSON data.  This results in visual data in
 much the same way as loading vector tiles would.
 */
@interface GeoJSONSource : NSObject

- (id _Nullable)initWithViewC:(NSObject<MaplyRenderControllerProtocol> * _Nonnull)baseVC GeoJSONURL:(NSURL * _Nonnull)geoJSONURL sldURL:(NSURL * _Nonnull)sldURL relativeDrawPriority:(int)relativeDrawPriority ;

- (void)startParseWithCompletion:(nonnull void (^)(void)) completionBlock;

- (void)startParse;

@property (nonatomic, readonly) bool loaded;
@property (nonatomic, assign) bool enabled;

@end
