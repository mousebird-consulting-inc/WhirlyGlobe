/*
 *  MBTileQuadSource.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/23/12.
 *  Copyright 2011-2012 mousebird consulting
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

#import "MBTileQuadSource.h"
#import "GlobeLayerViewWatcher.h"

using namespace WhirlyKit;

@implementation WhirlyKitMBTileQuadSource
{
    bool tilesStyles;
}

- (id)initWithPath:(NSString *)path
{
    self = [super init];
    if (self)
    {
        coordSys = new SphericalMercatorCoordSystem();
        
        // Open the sqlite DB
        if (sqlite3_open([path cStringUsingEncoding:NSASCIIStringEncoding],&sqlDb) != SQLITE_OK)
        {
            return nil;
        }
        
        // Look at the metadata
        sqlhelpers::StatementRead readStmt(sqlDb,@"select value from metadata where name='bounds';");
        if (!readStmt.stepRow())
        {
            return nil;
        }
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
        geoMbr.ll() = GeoCoord::CoordFromDegrees(ll_lon,ll_lat);
        geoMbr.ur() = GeoCoord::CoordFromDegrees(ur_lon,ur_lat);
        
        // And let's convert that over to spherical mercator
        Point3f ll = coordSys->geographicToLocal(geoMbr.ll());
        mbr.ll() = Point2f(ll.x(),ll.y());
        Point3f ur = coordSys->geographicToLocal(geoMbr.ur());
        mbr.ur() = Point2f(ur.x(),ur.y());
        
        minZoom = 0;  maxZoom = 8;
        sqlhelpers::StatementRead readStmt2(sqlDb,@"select value from metadata where name='minzoom';");
        if (readStmt2.stepRow())
            minZoom = [readStmt2.getString() intValue];
        sqlhelpers::StatementRead readStmt3(sqlDb,@"select value from metadata where name='maxzoom';");
        if (readStmt3.stepRow())
            maxZoom = [readStmt3.getString() intValue];
                
        // Note: We could load something and calculate this, but I don't want to slow us down here
        pixelsPerTile = 256;
        
        // See if there's a tiles table or it's the older(?) style
        sqlhelpers::StatementRead testStmt(sqlDb,@"SELECT name FROM sqlite_master WHERE type='table' AND name='tiles';");
        if (testStmt.stepRow())
            tilesStyles = true;
    }
    
    return self;
}

- (void)dealloc
{
    if (coordSys)
        delete coordSys;
    coordSys = nil;
    
    if (sqlDb)
        sqlite3_close(sqlDb);        
}

- (void)shutdown
{
    // Nothing much to do here
}

- (WhirlyKit::CoordSystem *)coordSystem
{
    return coordSys;
}

- (WhirlyKit::Mbr)totalExtents
{
    return mbr;
}

- (WhirlyKit::Mbr)validExtents
{
    return mbr;
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)tileMbr viewInfo:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize
{
    // Everything at the top is loaded in, so be careful
    if (ident.level == minZoom)
        return MAXFLOAT;

    float import = ScreenImportance(viewState, frameSize, viewState->eyeVec, pixelsPerTile, coordSys, viewState->coordAdapter, tileMbr);
//    if (import != 0.0)
//        NSLog(@"tile = (%d,%d,%d), import = %f",ident.x,ident.y,ident.level,import);
    return import;
}

// Just one fetch at a time
- (int)maxSimultaneousFetches
{
    return 1;
}

// Load the given tile.  We'll do that right here
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row
{
    NSData *imageData = nil;
    
    if (tilesStyles)
    {
        sqlhelpers::StatementRead readStmt(sqlDb,[NSString stringWithFormat:@"SELECT tile_data from tiles where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",level,col,row]);
        if (readStmt.stepRow())
            imageData = readStmt.getBlob();
    } else {
        sqlhelpers::StatementRead readStmt(sqlDb,[NSString stringWithFormat:@"SELECT tile_id from map where zoom_level='%d' AND tile_column='%d' AND tile_row='%d';",level,col,row]);
        if (readStmt.stepRow())
        {
            NSString *tile_id = readStmt.getString();
            sqlhelpers::StatementRead readStmt2(sqlDb,[NSString stringWithFormat:@"SELECT tile_data from images where tile_id='%@';",tile_id]);
            if (readStmt2.stepRow())
                imageData = readStmt2.getBlob();
        }
    }
    
//    if (!imageData)
//        NSLog(@"Missing tile: (%d,%d,%d)",col,row,level);
    
    // Tell the quad loader about the new tile data, whether its null or not
    [quadLoader dataSource:self loadedImage:imageData pvrtcSize:0 forLevel:level col:col row:row];
}


@end
