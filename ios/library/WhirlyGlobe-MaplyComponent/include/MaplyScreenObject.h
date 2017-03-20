/*
 *  MaplyScreenObject
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 2/28/15
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

/** @brief The Maply Screen Object is used to build up a more complex screen object from multiple pieces.
    @details You can use one or more of these to build up a combination of labels and images that form a single marker, label, or billboard.
  */
@interface MaplyScreenObject : NSObject

/** @brief Add a string to the screen object
    @param str The string to add
    @param font The font to use
    @param color The foreground color of the string.
  */
- (void)addString:(NSString *)str font:(UIFont *)font color:(UIColor *)color;

/** @brief Add an attributed string to the screen object.
    @details This adds an annotated string to the screen object.  The size will be based on the lenght of the string and the font.
  */
- (void)addAttributedString:(NSAttributedString *)str;

/** @brief Add an image scaled to the given size.
    @details Adds a UIImage or MaplyTexture object scaled to the given size.
  */
- (void)addImage:(id)image color:(UIColor *)color size:(CGSize)size;

/** @brief Calculate and return the current bounding box of the screen object.
  */
- (MaplyBoundingBox)getSize;

/** @brief Apply a scale to all the pieces of the screen object.
  */
- (void)scaleX:(double)x y:(double)y;

/** @brief Apply a translation to all the pieces of the screen object.
  */
- (void)translateX:(double)x y:(double)y;

/** @brief Add the contents of the given screen object to this screen object.
  */
- (void)addScreenObject:(MaplyScreenObject *)screenObj;

@end
