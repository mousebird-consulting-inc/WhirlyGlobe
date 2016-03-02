/*
 *  MaplyElevationSource.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/29/13.
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

#import "MaplyElevationSource.h"
#import "MaplyElevationSource_private.h"
#import "ElevationChunk.h"
#import "ElevationCesiumChunk.h"

@implementation MaplyElevationChunk

@end

@implementation MaplyElevationGridChunk

- (id)initWithGridData:(NSData *)data sizeX:(unsigned int)sizeX sizeY:(unsigned int)sizeY
{
    if (self = [super init])
    {
        if (sizeX*sizeY*2 == [data length])
            self.chunkImpl = [[WhirlyKitElevationGridChunk alloc] initWithShortData:data sizeX:sizeX sizeY:sizeY];
        else
            self.chunkImpl = [[WhirlyKitElevationGridChunk alloc] initWithFloatData:data sizeX:sizeX sizeY:sizeY];
    }
    
    return self;
}

@end

@implementation MaplyElevationCesiumChunk

- (id)initWithCesiumData:(NSData *)data sizeX:(unsigned int)sizeX sizeY:(unsigned int)sizeY
{
    if (self = [super init])
        self.chunkImpl = [[WhirlyKitElevationCesiumChunk alloc] initWithCesiumData:data sizeX:sizeX sizeY:sizeY];
    
    return self;
}

- (void)setScale:(float)scale
{
    _scale = scale;
    if ([self.chunkImpl respondsToSelector:@selector(setScale:)])
        [(WhirlyKitElevationCesiumChunk *)self.chunkImpl setScale:scale];
}

@end

@implementation MaplyElevationSourceTester
{
    MaplySphericalMercator *coordSys;
}

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    
    coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return self;
}

/// Coordinate system we're providing the data in
- (MaplyCoordinateSystem *)getCoordSystem
{
    return coordSys;
}

- (int)minZoom
{
    return 0;
}

// We'll go as low as they want since we're making it up
- (int)maxZoom
{
    return 30;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return true;
}

static const float MaxElev = 13420;
static const float ScaleFactor = 300;

/// Return an elevation chunk (or nil) for a given tile
- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID
{
    int numX = 10,numY = 10;

    // Figure out the tile extents in the local coordinate system
    MaplyCoordinate ll,ur;
    [coordSys getBoundsLL:&ll ur:&ur];
    MaplyCoordinate tileLL,tileUR;
    float dx = (ur.x-ll.x)/(1<<tileID.level);
    float dy = (ur.y-ll.y)/(1<<tileID.level);
    tileLL.x = dx * tileID.x + ll.x;
    tileLL.y = dy * tileID.y + ll.y;
    tileUR.x = dx * (tileID.x + 1) + ll.x;
    tileUR.y = dy * (tileID.y + 1) + ll.y;
    float cellX = dx / numX;
    float cellY = dy / numY;

    // Now work through the individual cells
    float *floatData = (float *)malloc(sizeof(float)*(numX+1)*(numY+1));
    for (unsigned int iy=0;iy<=numY;iy++)
        for (unsigned int ix=0;ix<=numX;ix++)
        {
            MaplyCoordinate coord;
            coord.x = tileLL.x + ix * cellX;
            coord.y = tileLL.y + iy * cellY;
            float elev = (1.0+sinf(coord.x*ScaleFactor)) * (1.0+ sinf(coord.y*ScaleFactor)) * MaxElev/2.0;
            floatData[iy*(numX+1)+ix] = elev;
        }
    
    NSData *data = [[NSData alloc] initWithBytesNoCopy:floatData length:sizeof(float)*(numX+1)*(numY+1) freeWhenDone:YES];
    MaplyElevationChunk *elevChunk = [[MaplyElevationGridChunk alloc] initWithGridData:data sizeX:(numX+1) sizeY:(numY+1)];
    
    return elevChunk;
}

@end
