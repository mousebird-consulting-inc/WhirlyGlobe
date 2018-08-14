/*
 *  LoadedTile.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/19/13.
 *  Copyright 2011-2017 mousebird consulting
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

#import <Foundation/Foundation.h>
#import <math.h>
#import "WhirlyVector.h"
#import "Scene.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "GlobeMath.h"
#import "sqlhelpers.h"
#import "QuadTreeNew.h"
#import "SceneRendererES.h"
#import "QuadDisplayLayerNew.h"
#import "TextureAtlas.h"
#import "ElevationChunk.h"
#import "DynamicTextureAtlas.h"

/** Type of the image being passed to the tile loader.
 UIImage - A UIImage object.
 NSDataAsImage - An NSData object containing PNG or JPEG data.
 WKLoadedImageNSDataRawData - An NSData object containing raw RGBA values.
 PVRTC4 - Compressed PVRTC, 4 bit, no alpha
 PKM - ETC2 and EAC textures
 Placeholder - This is an empty image (so no visual representation)
 that is nonetheless "valid" so its children will be paged.
 */
typedef enum {WKLoadedImageUIImage,WKLoadedImageNSDataAsImage,WKLoadedImageNSDataRawData,WKLoadedImagePVRTC4,WKLoadedImageNSDataPKM,WKLoadedImagePlaceholder,WKLoadedImageMax} WhirlyKitLoadedImageType;

/// Used to specify the image type for the textures we create
typedef enum {WKTileIntRGBA,
    WKTileUShort565,
    WKTileUShort4444,
    WKTileUShort5551,
    WKTileUByteRed,WKTileUByteGreen,WKTileUByteBlue,WKTileUByteAlpha,
    WKTileUByteRGB,
    WKTilePVRTC4,
    WKTileETC2_RGB8,WKTileETC2_RGBA8,WKTileETC2_RGB8_PunchAlpha,
    WKTileEAC_R11,WKTileEAC_R11_Signed,WKTileEAC_RG11,WKTileEAC_RG11_Signed,
} WhirlyKitTileImageType;

/// How we'll scale the tiles up or down to the nearest power of 2 (square) or not at all
typedef enum {WKTileScaleUp,WKTileScaleDown,WKTileScaleFixed,WKTileScaleNone} WhirlyKitTileScaleType;


/** The Loaded Image is handed back to the Tile Loader when an image
 is finished.  It can either be loaded or empty, or something of that sort.
 */
@interface WhirlyKitLoadedImage : NSObject

/// The data we're passing back
@property (nonatomic,assign) WhirlyKitLoadedImageType type;
/// Set if there are any border pixels in the image
@property (nonatomic,assign) int borderSize;
/// The UIImage or NSData object
@property (nonatomic) NSObject *imageData;
/// Some formats contain no size info (e.g. PVRTC).  In which case, this is set
@property (nonatomic,assign) int width,height;

/// Return a loaded image made of a standard UIImage
+ (WhirlyKitLoadedImage *)LoadedImageWithUIImage:(UIImage *)image;

/// Return a loaded image made from an NSData object containing PVRTC
+ (WhirlyKitLoadedImage *)LoadedImageWithPVRTC:(NSData *)imageData size:(int)squareSize;

/// Return a loaded image that's just an empty placeholder.
/// This means there's nothing to display, but the children are valid
+ (WhirlyKitLoadedImage *)PlaceholderImage;

/// Return a loaded image made from an NSData object that contains a PNG or JPG.
/// Basically somethign that UIImage will recognize if you initialize it with one.
+ (WhirlyKitLoadedImage *)LoadedImageWithNSDataAsPNGorJPG:(NSData *)imageData;

/// Generate an appropriate texture.
/// You could overload this, just be sure to respect the border pixels.
- (WhirlyKit::Texture *)buildTexture:(int)borderSize destWidth:(int)width destHeight:(int)height;

/// This will extract the pixels out of an image or NSData and store them for later use
- (bool)convertToRawData:(int)borderTexel;

@end

/** This is a more generic version of the Loaded Image.  It can be a single
 loaded image, a stack of them (for animation) and/or a terrain chunk.
 If you're doing a stack of images, make sure you set up the tile quad loader
 that way.
 */
@interface WhirlyKitLoadedTile : NSObject

@property (nonatomic,readonly) NSMutableArray *images;
@property (nonatomic) NSObject<WhirlyKitElevationChunk> *elevChunk;

@end

/** This protocol is used by the data sources to optionally tack some elevation on to a tile
 fetch.  Elevation often comes from a different source and we want to be able to reuse
 our generic image tile fetchers.
 */
@protocol WhirlyKitElevationHelper
/// Return the elevation data for the given tile or nil if there is none
- (NSObject<WhirlyKitElevationChunk> *)elevForLevel:(int)level col:(int)col row:(int)row;
@end
