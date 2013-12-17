/*
 *  MaplyAnimationTestTileSource.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
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

#import "MaplyAnimationTestTileSource.h"

@implementation MaplyAnimationTestTileSource
{
    int _minZoom,_maxZoom,_depth;
}

- (id)initWithCoordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom depth:(int)depth
{
    self = [super init];
    if (!self)
        return nil;
    
    _coordSys = coordSys;
    _minZoom = minZoom;
    _maxZoom = maxZoom;
    _depth = depth;
    _pixelsPerSide = 256;
    
    return self;
}

- (int)minZoom
{
    return _minZoom;
}

- (int)maxZoom
{
    return _maxZoom;
}

- (int)tileSize
{
    return _pixelsPerSide;
}

- (bool)tileIsLocal:(MaplyTileID)tileID
{
    return true;
}

static const int MaxDebugColors = 10;
static const int debugColors[MaxDebugColors] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

- (MaplyImageTile *)imageForTile:(MaplyTileID)tileID
{
    NSMutableArray *images = [NSMutableArray array];
    
    // One for each layer we're
    for (unsigned int ii=0;ii<_depth;ii++)
    {
        CGSize size;  size = CGSizeMake(128,128);
        UIGraphicsBeginImageContext(size);
        
        // Draw into the image context
        int hexColor = debugColors[tileID.level % MaxDebugColors];
        float red = (((hexColor) >> 16) & 0xFF)/255.0;
        float green = (((hexColor) >> 8) & 0xFF)/255.0;
        float blue = (((hexColor) >> 0) & 0xFF)/255.0;
        UIColor *backColor = [UIColor colorWithRed:red green:green blue:blue alpha:1.0];
        [backColor setFill];
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
        
        CGContextSetTextDrawingMode(ctx, kCGTextFill);
        [[UIColor whiteColor] setStroke];
        [[UIColor whiteColor] setFill];
        NSString *textStr = nil;
        if (_depth == 1)
            textStr = [NSString stringWithFormat:@"%d: (%d,%d)",tileID.level,tileID.x,tileID.y];
        else
            textStr = [NSString stringWithFormat:@"image %d",ii];
        [textStr drawInRect:CGRectMake(0,0,size.width,size.height) withFont:[UIFont systemFontOfSize:24.0]];
        
        // Grab the image and shut things down
        UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
        NSData *imgData = UIImagePNGRepresentation(retImage);
        UIGraphicsEndImageContext();
        [images addObject:imgData];
    }
    
    return [[MaplyImageTile alloc] initWithPNGorJPEGDataArray:images];
}

@end
