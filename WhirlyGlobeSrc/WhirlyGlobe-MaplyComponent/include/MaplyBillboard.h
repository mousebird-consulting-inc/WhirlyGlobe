/*
 *  MaplyBillboard.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 10/28/13.
 *  Copyright 2011-2013 mousebird consulting
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

/** @brief A billboard is tied to a specific point, but rotates to face the user.
    @details The billboard object represents a rectangle which is rooted at a specific point, but will rotate to face the user.  These are typically used in 3D, when the globe or map has a tilt.  They make little sense in 2D.
  */
@interface MaplyBillboard : NSObject

/// @brief The point this billboard is rooted at.
@property (nonatomic) MaplyCoordinate3d center;

/// @brief Size of the billboard in display coordinates.
@property (nonatomic) CGSize size;

/// @brief Color of the billboard's underlying polygon.
@property (nonatomic) UIColor *color;

/// @brief The UIImage (or MaplyTexture) to apply to the billboard.
@property (nonatomic,strong) id image;

/// @brief Set if you want to select these
@property (nonatomic) bool selectable;

/** @brief User data object for selection
 @details When the user selects a feature and the developer gets it in their delegate, this is an object they can use to figure out what the screen label means to them.
 */
@property (nonatomic,strong) id userObject;

@end
