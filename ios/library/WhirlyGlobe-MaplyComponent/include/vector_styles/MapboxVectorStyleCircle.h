/*
*  MapboxVectorStyleCircle.h
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

/**
 This is the pain portion o the MapboxVectorLayer Circle.
 
 You control the look of any rendered circles with this.  It would typically be produced in parsing a Mapbox Vector Style.
 */
@interface MapboxVectorCirclePaint : NSObject

/// Radius, in pixels, of the circle to be produced
@property (nonatomic) float radius;
/// Filled color o the circles
@property (nonatomic,strong) UIColor *fillColor;
/// Filled opacity of the circles
@property (nonatomic) float opacity;
/// Stroke width, in pixels, around the outside of the circles
@property (nonatomic) float strokeWidth;
/// Color of the stroke around the outside of the circles
@property (nonatomic,strong) UIColor *strokeColor;
/// Opacity o the stroke around the outside of the circles
@property (nonatomic) float strokeOpacity;

/// :nodoc:
- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry styleSet:(MapboxVectorStyleSet *)styleSet viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

/**
 This layer type produces simple circles that may have an inside and outside stroke.  Nice for data
 visualization.
 
 You don't create these.  They come from a Style sheet.
 */
@interface MapboxVectorLayerCircle : MaplyMapboxVectorStyleLayer

/// Controls the visual representation of the circles
@property (nonatomic,strong) MapboxVectorCirclePaint *paint;

/// :nodoc:
- (instancetype)initWithStyleEntry:(NSDictionary *)styleEntry parent:(MaplyMapboxVectorStyleLayer *)refLayer styleSet:(MapboxVectorStyleSet *)styleSet drawPriority:(int)drawPriority viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end


