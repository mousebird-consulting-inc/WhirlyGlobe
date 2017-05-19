/*
 *  MaplySticker.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 11/27/12.
 *  Copyright 2012-2017 mousebird consulting
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
#import "MaplyQuadImageTilesLayer.h"

/** 
    Stickers are rectangles placed on the globe with an image.
    
    The Maply Sticker will stretch a rectangle (in geographic) over the given extents and tack the given image on top of it.  Stickers differ from MaplyMarker objects in that they're big.  They can stretch over a larger are and need to be subdivided as such.
  */
@interface MaplySticker : NSObject

/// The lower left corner (in geographic) of the sticker
@property (nonatomic,assign) MaplyCoordinate ll;

/// The upper right corner (in geographic) of the sticker
@property (nonatomic,assign) MaplyCoordinate ur;

/// Angle of rotation around center
@property (nonatomic,assign) float rotation;

/** 
    If present, this is the coordinate system the sticker is represented in.
    
    By default the coordinates are in geographic.  If this is present, the coordinates are in this system.
  */
@property (nonatomic,strong) MaplyCoordinateSystem * __nullable coordSys;

/** 
    Image (or MaplyTexture) to stretch over the sticker.
    
    The UIImage (or MaplyTexture) is cached in the view controller, so multiple references will result in the same texture being used.  The view controller also cleans up the images when it's done with it.
  */
@property (nonatomic,strong) id __nullable image;

/** 
    Images to stretch over the sticker.
    
    This is an NSArray of UIImages (or MaplyTextures).  The images will be cached in the view controller, so multiple references will result in the same texture being used.  The view controller also cleans up the images when it's done with them.
    
    All the images passed in here will be presented to the shader program, if it has variables for them.  It's up to you to do something with them in the shader.
  */
@property (nonatomic,strong) NSArray * __nullable images;

/** 
    Set the image format for the created textures.
    
    OpenGL ES offers us several image formats that are more efficient than 32 bit RGBA, but they're not always appropriate.  This property lets you choose one of them.  The 16 or 8 bit ones can save a huge amount of space and will work well for some imagery, most maps, and a lot of weather overlays.
 
 | Image Format | Description |
 |:-------------|:------------|
 | MaplyImageIntRGBA | 32 bit RGBA with 8 bits per channel.  The default. |
 | MaplyImageUShort565 | 16 bits with 5/6/5 for RGB and none for A. |
 | MaplyImageUShort4444 | 16 bits with 4 bits for each channel. |
 | MaplyImageUShort5551 | 16 bits with 5/5/5 bits for RGB and 1 bit for A. |
 | MaplyImageUByteRed | 8 bits, where we choose the R and ignore the rest. |
 | MaplyImageUByteGreen | 8 bits, where we choose the G and ignore the rest. |
 | MaplyImageUByteBlue | 8 bits, where we choose the B and ignore the rest. |
 | MaplyImageUByteAlpha | 8 bits, where we choose the A and ignore the rest. |
 | MaplyImageUByteRGB | 8 bits, where we average RGB for the value. |
 | MaplyImage4Layer8Bit | 32 bits, four channels of 8 bits each.  Just like MaplyImageIntRGBA, but a warning not to do anything too clever in sampling. |
 */
@property (nonatomic) MaplyQuadImageFormat imageFormat;


@end
