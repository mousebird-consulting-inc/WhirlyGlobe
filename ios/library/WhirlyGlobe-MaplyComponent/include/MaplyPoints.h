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

/** @brief The Maply Points object is used to add a large number of static points to the scene.
    @details Rather than add a single 3D point we assume you want to add a lot of them all at once.  This object lets you do that and lets you assign the various data values to input attributes in your custom shader.
    @details All the cool kids have custom shaders.
  */
@interface MaplyPoints : NSObject

/// @brief Initialie with a hint as to the number of points you'll be adding (not required).
- (__nonnull id)initWithNumPoints:(int)numPoints;

/// @brief Transform to apply to the point locations.  A center is good.
@property (nonatomic) MaplyMatrix * __nullable transform;

/// @brief Add a geocoordinate in lon/lat and Z (meters).
- (void)addGeoCoordLon:(float)x lat:(float)y z:(float)z;

/// @brief Directly add a coordinate in display space.  Remember the globe is a sphere with radius = 1.0.
- (void)addDispCoordX:(float)x y:(float)y z:(float)z;

/// @brief Add a display space coordinate, but use doubles for precision.
- (void)addDispCoordDoubleX:(double)x y:(double)y z:(double)z;

/// @brief Add a color, which will be converted to 8 bits before going to the shader.
- (void)addColorR:(float)r g:(float)g b:(float)b a:(float)a;

/** @brief Add a new attribute array of the given type.
    @details If you have a custom shader, this is a convenient way to pass a large array of attributes to it.  Just specify the name (attribute name in the shader) and the type and then add the appropriate values.  The data will be handed down to the shader at render time.
    @param attrName The name of the attribute as used by the shader.
    @param type The data type of the attribute.
    @return An index (or -1 if invalid) for the attribute.  Use this in the addAttribute calls.
  */
- (int)addAttributeType:(NSString *__nonnull)attrName type:(MaplyShaderAttrType)type;

/// @brief Add an integer attribute.
- (void)addAttribute:(int)whichAttr iVal:(int)val;

/// @brief Add a float attribute.
- (void)addAttribute:(int)whichAttr fVal:(float)val;

/// @brief Add a two component float attribute.
- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY;

/// @brief Add a three component float attribute.
- (void)addAttribute:(int)whichAttr fValX:(float)fValX fValY:(float)valY fValZ:(float)valZ;

/// @brief Add a three component float attribute, but we'll store it at doubles until it gets to the shader.
- (void)addAttribute:(int)whichAttr valX:(double)valX valY:(double)valY valZ:(double)valZ;

/// @brief Add a four commponent float attribute.
- (void)addAttribute:(int)whichAttr fValX:(float)valX fValY:(float)valY fValZ:(float)valZ fValW:(float)valW;

@end
