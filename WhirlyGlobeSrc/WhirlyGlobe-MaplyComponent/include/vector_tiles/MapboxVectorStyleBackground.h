/*
 *  MapboxVectorStyleBackground.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import "MapboxVectorStyleSet.h"

@interface MapboxVectorBackgroundPaint : NSObject

@property (nonatomic, nullable) MaplyMapboxValueWrapper *color;
@property (nonatomic, nullable) MaplyMapboxValueWrapper *opacity;

- (nullable instancetype)initWithStyleEntry:(NSDictionary *__nonnull)styleEntry styleSet:(MaplyMapboxVectorStyleSet *__nonnull)styleSet viewC:(MaplyBaseViewController *__nonnull)viewC;

@end

/// @brief Background description
@interface MapboxVectorLayerBackground : MaplyMapboxVectorStyleLayer

//@property (nonatomic) MapboxVectorLayoutBackground *layout;
@property (nonatomic, nullable) MapboxVectorBackgroundPaint *paint;

- (nullable instancetype)initWithStyleEntry:(NSDictionary *__nonnull)styleEntry parent:(MaplyMapboxVectorStyleLayer *__nonnull)refLayer styleSet:(MaplyMapboxVectorStyleSet *__nonnull)styleSet drawPriority:(int)drawPriority viewC:(MaplyBaseViewController *__nonnull)viewC;

- (nullable NSArray *)buildObjects:(NSArray *__nonnull)vecObjs forTile:(MaplyTileID)tileID viewC:(MaplyBaseViewController *__nonnull)viewC;

@end
