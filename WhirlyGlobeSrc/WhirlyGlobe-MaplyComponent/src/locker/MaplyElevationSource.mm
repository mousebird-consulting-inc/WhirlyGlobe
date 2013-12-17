/*
 *  MaplyElevationSource.m
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 8/29/13.
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

#import "MaplyElevationSource.h"
#import "MaplyElevationSource_private.h"

@implementation MaplyElevationChunk

- (id)initWithData:(NSData *)data numX:(unsigned int)numX numY:(unsigned int)numY
{
    self = [super init];
    if (!self)
        return nil;
    
    _numX = numX;
    _numY = numY;
    _data = data;
    
    return self;
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

- (bool)tileIsLocal:(MaplyTileID)tileID
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
    MaplyElevationChunk *elevChunk = [[MaplyElevationChunk alloc] initWithData:data numX:(numX+1) numY:(numY+1)];
    
    return elevChunk;
}

@end

@implementation MaplyElevationSourceAdapter
{
    NSObject<MaplyElevationSourceDelegate> *elevSource;
}

- (id)initWithElevationSource:(NSObject<MaplyElevationSourceDelegate> *)inElevSource
{
    self = [super init];
    if (!self)
        return nil;
    elevSource = inElevSource;
    
    return self;
}

- (WhirlyKitElevationChunk *)elevForLevel:(int)level col:(int)col row:(int)row
{
    MaplyTileID tileID;
    tileID.x = col;    tileID.y = row;    tileID.level = level;
    MaplyElevationChunk *maplyChunk = [elevSource elevForTile:tileID];
    if (maplyChunk)
    {
        WhirlyKitElevationChunk *wkChunk = [[WhirlyKitElevationChunk alloc] initWithFloatData:maplyChunk.data sizeX:maplyChunk.numX sizeY:maplyChunk.numY];
        return wkChunk;
    }
    return nil;
}

@end
