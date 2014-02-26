/*
 *  MaplyImageTile.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/18/13.
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

/** @brief Describes a single tile worth of data, which may be multiple images.
    @details Delegates can pass back a single UIImage or NSData object, but if they want to do anything more complex, they need to do it with this.
 */
@interface MaplyImageTile : NSObject

/// @brief Initialize as a placeholder image.
/// @details Placeholder images are blank, but they allow the pager to keep loading their children.
- (id)initAsPlaceholder;

/** @brief Initialize with an NSData object containing 32 bit pixels.
    @details This sets up the tile with an NSData object containing raw pixels.  The pixels are 32 bit RGBA even if you're targeting a smaller pixel format.
    @param data The NSData object containing 32 bit RGBA pixels.
    @param width The width of the raw image contained in the data object.
    @param height The height of the raw image contained in the data object.
 */
- (id)initWithRawImage:(NSData *)data width:(int)width height:(int)height;

/** @brief Initialize with an array of NSData objects containing 32 bit pixels.
    @details This does the same thng as initWithRawData:width:height: but for tiles that contain multiple return images.
    @param data The NSArray of NSData objects containing 32 bit RGBA pixels.
    @param width The width of the raw image contained in the data object.
    @param height The height of the raw image contained in the data object.
    @see initWithRawData:width:height:
 */
- (id)initWithRawImageArray:(NSArray *)data width:(int)width height:(int)height;

/** @brief Initialize with a single UIImage for the tile.
    @details This sets up the given UIImage as the return for the given tile.  You can then set targetSize and such.
 */
- (id)initWithImage:(UIImage *)image;

/** @brief Initialize with an NSArray of UIImage objects for a tile that requires multiple return images.
 */
- (id)initWithImageArray:(NSArray *)images;

/** @brief Initialize with an NSData object containing PNG or JPEG data that can be interpreted by UIImage.
 */
- (id)initWithPNGorJPEGData:(NSData *)data;

/** @brief Initialize with an NSArray of NSData objects containing PNG or JPEG data that can be interpreted by UIImage
    @details This is for tiles that require multiple images.
 */
- (id)initWithPNGorJPEGDataArray:(NSArray *)data;

/** @brief Initialize with an NSObject.  We'll try to figure out what it is from the type.
    @details We'll look at the data type and try to figure out what you're passing in.  In general, it's better to call one of the specific init routines.
  */
- (id)initWithRandomData:(id)theObj;

/** @brief Target size for the image(s) represented by this tile.
    @details This instructs the pager to rescale the image(s) to the given target size.  This is probably faster than doing it yourself because we can extract the data and rescale in the same step.
 */
@property (nonatomic) CGSize targetSize;

/** @brief Image size.
    @details This is the size of the image as it presently exists.
  */
@property (nonatomic,readonly) CGSize size;

@end
