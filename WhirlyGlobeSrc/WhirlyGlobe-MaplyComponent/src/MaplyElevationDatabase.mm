/*
 *  MaplyElevationDatabase.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/7/13.
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
#import "MaplyElevationDatabase.h"
#import "NSData+Zlib.h"
#import "FMDatabase.h"
#import "FMDatabaseQueue.h"

@implementation MaplyElevationDatabase
{
    FMDatabase *db;
    FMDatabaseQueue *queue;
    int _minZoom,_maxZoom;
}

- (id)initWithName:(NSString *)name
{
    self = [super init];
    if (!self)
        return nil;
    
    NSString *infoPath = nil;
    // See if that was a direct path first
    if ([[NSFileManager defaultManager] fileExistsAtPath:name])
        infoPath = name;
    else {
        // Now try looking for it in the bundle
        infoPath = [[NSBundle mainBundle] pathForResource:name ofType:@"sqlite"];
        if (!infoPath)
            return nil;
    }
    
    db = [[FMDatabase alloc] initWithPath:infoPath];
    if (!db)
        return nil;
    
    [db openWithFlags:SQLITE_OPEN_READONLY];
    
    FMResultSet *res = [db executeQuery:@"SELECT minx,miny,maxx,maxy,tilesizex,tilesizey,minlevel,maxlevel FROM manifest"];
    if ([res next])
    {
        _minX = [res doubleForColumn:@"minx"];
        _minY = [res doubleForColumn:@"miny"];
        _maxX = [res doubleForColumn:@"maxx"];
        _maxY = [res doubleForColumn:@"maxy"];
        _tileSizeX = [res intForColumn:@"tilesizex"];
        _tileSizeY = [res intForColumn:@"tilesizey"];
        _minZoom = [res intForColumn:@"minlevel"];
        _maxZoom = [res intForColumn:@"maxlevel"];
    }
    
    queue = [FMDatabaseQueue databaseQueueWithPath:infoPath];

    return self;
}

- (void)dealloc
{
    [db close];
}

- (MaplyCoordinateSystem *)getCoordSystem
{
    // Note: Should be in the database
    return [[MaplySphericalMercator alloc] initWebStandard];
}

- (int)minZoom
{
    return _minZoom;
}

// Note: Need to put this in the database
- (int)maxZoom
{
    return _maxZoom;
}

- (bool)tileIsLocal:(MaplyTileID)tileID
{
    return true;
}

- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID
{
    // Put together the precalculated quad index.  This is faster
    //  than x,y,level
    int quadIdx = 0;
    for (int iq=0;iq<tileID.level;iq++)
        quadIdx += (1<<iq)*(1<<iq);
    quadIdx += tileID.y*(1<<tileID.level)+tileID.x;

    NSData * __block uncompressedData=nil;
    bool __block tilePresent = false;
    // Note: Need to sort this out
    [queue inDatabase:^(FMDatabase *theDb) {
        // Now look for the tile
        FMResultSet *res = [theDb executeQuery:[NSString stringWithFormat:@"SELECT data FROM elevationtiles WHERE quadindex=%d;",quadIdx]];
        NSData *data = nil;
        if ([res next])
        {
            tilePresent = true;
            data = [res dataForColumn:@"data"];
        }
        if (data && [data length] > 0)
            uncompressedData = [data uncompressGZip];
        [res close];
    }];
    
    if (!uncompressedData || [uncompressedData length] == 0)
    {
        if (tilePresent)
        {
            // Return a tile with all zeros
            // Note: This could be optimized
            float *floats = (float *)malloc(sizeof(float)*_tileSizeX*_tileSizeY);
            memset(floats, 0, sizeof(float)*_tileSizeX*_tileSizeY);
            NSData *floatData = [NSData dataWithBytesNoCopy:floats length:_tileSizeX*_tileSizeY*sizeof(float) freeWhenDone:YES];
            MaplyElevationChunk *chunk = [[MaplyElevationChunk alloc] initWithData:floatData numX:_tileSizeX numY:_tileSizeY];
            return chunk;
        } else
            return nil;
    }
    
//    NSLog(@"Loading tile: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
    float *floats = (float *)malloc(sizeof(float)*_tileSizeX*_tileSizeY);
    for (unsigned int ii=0;ii<_tileSizeX*_tileSizeY;ii++)
        floats[ii] = ((short *)[uncompressedData bytes])[ii];
    NSData *floatData = [NSData dataWithBytesNoCopy:floats length:_tileSizeX*_tileSizeY*sizeof(float) freeWhenDone:YES];
    MaplyElevationChunk *chunk = [[MaplyElevationChunk alloc] initWithData:floatData numX:_tileSizeX numY:_tileSizeY];

    return chunk;
}

@end
