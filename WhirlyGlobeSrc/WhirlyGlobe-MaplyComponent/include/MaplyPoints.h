/*
 *  MaplyPoints.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/21/15
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

#import <UIKit/UIKit.h>
#import "MaplyCoordinate.h"
#import "MaplyBaseViewController.h"
#import "MaplyMatrix.h"

@interface MaplyPoints : NSObject

- (__nonnull id)initWithNumPoints:(int)numPoints;

@property (nonatomic) MaplyMatrix * __nullable transform;

- (void)addGeoCoordLon:(float)x lat:(float)y z:(float)z;

- (void)addDispCoordX:(float)x y:(float)y z:(float)z;

- (void)addDispCoordDoubleX:(double)x y:(double)y z:(double)z;

- (void)addColorR:(float)r g:(float)g b:(float)b a:(float)a;

- (int)addAttributeType:(NSString *__nonnull)attrName type:(MaplyShaderAttrType)type;

- (void)addAttribute:(int)whichAttr iVal:(int)val;

- (void)addAttribute:(int)whichAttr fVal:(float)val;

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY;

- (void)addAttribute:(int)whichAttr fValX:(float)fValX fValY:(float)valY fValZ:(float)valZ;

- (void)addAttribute:(int)whichAttr valX:(double)valX valY:(double)valY valZ:(double)valZ;

- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ fValW:(float)valW;

@end
