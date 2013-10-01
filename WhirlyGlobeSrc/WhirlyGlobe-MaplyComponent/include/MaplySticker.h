/*
 *  MaplySticker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 11/27/12.
 *  Copyright 2012 mousebird consulting
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
#import <MaplyCoordinate.h>

/** @brief Stickers are rectangles placed on the globe with an image.
    @details The Maply Sticker will stretch a rectangle (in geographic) over the given extents and tack the given image on top of it.  Stickers differ from MaplyMarker objects in that they're big.  They can stretch over a larger are and need to be subdivided as such.
  */
@interface MaplySticker : NSObject

/// @brief The lower left corner (in geographic) of the sticker
@property (nonatomic,assign) MaplyCoordinate ll;
/// @brief The upper right corner (in geographic) of the sticker
@property (nonatomic,assign) MaplyCoordinate ur;
/// @brief Angle of rotation around center
@property (nonatomic,assign) float rotation;
/** @brief Image to stretch over the sticker.
    @details The UIImage is cached in the view controller, so multiple references will result in the same texture being used.  The view controller also cleans up the images when it's done with it.
  */
@property (nonatomic,strong) UIImage *image;

@end
