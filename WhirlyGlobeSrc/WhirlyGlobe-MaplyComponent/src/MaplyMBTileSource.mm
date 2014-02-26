/*
 *  MaplyMBTileSource.m
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

#import "MaplyMBTileSource.h"
#import "MaplyCoordinateSystem.h"
#import "WhirlyGlobe.h"
#import "MaplyCoordinateSystem_private.h"
#import "sqlhelpers.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyMBTileSource
{
    bool tilesStyles;
    int _minZoom,_maxZoom;
    int _pixelsPerTile;
    WhirlyKit::Mbr _mbr;
    WhirlyKit::GeoMbr _geoMbr;
    sqlite3 *_sqlDb;
}

- (id)initWithMBTiles:(NSString *)mbTilesName
{
    self = [super init];
    if (!self)
        return nil;
    
    NSString *infoPath = nil;
    // See if that was a direct path first
    if ([[NSFileManager defaultManager] fileExistsAtPath:mbTilesName])
        infoPath = mbTilesName;
    else {
        // Now try looking for it in the bundle
        infoPath = [[NSBundle mainBundle] pathForResource:mbTilesName ofType:@"mbtiles"];
        if (!infoPath)
            return nil;
    }
    
    // Open the sqlite DB
    if (sqlite3_open([infoPath cStringUsingEncoding:NSASCIIStringEncoding],&_sqlDb) != SQLITE_OK)
    {
        return nil;
    }
    
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    // Look at the metadata
    sqlhelpers::StatementRead readStmt(_sqlDb,@"select value from metadata where name='bounds';");
    if (readStmt.stepRow())
    {
        NSString *bounds = readStmt.getString();
        NSScanner *scan = [NSScanner scannerWithString:bounds];
        NSMutableCharacterSet *charSet = [[NSMutableCharacterSet alloc] init];
        [charSet addCharactersInString:@","];
        [scan setCharactersToBeSkipped:charSet];
        double ll_lat,ll_lon,ur_lat,ur_lon;
        if (![scan scanDouble:&ll_lon] ||
            ![scan scanDouble:&ll_lat] ||
            ![scan scanDouble:&ur_lon] ||
            ![scan scanDouble:&ur_lat])
        {
            return nil;
        }
        _geoMbr.ll() = GeoCoord::CoordFromDegrees(ll_lon,ll_lat);
        _geoMbr.ur() = GeoCoord::CoordFromDegrees(ur_lon,ur_lat);
    } else {
        // No bounds implies it covers the whole earth
        _geoMbr.ll() = GeoCoord::CoordFromDegrees(-180, -85.0511);
        _geoMbr.ur() = GeoCoord::CoordFromDegrees(180, 85.0511);
    }
    
    // And let's convert that over to spherical mercator
    Point3f ll = [_coordSys getCoordSystem]->geographicToLocal(_geoMbr.ll());
    _mbr.ll() = Point2f(ll.x(),ll.y());
    Point3f ur = [_coordSys getCoordSystem]->geographicToLocal(_geoMbr.ur());
    _mbr.ur() = Point2f(ur.x(),ur.y());
    
    _minZoom = 0;  _maxZoom = 8;
    sqlhelpers::StatementRead readStmt2(_sqlDb,@"select value from metadata where name='minzoom';");
    if (readStmt2.stepRow())
        _minZoom = [readStmt2.getString() intValue];
    else {
        // Read it the hard way
        sqlhelpers::StatementRead readStmt3(_sqlDb,@"select min(zoom_level) from tiles;");
        if (readStmt3.stepRow())
            _minZoom = [readStmt3.getString() intValue];
    }
    sqlhelpers::StatementRead readStmt3(_sqlDb,@"select value from metadata where name='maxzoom';");
    if (readStmt3.stepRow())
        _maxZoom = [readStmt3.getString() intValue];
    else {
        // Read it the hard way
        sqlhelpers::StatementRead readStmt3(_sqlDb,@"select max(zoom_level) from tiles;");
        if (readStmt3.stepRow())
            _maxZoom = [readStmt3.getString() intValue];
    }
    
    // Note: We could load something and calculate this, but I don't want to slow us down here
    _pixelsPerTile = 256;
    
    // See if there's a tiles table or it's the older(?) style
    sqlhelpers::StatementRead testStmt(_sqlDb,@"SELECT name FROM sqlite_master WHERE type='table' AND name='tiles';");
    if (testStmt.stepRow())
        tilesStyles = true;

    
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
    return _pixelsPerTile;
}

- (bool)tileIsLocal:(MaplyTileID)tileID
{
    return true;
}

- (id)imageForTile:(MaplyTileID)tileID
{
    NSData *imageData = nil;
    
    @synchronized(self)
    {
        if (tilesStyles)
        {
            sqlhelpers::StatementRead readStmt(_sqlDb,[NSString stringWithFormat:@"SELECT tile_data from tiles where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",tileID.level,tileID.x,tileID.y]);
            if (readStmt.stepRow())
                imageData = readStmt.getBlob();
        } else {
            sqlhelpers::StatementRead readStmt(_sqlDb,[NSString stringWithFormat:@"SELECT tile_id from map where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",tileID.level,tileID.x,tileID.y]);
            if (readStmt.stepRow())
            {
                NSString *tile_id = readStmt.getString();
                sqlhelpers::StatementRead readStmt2(_sqlDb,[NSString stringWithFormat:@"SELECT tile_data from images where tile_id='%@';",tile_id]);
                if (readStmt2.stepRow())
                    imageData = readStmt2.getBlob();
            }
        }
    }
    
    return imageData;
}

@end
