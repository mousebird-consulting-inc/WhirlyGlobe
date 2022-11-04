/*  MaplyMBTileFetcher.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2011-2022 mousebird consulting inc
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
 */

#import "data_sources/MaplyMBTileFetcher.h"
#import "MaplyCoordinateSystem_private.h"
#import "WhirlyGlobeLib.h"
#import "sqlhelpers.h"

using namespace WhirlyKit;

@implementation MaplyMBTileFetcher
{
    bool tilesStyles;
    int minZoom,maxZoom;
    Mbr mbr;
    GeoMbr geoMbr;
    sqlite3 *sqlDb;
    MaplyCoordinateSystem *coordSys;
}

- (nullable instancetype)initWithMBTiles:(NSString *__nonnull)mbTilesName
{
    return [self initWithMBTiles:mbTilesName cacheSize:-1];
}

- (nullable instancetype)initWithMBTiles:(NSString *__nonnull)mbTilesName
                               cacheSize:(int)cacheSize
{
    NSString *infoPath = nil;
    
    // fileExistsAtPath can't handle file URLs
    if (NSURL* url = [NSURL URLWithString:mbTilesName])
    {
        mbTilesName = url.path;
    }

    // See if that was a direct path first
    if ([[NSFileManager defaultManager] fileExistsAtPath:mbTilesName])
        infoPath = mbTilesName;
    else {
        // Try the documents directory
        NSString *docDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        infoPath = [NSString stringWithFormat:@"%@/%@",docDir,mbTilesName];
        if (![[NSFileManager defaultManager] fileExistsAtPath:infoPath])
            infoPath = nil;
        if (!infoPath)
        {
            // Now try looking for it in the bundle
            infoPath = [[NSBundle mainBundle] pathForResource:mbTilesName ofType:@"mbtiles"];
            if (!infoPath)
            {
                infoPath = [[NSBundle mainBundle] pathForResource:mbTilesName ofType:@"sqlite"];
                if (!infoPath)
                    return nil;
            }
        }
    }

    const char* nameStr = [infoPath cStringUsingEncoding:NSASCIIStringEncoding];
    if (!nameStr || !nameStr[0])
    {
        return nil;
    }

    // Open the sqlite DB
    sqlite3 *db = nil;
    // Disable writes, eliminating the need for locking.
    const int flags = SQLITE_OPEN_READONLY |
                      SQLITE_OPEN_NOMUTEX;
                      //| SQLITE_OPEN_EXRESCODE;    // extended error codes on failure - available in later versions
    const int openRes = sqlite3_open_v2(nameStr, &db, flags, nullptr);
    if (openRes != SQLITE_OK)
    {
        const char* const err = sqlite3_errstr(openRes);
        wkLogLevel(Error, "SQLite failed to open '%s' - %d: %s", nameStr, openRes, err ? err : "?");
        return nil;
    }

    const auto cs = [[MaplySphericalMercator alloc] initWebStandard];

    if (cacheSize >= 0)
    {
        try
        {
            sqlhelpers::StatementRead pageStmt(db, "PRAGMA page_size", false);
            const int pageSize = pageStmt.stepRow() ? pageStmt.getInt() : 0;
            if (pageSize > 0)
            {
                const int cachePages = (cacheSize + pageSize - 1) / pageSize;
                const auto sql = "PRAGMA cache_size=" + std::to_string(cachePages);
                sqlhelpers::StatementRead configStmt(db, sql.c_str(), true);

                sqlhelpers::StatementRead cacheStmt(db, "PRAGMA cache_size", false);
                if (cacheStmt.stepRow())
                {
                    const int actualCachePages = cacheStmt.getInt();
                    wkLogLevel(Info, "SQLite cache size set to %d pages = %d bytes",
                               actualCachePages, actualCachePages * pageSize);
                }
            }
        }
        catch (const std::exception &e)
        {
            wkLogLevel(Warn, "Failed to set SQLite cache (%s)", e.what());
        }
        catch (int e)
        {
            const char* const str = sqlite3_errstr(e);
            wkLogLevel(Warn, "Failed to set SQLite cache (%d): %s", e, str ? str : "?");
        }
        catch (...)
        {
            const int e = sqlite3_errcode(db);
            const char* const str = sqlite3_errmsg(db);
            wkLogLevel(Warn, "Failed to set SQLite cache (%d): %s", e, str ? str : "?");
        }
    }

    // Look at the metadata
    GeoMbr gmbr;
    bool failed = false;
    try
    {
        sqlhelpers::StatementRead readStmt(db,@"select value from metadata where name='bounds';");
        if (readStmt.stepRow())
        {
            NSString *bounds = readStmt.getString();
            NSScanner *scan = [NSScanner scannerWithString:bounds];
            NSMutableCharacterSet *charSet = [[NSMutableCharacterSet alloc] init];
            [charSet addCharactersInString:@", "];
            [scan setCharactersToBeSkipped:charSet];
            double ll_lat,ll_lon,ur_lat,ur_lon;
            if (![scan scanDouble:&ll_lon] ||
                ![scan scanDouble:&ll_lat] ||
                ![scan scanDouble:&ur_lon] ||
                ![scan scanDouble:&ur_lat])
            {
                return nil;
            }
            gmbr.ll() = GeoCoord::CoordFromDegrees(ll_lon,ll_lat);
            gmbr.ur() = GeoCoord::CoordFromDegrees(ur_lon,ur_lat);
        } else {
            // No bounds implies it covers the whole earth
            gmbr.ll() = GeoCoord::CoordFromDegrees(-180, -85.0511);
            gmbr.ur() = GeoCoord::CoordFromDegrees(180, 85.0511);
        }
        
        // And let's convert that over to spherical mercator
        const Point3f ll = [cs getCoordSystem]->geographicToLocal(gmbr.ll());
        mbr.ll() = Point2f(ll.x(),ll.y());
        const Point3f ur = [cs getCoordSystem]->geographicToLocal(gmbr.ur());
        mbr.ur() = Point2f(ur.x(),ur.y());

        minZoom = 0;  maxZoom = 8;
        sqlhelpers::StatementRead readStmt2(db,@"select value from metadata where name='minzoom';");
        if (readStmt2.stepRow())
            minZoom = [readStmt2.getString() intValue];
        else {
            // Read it the hard way
            sqlhelpers::StatementRead readStmt3(db,@"select min(zoom_level) from tiles;");
            if (readStmt3.stepRow())
                minZoom = [readStmt3.getString() intValue];
        }
        sqlhelpers::StatementRead readStmt3(db,@"select value from metadata where name='maxzoom';");
        if (readStmt3.stepRow())
            maxZoom = [readStmt3.getString() intValue];
        else {
            // Read it the hard way
            sqlhelpers::StatementRead readStmt3(db,@"select max(zoom_level) from tiles;");
            if (readStmt3.stepRow())
                maxZoom = [readStmt3.getString() intValue];
        }
        
        sqlhelpers::StatementRead readStmt4(db,"select value from metadata where name='format';");
        if (readStmt4.stepRow())
            _format = readStmt4.getString();
        
        // See if there's a tiles table or it's the older(?) style
        sqlhelpers::StatementRead testStmt(db,@"SELECT name FROM sqlite_master WHERE type='table' AND name='tiles';");
        if (testStmt.stepRow())
            tilesStyles = true;
    }
    catch (const std::exception &e)
    {
        wkLogLevel(Error, "Exception fetching MBTiles metadata (%s)", e.what());
        failed = true;
    }
    catch (int e)
    {
        const char* const str = sqlite3_errstr(e);
        wkLogLevel(Error, "Exception fetching MBTiles metadata (%d): %s", e, str ? str : "?");
        failed = true;
    }
    catch (...)
    {
        const int e = sqlite3_errcode(db);
        const char* const str = sqlite3_errmsg(db);
        wkLogLevel(Error, "Exception fetching MBTiles metadata (%d): %s", e, str ? str : "?");
        failed = true;
    }

    if (failed)
    {
        if (db)
        {
            sqlite3_close(db);
        }
        return nil;
    }

    if ((self = [super initWithName:mbTilesName minZoom:minZoom maxZoom:maxZoom]))
    {
        geoMbr = gmbr;
        coordSys = cs;
        sqlDb = db;
    }
    
    return self;
}

- (MaplyCoordinateSystem *)coordSys
{
    return coordSys;
}

- (id)dataForTile:(id)fetchInfo tileID:(MaplyTileID)tileID;
{
    NSData *imageData = nil;
    
    @synchronized(self)
    {
        try {
            if (tilesStyles)
            {
                sqlhelpers::StatementRead readStmt(sqlDb,[NSString stringWithFormat:@"SELECT tile_data from tiles where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",tileID.level,tileID.x,tileID.y]);
                if (readStmt.stepRow())
                    imageData = readStmt.getBlob();
            } else {
                sqlhelpers::StatementRead readStmt(sqlDb,[NSString stringWithFormat:@"SELECT tile_id from map where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",tileID.level,tileID.x,tileID.y]);
                if (readStmt.stepRow())
                {
                    NSString *tile_id = readStmt.getString();
                    sqlhelpers::StatementRead readStmt2(sqlDb,[NSString stringWithFormat:@"SELECT tile_data from images where tile_id='%@';",tile_id]);
                    if (readStmt2.stepRow())
                        imageData = readStmt2.getBlob();
                }
            }
        } catch (int e) {
            NSLog(@"Exception in [MaplyMBTileSouce imageForTile:]");
        }
    }
    
    return imageData;
}

- (void)shutdown
{
    [super shutdown];
    
    if (sqlDb) {
        sqlite3_close(sqlDb);
        sqlDb = nullptr;
    }
}

@end
