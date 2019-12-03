/*
 *  MapboxVectorStyleLine.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/17/15.
 *  Copyright 2011-2019 mousebird consulting
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

#import "vector_styles/MapboxVectorStyleSet.h"

typedef enum {MBLineCapButt,MBLineCapRound,MBLineCapSquare} MapboxVectorLineCap;
typedef enum {MBLineJoinBevel,MBLineJoinRound,MBLineJoinMiter} MapboxVectorLineJoin;

@interface MapboxVectorLineLayout : NSObject

@property (nonatomic) bool visible;
@property (nonatomic) MapboxVectorLineCap cap;
@property (nonatomic) MapboxVectorLineJoin join;
@property (nonatomic) double miterLimit;
@property (nonatomic) double roundLimit;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

@interface MapboxVectorLineDashArray : NSObject
/// @brief Array of NSNumbers with alternating dashes and gaps
@property (nonatomic,strong) NSArray *dashes;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

@interface MapboxVectorLinePaint : NSObject

@property (nonatomic) MapboxTransDouble *opacity;
@property (nonatomic) MapboxTransColor *color;
@property (nonatomic) MapboxTransDouble *width;
@property (nonatomic) MapboxVectorLineDashArray *lineDashArray;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

/// @brief The line style
@interface MapboxVectorLayerLine : MaplyMapboxVectorStyleLayer

@property (nonatomic,strong) MapboxVectorLineLayout *layout;
@property (nonatomic,strong) MapboxVectorLinePaint *paint;
@property (nonatomic) bool linearClipToBounds;
@property (nonatomic) bool dropGridLines;
// If non-zero we'll subdivide the line along a globe to the given tolerance
@property (nonatomic) double subdivToGlobe;

- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end
