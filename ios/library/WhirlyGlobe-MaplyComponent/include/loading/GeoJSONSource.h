/*  GeoJSONSource.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Ranen Ghosh on 2016-11-18.
 *  Copyright 2016-2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

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
