/*  MaplyVectorStyleSimple.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 3/15/16.
 *  Copyright 2011-2021 mousebird consulting
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

#import <Foundation/Foundation.h>
#import "vector_tiles/MapboxVectorTiles.h"

/** 
    Simple default style to see something in vector tile data.
    
    A simple vector style that displays each layer in a random color.
    Use this as a starting point for your own style.
  */
@interface MaplyVectorStyleSimpleGenerator : NSObject<MaplyVectorStyleDelegate>

@property (nonatomic,weak) NSObject<MaplyRenderControllerProtocol> *viewC;

/// Initialize with a map view controller
- (id)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/// uuid for the styles
- (long long)generateID;

@end

/** 
    Base class for the simple vector style.
  */
@interface MaplyVectorStyleSimple : NSObject<MaplyVectorStyle>

/// Unique Identifier for this style
@property (nonatomic) long long uuid;

/// Set if this geometry is additive (e.g. sticks around) rather than replacement
@property (nonatomic) bool geomAdditive;

/// Priority for sorting among layers
@property (nonatomic) int drawPriority;

@property (nonatomic,weak) NSObject<MaplyRenderControllerProtocol> *viewC;

- (id)initWithGen:(MaplyVectorStyleSimpleGenerator *)gen viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

@end

/** 
    Simple filled polygon with a random color.
  */
@interface MaplyVectorStyleSimplePolygon : MaplyVectorStyleSimple

@property (nonatomic,strong) UIColor *color;

@end

/** 
    Simple point we'll convert to a label.
  */
@interface MaplyVectorStyleSimplePoint : MaplyVectorStyleSimple

@property (nonatomic,strong) UIFont *font;

@end

/** 
    Simple linear with a random color.
  */
@interface MaplyVectorStyleSimpleLinear : MaplyVectorStyleSimple

@property (nonatomic,strong) UIColor *color;

@end
