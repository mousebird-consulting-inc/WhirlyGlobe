/*
 *  MaplyImageTile.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/18/13.
 *  Copyright 2011-2019 mousebird consulting
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
#import "control/MaplyRenderController.h"

@class MaplyElevationChunk;

/** 
    Describes a single tile worth of data, which may be multiple images.
    
    Delegates can pass back a single UIImage or NSData object, but if they want to do anything more complex, they need to do it with this.
 */
@interface MaplyImageTile : NSObject

/** 
    Initialize with an NSData object containing 32 bit pixels.
    
    This sets up the tile with an NSData object containing raw pixels.  The pixels are 32 bit RGBA even if you're targeting a smaller pixel format.
    
    @param data The NSData object containing 32 bit RGBA pixels.
    
    @param width The width of the raw image contained in the data object.
    
    @param height The height of the raw image contained in the data object.
 */
- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/**
 Initialize with an NSData object containing 32 bit pixels.
 
 This sets up the tile with an NSData object containing raw pixels.  The pixels are 32 bit RGBA even if you're targeting a smaller pixel format.
 
 @param data The NSData object containing 32 bit RGBA pixels.
 
 @param width The width of the raw image contained in the data object.
 
 @param height The height of the raw image contained in the data object.
 
 @param comp The number of components (1, 2 or 4)
 */
- (instancetype)initWithRawImage:(NSData *)data width:(int)width height:(int)height components:(int)comp viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/**
Initialize with an NSData object containing pixels of a given format.

This sets up the tile with an NSData object containing raw pixels.  The pixels are defined by the format.

@param data The NSData object containing pixels.
 
@param format The image format the data is already in.

@param width The width of the raw image contained in the data object.

@param height The height of the raw image contained in the data object.
*/
- (instancetype)initWithRawImage:(NSData *)data format:(MaplyQuadImageFormat)format width:(int)width height:(int)height viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/** 
    Initialize with a single UIImage for the tile.
    
    This sets up the given UIImage as the return for the given tile.  You can then set targetSize and such.
 */
- (instancetype)initWithImage:(UIImage *)image viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/** 
    Initialize with an NSData object containing PNG or JPEG data that can be interpreted by UIImage.
    
    We're expecting PNG, JPEG or another self identified format (e.g. PKM).  These we can interpret ourselves.
 */
- (instancetype)initWithPNGorJPEGData:(NSData *)data viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC;

/**
    Border size that was set on initialization.
 
    If there's a built in border as part of the image data passed in during initialization, set it here.
    Normally this is 0.
  */
@property (nonatomic,assign) int borderSize;

/**
 Target size for the image(s) represented by this tile.
 
 This instructs the pager to rescale the image(s) to the given target size.  This is probably faster than doing it yourself because we can extract the data and rescale in the same step.
 */
@property (nonatomic) CGSize targetSize;

/**
    Preprocess into a simple texture format.
 
    Extracting from PNG or JPEG or whatever often requires a bit of work.  We'll do that work later,
    if this isn't called.  But if you do call it here then you can do that work on your own thread.
  */
- (void)preprocessTexture;

@end
