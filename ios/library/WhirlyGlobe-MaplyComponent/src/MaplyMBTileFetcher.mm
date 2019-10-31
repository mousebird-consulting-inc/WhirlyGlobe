/*
 *  MaplyMBTileFetcher.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 9/13/18.
 *  Copyright 2011-2018 mousebird consulting inc
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

#import "MaplyMBTileFetcher.h"
#import "MaplyCoordinateSystem_private.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

// Encapsulates a single tile load request
@interface MaplyMBTileFetchInfo : NSObject

@property (nonatomic,assign) int x;
@property (nonatomic,assign) int y;
@property (nonatomic,assign) int level;

@end

@implementation MaplyMBTileFetchInfo
@end

// Internal object used by the QuadImageLoader to generate tile load info
@interface MaplyMBTileInfo : NSObject<MaplyTileInfoNew>
@end

@implementation MaplyMBTileInfo
{
    int minZoom,maxZoom;
}

- (instancetype)initWithMinZoom:(int)inMinZoom maxZoom:(int)inMaxZoom
{
    self = [super init];
    minZoom = inMinZoom;
    maxZoom = inMaxZoom;
    
    return self;
}

- (id _Nullable)fetchInfoForTile:(MaplyTileID)tileID
{
    MaplyMBTileFetchInfo *fetchInfo = [[MaplyMBTileFetchInfo alloc] init];
    fetchInfo.x = tileID.x;
    fetchInfo.y = tileID.y;
    fetchInfo.level = tileID.level;
    
    return fetchInfo;
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

@end

// A single tile that we're aware of
class TileInfo
{
public:
    TileInfo() : priority(0), importance(0.0), request(nil), fetchInfo(nil) { }
    
    /// Comparison based on importance, tile source, then x,y,level
    bool operator < (const TileInfo &that) const
    {
        if (this->priority == that.priority) {
            if (this->importance == that.importance) {
                return this->request < that.request;
            }
            return this->importance < that.importance;
        }
        return this->priority > that.priority;
    }
    
    // Priority before importance
    int priority;
    
    // Importance of this tile request as passed in by the fetch request
    double importance;
    
    // The request as it came from outside the tile fetcher
    MaplyTileFetchRequest *request;
    
    // Specific fetchInfo from the fetch request.
    MaplyMBTileFetchInfo *fetchInfo;
};

typedef std::shared_ptr<TileInfo> TileInfoRef;
typedef struct {
    bool operator () (const TileInfoRef a,const TileInfoRef b) const {
        return *a < *b;
    }
} TileInfoSorter;
typedef std::set<TileInfoRef,TileInfoSorter> TileInfoSet;
typedef std::map<MaplyTileFetchRequest *,TileInfoRef> TileFetchMap;

@implementation MaplyMBTileFetcher
{
    bool active;
    bool loadScheduled;
    bool tilesStyles;
    NSString *name;
    int minZoom,maxZoom;
    Mbr mbr;
    GeoMbr geoMbr;
    sqlite3 *sqlDb;
    MaplyCoordinateSystem *coordSys;
    MaplyMBTileInfo *tileInfo;
    dispatch_queue_t queue;

    TileInfoSet toLoad;  // Tiles sorted by importance
    TileFetchMap tilesByFetchRequest;  // Tiles sorted by fetch request
}

- (nullable instancetype)initWithMBTiles:(NSString *__nonnull)mbTilesName
{
    self = [super init];
    if (!self)
        return nil;
    name = mbTilesName;
    active = false;

    NSString *infoPath = nil;
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
    
    // Open the sqlite DB
    if (sqlite3_open([infoPath cStringUsingEncoding:NSASCIIStringEncoding],&sqlDb) != SQLITE_OK)
    {
        return nil;
    }
    
    coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    // Look at the metadata
    try
    {
        sqlhelpers::StatementRead readStmt(sqlDb,@"select value from metadata where name='bounds';");
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
            geoMbr.ll() = GeoCoord::CoordFromDegrees(ll_lon,ll_lat);
            geoMbr.ur() = GeoCoord::CoordFromDegrees(ur_lon,ur_lat);
        } else {
            // No bounds implies it covers the whole earth
            geoMbr.ll() = GeoCoord::CoordFromDegrees(-180, -85.0511);
            geoMbr.ur() = GeoCoord::CoordFromDegrees(180, 85.0511);
        }
        
        // And let's convert that over to spherical mercator
        Point3f ll = [coordSys getCoordSystem]->geographicToLocal(geoMbr.ll());
        mbr.ll() = Point2f(ll.x(),ll.y());
        Point3f ur = [coordSys getCoordSystem]->geographicToLocal(geoMbr.ur());
        mbr.ur() = Point2f(ur.x(),ur.y());
        
        minZoom = 0;  maxZoom = 8;
        sqlhelpers::StatementRead readStmt2(sqlDb,@"select value from metadata where name='minzoom';");
        if (readStmt2.stepRow())
            minZoom = [readStmt2.getString() intValue];
        else {
            // Read it the hard way
            sqlhelpers::StatementRead readStmt3(sqlDb,@"select min(zoom_level) from tiles;");
            if (readStmt3.stepRow())
                minZoom = [readStmt3.getString() intValue];
        }
        sqlhelpers::StatementRead readStmt3(sqlDb,@"select value from metadata where name='maxzoom';");
        if (readStmt3.stepRow())
            maxZoom = [readStmt3.getString() intValue];
        else {
            // Read it the hard way
            sqlhelpers::StatementRead readStmt3(sqlDb,@"select max(zoom_level) from tiles;");
            if (readStmt3.stepRow())
                maxZoom = [readStmt3.getString() intValue];
        }
        
        sqlhelpers::StatementRead readStmt4(sqlDb,"select value from metadata where name='format';");
        if (readStmt4.stepRow())
            _format = readStmt4.getString();

        // See if there's a tiles table or it's the older(?) style
        sqlhelpers::StatementRead testStmt(sqlDb,@"SELECT name FROM sqlite_master WHERE type='table' AND name='tiles';");
        if (testStmt.stepRow())
            tilesStyles = true;
    } catch (int e) {
        NSLog(@"Exception fetching MBTiles metadata");
        return nil;
    }
    
    tileInfo = [[MaplyMBTileInfo alloc] initWithMinZoom:minZoom maxZoom:maxZoom];
    queue = dispatch_queue_create("MBTiles Fetcher", NULL);
    
    active = true;
    return self;
}

- (NSString * _Nonnull)name
{
    return name;
}

- (MaplyCoordinateSystem *)coordSys
{
    return coordSys;
}

- (int)minZoom
{
    return minZoom;
}

- (int)maxZoom
{
    return maxZoom;
}

- (NSObject<MaplyTileInfoNew> *)tileInfo
{
    return tileInfo;
}

- (NSData *)imageForTile:(MaplyMBTileFetchInfo *)tileID
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

- (dispatch_queue_t)getQueue
{
    return queue;
}

- (void)updateLoading
{
    loadScheduled = false;
    
    if (!active)
        return;
    
    if (toLoad.empty())
        return;
    
    // Take the first one off the stack
    auto it = toLoad.rbegin();
    TileInfoRef tile = *it;
    
    // The actual data fetch.  Woo.
    NSData *imageData = [self imageForTile:tile->fetchInfo];
    NSError *error = nil;
    if (!imageData) {
        error = [[NSError alloc] initWithDomain:@"MaplyMBTileFetcher" code:0 userInfo:@{NSLocalizedDescriptionKey: @"Failed to fetch tile from sqlite file"}];
    }

    MaplyMBTileFetcher * __weak weakSelf = self;
    // Do the callback on a background queue
    // Because the parsing might take a while
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       // We assume the parsing is going to take some time
                       if (imageData) {
                           tile->request.success(tile->request,imageData);
                       } else {
                           tile->request.failure(tile->request, error);
                       }
                       
                       dispatch_queue_t theQueue = [weakSelf getQueue];
                       if (theQueue)
                           dispatch_async(theQueue,
                                          ^{
                                              [weakSelf updateLoading];
                                          });
                   });
    
    [weakSelf finishTile:tile];
}

- (void)finishTile:(TileInfoRef)tile
{
    // Done with the tile, so take it out of here
    toLoad.erase(tile);
    auto rit = tilesByFetchRequest.find(tile->request);
    if (rit != tilesByFetchRequest.end())
        tilesByFetchRequest.erase(rit);
}

- (void)scheduleLoading
{
    if (!active)
        return;
    
    if (!loadScheduled) {
        loadScheduled = true;
        dispatch_async(queue, ^{
            [self updateLoading];
        });
    }
}

- (void)startTileFetches:(NSArray<MaplyTileFetchRequest *> * _Nonnull)requests
{
    if (!active)
        return;

    // Check each of the fetchInfo objects
    for (MaplyTileFetchRequest *request in requests)
        if (![request.fetchInfo isKindOfClass:[MaplyMBTileFetchInfo class]]) {
            NSLog(@"MaplyMBTileFetcher is expecting MaplyMBTileFetchInfo objects.  Rejecting requests.");
            return;
        }

    dispatch_async(queue, ^{
        for (MaplyTileFetchRequest *request in requests) {
            // Set up new request
            TileInfoRef tile(new TileInfo());
            tile->importance = request.importance;
            tile->priority = request.priority;
            tile->request = request;
            tile->fetchInfo = request.fetchInfo;
            self->tilesByFetchRequest[request] = tile;
            self->toLoad.insert(tile);
        }

        [self scheduleLoading];
    });
}

- (void)cancelTileFetches:(NSArray * _Nonnull)requests
{
    if (!active)
        return;

    dispatch_async(queue, ^{
        for (MaplyTileFetchRequest *request in requests) {
            auto it = self->tilesByFetchRequest.find(request);
            if (it == self->tilesByFetchRequest.end()) {
                // Wasn't there.  Ignore.
                return;
            }
            TileInfoRef tile = it->second;
            self->toLoad.erase(tile);
            self->tilesByFetchRequest.erase(it);
        }
    });
}

- (id _Nonnull)updateTileFetch:(id _Nonnull)request priority:(int)priority importance:(double)importance
{
    if (!active)
        return nil;
    
    dispatch_async(queue, ^{
        auto it = self->tilesByFetchRequest.find(request);
        if (it == self->tilesByFetchRequest.end())
            return;
        
        // Reinsert the tile with the new values
        TileInfoRef tile = it->second;
        self->toLoad.erase(tile);
        tile->priority = priority;
        tile->importance = importance;
        self->toLoad.insert(tile);
    });
    
    return request;
}

- (void)shutdown
{
    active = false;

    // Execute an empty task and wait for it to return
    // This drains the queue
    dispatch_sync(queue, ^{});
    
    if (sqlDb)
        sqlite3_close(sqlDb);
    
    queue = nil;
}

@end
