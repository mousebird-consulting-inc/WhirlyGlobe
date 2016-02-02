/*
 *  MaplyBillboard.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/28/13.
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
#import "MaplyScreenObject.h"

/** @brief A billboard is tied to a specific point, but rotates to face the user.
    @details The billboard object represents a rectangle which is rooted at a specific point, but will rotate to face the user.  These are typically used in 3D, when the globe or map has a tilt.  They make little sense in 2D.
  */
@interface MaplyBillboard : NSObject

/** @brief The point this billboard is rooted at.
    @details The x and y coordinates are radians.  The z coordinate is in meters.
  */
@property (nonatomic) MaplyCoordinate3d center;

/// @brief Set if you want to select these
@property (nonatomic) bool selectable;

/// @brief The 2D polygonal description of what the billboard should be
@property (nonatomic,strong) MaplyScreenObject *screenObj;

/** @brief Vertex attributes to apply to this billboard.
    @details MaplyVertexAttribute objects are passed all the way to the shader.  Read that page for details on what they do.
    @details The array of vertex attributes provided here will be copied onto all the vertices we create for the shader.  This means you can use these to do things for a single billboard in your shader.
 */
@property (nonatomic,strong) NSArray *vertexAttributes;

/** @brief User data object for selection
    @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen label means to them.
 */
@property (nonatomic,strong) id userObject;

/** @brief Initialize with a single image, a given background and a size.
    @details This will create a simple billboard with a single image pasted on it.
    @param texture This can either be a UIImage or a MaplyTexture.
    @param color Color for the polygon that makes up the billboard.
    @param size Size of the billboard in display space.
  */
- (id)initWithImage:(id)texture color:(UIColor *)color size:(CGSize)size;

@end
