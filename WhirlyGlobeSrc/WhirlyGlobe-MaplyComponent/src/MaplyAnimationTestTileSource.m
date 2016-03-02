/*
 *  MaplyAnimationTestTileSource.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/4/13.
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

#import "MaplyAnimationTestTileSource.h"
#import "MaplyQuadImageTilesLayer.h"

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
    _useDelay = true;
    
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

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return true;
}

static const int MaxDebugColors = 10;
static const int debugColors[MaxDebugColors] = {0x86812D, 0x5EB9C9, 0x2A7E3E, 0x4F256F, 0xD89CDE, 0x773B28, 0x333D99, 0x862D52, 0xC2C653, 0xB8583D};

// Make sure this tile exists in the real world
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    MaplyCoordinate ll,ur;
    [_coordSys getBoundsLL:&ll ur:&ur];
    
    int numTiles = 1<<tileID.level;
    double tileSizeX = (bbox->ur.x-bbox->ll.x)/numTiles;
    double tileSizeY = (bbox->ur.y-bbox->ll.y)/numTiles;
    double midX = tileSizeX*(tileID.x+0.5) + bbox->ll.x;
    double midY = tileSizeY*(tileID.y+0.5) + bbox->ll.y;
    
    if (midX < ll.x || midX > ur.x)
        return false;
    if (midY < ll.y || midY > ur.y)
        return false;
    
    return true;
}

- (NSData *)imgDataForTile:(MaplyTileID)tileID frame:(int)frame
{
    CGSize size;  size = CGSizeMake(128,128);
    UIGraphicsBeginImageContext(size);
    
    // Draw into the image context
    int hexColor = debugColors[tileID.level % MaxDebugColors];
    float red = (((hexColor) >> 16) & 0xFF)/255.0;
    float green = (((hexColor) >> 8) & 0xFF)/255.0;
    float blue = (((hexColor) >> 0) & 0xFF)/255.0;
    UIColor *backColor = nil;
    UIColor *fillColor = [UIColor whiteColor];
    if (_transparentMode)
    {
        backColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.5];
        fillColor = [UIColor colorWithRed:red green:green blue:blue alpha:0.5];
    } else {
        backColor = [UIColor colorWithRed:0.9 green:0.9 blue:0.9 alpha:1.0];
        fillColor = [UIColor colorWithRed:red green:green blue:blue alpha:1.0];
    }
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    // Draw a rectangle around the edges for testing
    [backColor setFill];
    if (_transparentMode)
    {
        CGContextFillRect(ctx, CGRectMake(0, 0, size.width, size.height));
        [fillColor setStroke];
        CGContextStrokeRect(ctx, CGRectMake(0, 0, size.width-1, size.height-1));
    } else {
        CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
    }
    
    [fillColor setStroke];
    [fillColor setFill];
    CGContextSetTextDrawingMode(ctx, kCGTextFill);
    NSString *textStr = nil;
    if (_depth == 1)
        textStr = [NSString stringWithFormat:@"%d: (%d,%d)",tileID.level,tileID.x,tileID.y];
    else
        textStr = [NSString stringWithFormat:@"%d: (%d,%d); %d",tileID.level,tileID.x,tileID.y,frame];
    [textStr drawInRect:CGRectMake(0,0,size.width,size.height) withFont:[UIFont systemFontOfSize:24.0]];
    
    // Grab the image and shut things down
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    NSData *imgData = UIImagePNGRepresentation(retImage);
    UIGraphicsEndImageContext();

    return imgData;
}

static const float MaxDelay = 1.0;

- (void)startFetchLayer:(MaplyQuadImageTilesLayer *)layer tile:(MaplyTileID)tileID
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       if (_useDelay)
                       {
                           // Random delay
                           usleep(drand48()* MaxDelay * 1e6);
                       }

                       NSMutableArray *images = [NSMutableArray array];
                       for (unsigned int ii=0;ii<layer.imageDepth;ii++)
                       {
                           NSData *imgData = [self imgDataForTile:tileID frame:ii];
                           [images addObject:imgData];
                       }
                       
                       [layer loadedImages:[[MaplyImageTile alloc] initWithPNGorJPEGDataArray:images] forTile:tileID];
                   }
                   );
}

- (void)startFetchLayer:(MaplyQuadImageTilesLayer *)layer tile:(MaplyTileID)tileID frame:(int)frame
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       if (_useDelay)
                       {
                           // Random delay
                           usleep(drand48()* MaxDelay * 1e6);
                       }

                       NSData *imgData = [self imgDataForTile:tileID frame:frame];
                       
                       [layer loadedImages:[[MaplyImageTile alloc] initWithPNGorJPEGData:imgData] forTile:tileID frame:frame];
                   }
                   );
}

@end
