/*
 *  MaplyVectorStyle.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 1/3/14.
 *  Copyright 2011-2014 mousebird consulting
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
 *
 */

#import <UIKit/UIKit.h>
#import "MaplyQuadPagingLayer.h"

@interface MaplyVectorTileStyleSettings : NSObject

@property (nonatomic) float lineScale;
@property (nonatomic) float textScale;
@property (nonatomic) float markerScale;
@property (nonatomic) float mapScaleScale;

@end

@interface MaplyVectorTileStyle : NSObject

@property (nonatomic) NSString *uuid;

+ (id)styleFromStyleEntry:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings viewC:(MaplyBaseViewController *)viewC;

- (id)initWithStyleEntry:(NSDictionary *)styleEntry viewC:(MaplyBaseViewController *)viewC;

// Turn the min/maxscaledenom into height ranges for minVis/maxVis
- (void)resolveVisibility:(NSDictionary *)styleEntry settings:(MaplyVectorTileStyleSettings *)settings desc:(NSMutableDictionary *)desc;

- (NSArray *)buildObjects:(NSArray *)vecObjs viewC:(MaplyBaseViewController *)viewC;

@property (nonatomic,weak) MaplyBaseViewController *viewC;
@property (nonatomic) bool geomAdditive;

@end
