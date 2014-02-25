/*
 *  MaplyImageTile_private.h
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

#import "MaplyImageTile.h"
#import "WhirlyGlobe.h"

typedef enum {MaplyImgTypeImage,MaplyImgTypeData,MaplyImgTypeRawImage,MaplyImgTypePlaceholder} MaplyImgType;

@interface MaplyImageTile()

// Convert to raw data internally.  This may require some decompression, so we pull it (optionally) ouot
- (void)convertToRaw:(int)borderTexel destWidth:(int)inDestWidth destHeight:(int)inDestHeight;

// Build a WhirlyKit texture for the given layer of image data
- (WhirlyKit::Texture *)buildTextureFor:(int)which border:(int)reqBorderTexel destWidth:(int)destWidth destHeight:(int)destHeight;

// Internal type
@property (nonatomic) MaplyImgType type;

@end
