/*
 *  MaplyTextureBuilder.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/30/14.
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

/** @brief The Maply Linear Texture Builder is used to construct linear textures
    for use on widened vectors.
    @details Linear textures of this type are used to represent dotted and dashed lines.  These may come from Mapnik configuration files or you can just make them up yourself.
    @details After creating an image with this object, you'll want to pass it as a parameter to the widened vector add method.
  */
@interface MaplyLinearTextureBuilder : NSObject

/** @brief Set the pattern of dots and empty spaces.
    @details This is an array of NSNumbers (treated as integers) that specify how big an element in the given pattern is.  The first element is on, the next off and so forth.
  */
- (void)setPattern:(NSArray *)elements;

/** @brief Build the image from the size and elements specified.
    @details If you've set a reasonable size and added a pattern, this will render the pattern into the image and return it.  If the size of the image differs from the size of the elements, they will be scaled to the image.
  */
- (UIImage *)makeImage;

@end
