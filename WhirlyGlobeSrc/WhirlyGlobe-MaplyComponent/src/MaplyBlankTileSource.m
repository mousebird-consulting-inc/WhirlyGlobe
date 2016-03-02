/*
 *  MaplyBlankTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/15/15.
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

#import "MaplyBlankTileSource.h"

@implementation MaplyBlankTileSource
{
    int _minZoom,_maxZoom,_depth;
}

- (id)initWithCoordSys:(MaplyCoordinateSystem *)coordSys minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    _coordSys = coordSys;
    _minZoom = minZoom;
    _maxZoom = maxZoom;
    _color = [UIColor whiteColor];
    _pixelsPerSide = 128;
    
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

- (MaplyImageTile *)imageForTile:(MaplyTileID)tileID
{
    return [self imageForTile:tileID frame:-1];
}

// Make sure this tile exists in the real world
- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    int numTiles = 1<<tileID.level;
    double tileSizeX = (bbox->ur.x-bbox->ll.x)/numTiles;
    double tileSizeY = (bbox->ur.y-bbox->ll.y)/numTiles;
    double midX = tileSizeX*(tileID.x+0.5) + bbox->ll.x;
    double midY = tileSizeY*(tileID.y+0.5) + bbox->ll.y;
    
    if (midX < -M_PI || midX > M_PI)
        return false;
    if (midY < -M_PI/2.0 || midY > M_PI/2.0)
        return false;
    
    return true;
}

- (MaplyImageTile *)imageForTile:(MaplyTileID)tileID frame:(int)frame
{
    CGSize size;  size = CGSizeMake(_pixelsPerSide,_pixelsPerSide);
    UIGraphicsBeginImageContext(size);
    
    CGContextRef ctx = UIGraphicsGetCurrentContext();

        // Draw a rectangle around the edges for testing
    [_color setFill];
    CGContextFillRect(ctx, CGRectMake(0,0,size.width,size.height));
    
    UIImage *retImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return [[MaplyImageTile alloc] initWithImage:retImage];
}

@end
