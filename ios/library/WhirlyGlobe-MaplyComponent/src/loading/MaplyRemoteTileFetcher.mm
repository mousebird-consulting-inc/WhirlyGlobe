/*
 *  MaplyTileFetcher.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/15/18.
 *  Copyright 2011-2019 Saildrone Inc
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

#import "loading/MaplyRemoteTileFetcher.h"
#import "MaplyRenderController_private.h"

namespace WhirlyKit
{

// A single tile that we're aware of
class TileInfo
{
public:
    TileInfo()
    : state(ToLoad), isLocal(false), tileSource(NULL), priority(0), importance(0.0), group(0), request(nil), fetchInfo(nil), task(nil) { }

    /// Comparison based on importance, tile source, then x,y,level
    bool operator < (const TileInfo &that) const
    {
        if (this->isLocal == that.isLocal) {
            if (this->priority == that.priority) {
                if (this->importance == that.importance) {
                    if (group == that.group)
                    {
                        if (tileSource == that.tileSource) {
                            return request < that.request;
                        }
                        return tileSource < that.tileSource;
                    }
                    return group < that.group;
                }
                return this->importance < that.importance;
            }
            return this->priority > that.priority;
        }
        return this->isLocal < that.isLocal;
    }
    
    // Clean up references to make things happier
    void clear() {
//        request = nil;
//        fetchInfo = nil;
//        task = nil;
    }

    // We're either loading it or going to load it eventually
    typedef enum {ToLoad,Loading} State;
    State state;
    
    // Set if we know the tile is cached
    bool isLocal;

    // Used to uniquely identify a group of requests
    id tileSource;
    
    // Priority before importance
    int priority;
    
    // Importance of this tile request as passed in by the fetch request
    double importance;
    
    // Group last.  Used for tiles with multiple sources.
    int group;

    // The request as it came from outside the tile fetcher
    MaplyTileFetchRequest *request;
    
    // Specific fetchInfo from the fetch request.
    MaplyRemoteTileFetchInfo *fetchInfo;
    
    // If we're loading it, this is the data task associated with it
    NSURLSessionDataTask *task;
    
    // When this request was first queued
    NSTimeInterval startTime;
};


typedef std::shared_ptr<TileInfo> TileInfoRef;
typedef struct {
    bool operator () (const TileInfoRef a,const TileInfoRef b) const {
        return *a < *b;
    }
} TileInfoSorter;
typedef std::set<TileInfoRef,TileInfoSorter> TileInfoSet;
typedef std::map<MaplyTileFetchRequest *,TileInfoRef> TileFetchMap;

}

using namespace WhirlyKit;

@implementation MaplyRemoteTileInfoNew
{
    std::vector<MbrD> validMbrs;
    MaplyCoordinateSystem *bboxCoordSys;
}

- (nonnull instancetype)initWithBaseURL:(NSString *__nonnull)inBaseURL minZoom:(int)inMinZoom maxZoom:(int)inMaxZoom
{
    self = [super init];
    _baseURL = inBaseURL;
    _minZoom = inMinZoom;
    _maxZoom = inMaxZoom;
    
    return self;
}

- (void)addValidBounds:(MaplyBoundingBoxD)bbox coordSystem:(MaplyCoordinateSystem *)coordSys
{
    MbrD validMbr;
    validMbr.addPoint(Point2d(bbox.ll.x,bbox.ll.y));
    validMbr.addPoint(Point2d(bbox.ur.x,bbox.ur.y));
    validMbrs.push_back(validMbr);
    
    bboxCoordSys = coordSys;
}

- (bool)tileIsValid:(MaplyTileID)tileID
{
    if (!_coordSys || !bboxCoordSys || validMbrs.empty())
        return true;

    // Bounding box for the whole coordinate system
    MaplyBoundingBox wholeBBox = [_coordSys getBounds];
    MbrD wholeMbr;
    wholeMbr.addPoint(Point2f(wholeBBox.ll.x,wholeBBox.ll.y));
    wholeMbr.addPoint(Point2f(wholeBBox.ur.x,wholeBBox.ur.y));

    // Make the bounding box for this particular tile (in the native coord system)
    int numLevel = 1<<tileID.level;
    double spanX = wholeBBox.ur.x - wholeBBox.ll.x;
    double spanY = wholeBBox.ur.y - wholeBBox.ll.y;
    double dx = spanX/numLevel, dy = spanY/numLevel;
    Point3d pts[4];
    double nudge = spanX * 1e-7;   // Nudge things in a bit to avoid a round earth problem
    pts[0] = Point3d(wholeBBox.ll.x+dx*tileID.x,wholeBBox.ll.y+dy*tileID.y,0.0) + Point3d(nudge,nudge,0.0);
    pts[1] = Point3d(wholeBBox.ll.x+dx*(tileID.x+1),wholeBBox.ll.y+dy*tileID.y,0.0) + Point3d(-nudge,nudge,0.0);
    pts[2] = Point3d(wholeBBox.ll.x+dx*(tileID.x+1),wholeBBox.ll.y+dy*(tileID.y+1),0.0) + Point3d(-nudge,-nudge,0.0);
    pts[3] = Point3d(wholeBBox.ll.x+dx*tileID.x,wholeBBox.ll.y+dy*(tileID.y+1),0.0) + Point3d(nudge,-nudge,0.0);

    // Project the corners into our coordinate system
    MbrD tileMbr;
    for (unsigned int ii=0;ii<4;ii++) {
        Point3d validPt = CoordSystemConvert3d([_coordSys getCoordSystem].get(), [bboxCoordSys getCoordSystem].get(), pts[ii]);
        tileMbr.addPoint(Point2d(validPt.x(),validPt.y()));
    }

    // Within any of the valid MBRs is fine
    for (auto validMbr : validMbrs)
        if (tileMbr.overlaps(validMbr))
            return true;
    
    return false;
}

- (NSURLRequest *)urlRequestForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    if (![self tileIsValid:tileID])
        return nil;
    
    int y = flipY ? ((int)(1<<tileID.level)-tileID.y)-1 : tileID.y;
    NSMutableURLRequest *urlReq = nil;

    // Fill out the replacement string
    NSString *fullURLStr = [[[_baseURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                             stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                            stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
    urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
    if (_timeOut != 0.0)
        [urlReq setTimeoutInterval:_timeOut];
    
    for (NSString *key in _headers.allKeys) {
        [urlReq addValue:_headers[key] forHTTPHeaderField:key];
    }
    
    return urlReq;
}

- (NSString *)fileNameForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    if (!_cacheDir)
        return nil;
    
    NSString *localName = [NSString stringWithFormat:@"%@/%d_%d_%d",_cacheDir,tileID.level,tileID.x,tileID.y];
    
    return localName;
}

- (id _Nullable)fetchInfoForTile:(MaplyTileID)tileID flipY:(bool)flipY
{
    MaplyRemoteTileFetchInfo *fetchInfo = [[MaplyRemoteTileFetchInfo alloc] init];
    fetchInfo.urlReq = [self urlRequestForTile:tileID flipY:flipY];
    fetchInfo.cacheFile = [self fileNameForTile:tileID flipY:flipY];
    
    if (!fetchInfo.urlReq)
        return nil;
    
    return fetchInfo;
}

@end

@implementation MaplyRemoteTileFetchInfo

- (id)init
{
    self = [super init];
    
    return self;
}

@end

@implementation MaplyRemoteTileFetcherStats

-(instancetype)initWithFetcher:(MaplyRemoteTileFetcher *)fetcher
{
    self = [super init];
    _fetcher = fetcher;
    _startDate = [[NSDate alloc] init];
    _totalRequests = 0;
    _remoteRequests = 0;
    _totalCancels = 0;
    _totalFails = 0;
    _remoteData = 0;
    _localData = 0;
    _totalLatency = 0;
    _maxActiveRequests = 0;
    _activeRequests = 0;
    
    return self;
}

- (void)dealloc
{
    
}

- (void)addStats:(MaplyRemoteTileFetcherStats * __nonnull)stats
{
    _totalRequests += stats.totalRequests;
    _remoteRequests += stats.remoteRequests;
    _totalCancels += stats.totalCancels;
    _totalFails += stats.totalFails;
    _remoteData += stats.remoteData;
    _localData += stats.localData;
    _totalLatency += stats.totalLatency;
    // Active requests don't add
}

- (void)dump
{
    NSLog(@"---MaplyTileFetcher %@ Stats---",_fetcher.name);
    NSLog(@"   Active Requests = %d",_activeRequests);
    NSLog(@"   Max Active Requests = %d",_maxActiveRequests);
    NSLog(@"   Total Requests = %d",_totalRequests);
    NSLog(@"   Canceled Requests = %d",_totalCancels);
    NSLog(@"   Failed Requests = %d",_totalFails);
    NSLog(@"   Data Transferred = %.2fMB",_remoteData / (1024.0*1024.0));
    if (_remoteRequests > 0) {
        NSLog(@"   Latency per request = %.2fms",_totalLatency / _remoteRequests * 1000.0);
        NSLog(@"   Average request size = %.2fKB",_remoteData / _remoteRequests / 1024.0);
    }
    NSLog(@"   Cached Data = %.2fMB",_localData / (1024.0*1024.0));
    NSLog(@"   Num Simultaneous = %d",_fetcher.numConnections);
}

@end

@implementation MaplyRemoteTileFetcherLogEntry
@end

@implementation MaplyRemoteTileFetcherLog
{
    std::vector<MaplyRemoteTileFetcherLogEntry *> entries;
}

- (void)addRemoteFailure:(TileInfoRef)tileInfo
{
    MaplyRemoteTileFetcherLogEntry *entry = [[MaplyRemoteTileFetcherLogEntry alloc] init];
    entry.urlReq = tileInfo->fetchInfo.urlReq;
    entry.success = false;
    entry.size = 0;
    entry.wasCached = false;
    entry.queuedTime = tileInfo->startTime;
    entry.startedTime = tileInfo->startTime;
    entry.finishedTime = TimeGetCurrent();

    @synchronized (self) {
        entries.push_back(entry);
    }
}

- (void)addRemoteSuccess:(TileInfoRef)tileInfo length:(int)length startTime:(NSTimeInterval)fetchStartTile
{
    MaplyRemoteTileFetcherLogEntry *entry = [[MaplyRemoteTileFetcherLogEntry alloc] init];
    entry.urlReq = tileInfo->fetchInfo.urlReq;
    entry.success = true;
    entry.size = length;
    entry.wasCached = false;
    entry.queuedTime = tileInfo->startTime;
    entry.startedTime = fetchStartTile;
    entry.finishedTime = TimeGetCurrent();

    @synchronized (self) {
        entries.push_back(entry);
    }
}

- (void)addCache:(TileInfoRef)tileInfo length:(int)length
{
    MaplyRemoteTileFetcherLogEntry *entry = [[MaplyRemoteTileFetcherLogEntry alloc] init];
    entry.urlReq = tileInfo->fetchInfo.urlReq;
    entry.success = true;
    entry.size = length;
    entry.wasCached = true;
    entry.queuedTime = tileInfo->startTime;
    entry.startedTime = tileInfo->startTime;
    entry.finishedTime = TimeGetCurrent();

    @synchronized (self) {
        entries.push_back(entry);
    }
}

- (NSArray<MaplyRemoteTileFetcherLogEntry *> * __nullable)getEntries
{
    @synchronized (self) {
        NSMutableArray *ret = [[NSMutableArray alloc] init];
        for (MaplyRemoteTileFetcherLogEntry *entry : entries) {
            [ret addObject:entry];
        }
        
        self.endTime = TimeGetCurrent();
        return ret;
    }
}

- (NSString *)dump
{
    NSArray<MaplyRemoteTileFetcherLogEntry *> *theEntries = [self getEntries];
    std::stringstream sstr;
    
    // (R) latency fetchLatency size URL
    // (C) latency size URL
    // (F) URL
    sstr << "TYPE,WHEN(s),TOTAL(s),NETWORK(s),SIZE(k),URL\n";
    sstr.precision(2);
    for (MaplyRemoteTileFetcherLogEntry *entry in theEntries) {
        if (entry.success) {
            sstr << (entry.wasCached ? "(C)" : "(R)") << ",";
            sstr << entry.startedTime-_startTime << ",";
            sstr << entry.finishedTime-entry.queuedTime << ",";
            if (entry.wasCached)
                sstr << "";
            else
                sstr << entry.finishedTime-entry.startedTime;
            sstr << ",";
        } else {
            sstr << "(F),,,,";
        }
        sstr << (float)entry.size/1024 << ",";
        sstr << [entry.urlReq.URL.path cStringUsingEncoding:NSASCIIStringEncoding];
        sstr << "\n";
    }
    
    return [NSString stringWithFormat:@"%s", sstr.str().c_str()];
}

@end

@implementation MaplyRemoteTileFetcher
{
    bool active;
    NSString *name;
    NSURLSession *session;
    dispatch_queue_t queue;
    
    TileInfoSet loading;  // Tiles that are currently loading
    TileFetchMap tilesByFetchRequest;  // Tiles sorted by fetch request
    TileInfoSet toLoad;  // Tiles sorted by importance
    
    // Keeps track of stats
    MaplyRemoteTileFetcherStats *allStats;
    MaplyRemoteTileFetcherStats *recentStats;
    
    MaplyRemoteTileFetcherLog *log;
}

- (instancetype)initWithName:(NSString *)inName connections:(int)numConnections
{
    self = [super init];
    name = inName;
    active = true;
    _numConnections = numConnections;
    // All the internal work is done on a single queue.  Nothing significant, really.
    queue = dispatch_queue_create("MaplyRemoteTileFetcher", DISPATCH_QUEUE_SERIAL);
    session = [NSURLSession sharedSession];
    allStats = [[MaplyRemoteTileFetcherStats alloc] initWithFetcher:self];
    recentStats = [[MaplyRemoteTileFetcherStats alloc] initWithFetcher:self];
            
    return self;
}

/// Return the fetching stats since the beginning or since the last reset
- (MaplyRemoteTileFetcherStats * __nullable)getStats:(bool)allTime
{
    if (allTime) {
        return allStats;
    } else {
        return recentStats;
    }
}

- (NSString * _Nonnull)name
{
    return name;
}

- (void)resetStats
{
    recentStats = [[MaplyRemoteTileFetcherStats alloc] initWithFetcher:self];
}

- (void)resetActiveStats
{
    MaplyRemoteTileFetcher * __weak weakSelf = self;
    
    dispatch_async(queue,
                   ^{
                       [weakSelf resetActiveStatsLocal];
                   });
}

- (void)resetActiveStatsLocal
{
    recentStats.activeRequests = toLoad.size() + loading.size();
    recentStats.maxActiveRequests = recentStats.activeRequests;
}

- (void)startLogging
{
    log = [[MaplyRemoteTileFetcherLog alloc] init];
    log.startTime = TimeGetCurrent();
}

- (MaplyRemoteTileFetcherLog *)stopLogging
{
    if (log) {
        MaplyRemoteTileFetcherLog *retLog = log;
        log.endTime = TimeGetCurrent();
        log = nil;
        return retLog;
    }
    
    return nil;
}


- (dispatch_queue_t)getQueue
{
    return queue;
}

- (void)startTileFetches:(NSArray<MaplyTileFetchRequest *> *)requests
{
    if (!active || ![requests count])
        return;
    
    // Check each of the fetchInfo objects
    for (MaplyTileFetchRequest *request in requests)
        if (![request.fetchInfo isKindOfClass:[MaplyRemoteTileFetchInfo class]]) {
            NSLog(@"MaplyRemoteTileFetcher is expecting MaplyRemoteTileFetchInfo objects.  Rejecting requests.");
            return;
        }
    
    MaplyRemoteTileFetcher * __weak weakSelf = self;
    dispatch_async(queue,
    ^{
       [weakSelf startTileFetchesLocal:requests];
    });
}

- (void)cancelTileFetches:(NSArray<MaplyTileFetchRequest *> *)requests
{
    if (!active || ![requests count])
        return;
    
    MaplyRemoteTileFetcher * __weak weakSelf = self;
    dispatch_async(queue,
                   ^{
                       [weakSelf cancelTileFetchesLocal:requests];
                   });
}

// Run on the dispatch queue
- (void)startTileFetchesLocal:(NSArray<MaplyTileFetchRequest *> *)requests
{
    allStats.totalRequests = allStats.totalRequests + requests.count;
    recentStats.totalRequests = recentStats.totalRequests + requests.count;

    NSTimeInterval now = TimeGetCurrent();
    
    for (MaplyTileFetchRequest *request in requests) {
        // Set up new request
        TileInfoRef tile(new TileInfo());
        tile->tileSource = request.tileSource;
        tile->importance = request.importance;
        tile->priority = request.priority;
        tile->group = request.group;
        tile->state = TileInfo::ToLoad;
        tile->request = request;
        tile->fetchInfo = request.fetchInfo;
        tile->startTime = now;
        tilesByFetchRequest[request] = tile;

        // If it's already cached, just short circuit this
        if (tile->fetchInfo.cacheFile && [self isTileLocal:tile fileName:tile->fetchInfo.cacheFile])
            tile->isLocal = true;

        // Just run the normal load
        toLoad.insert(tile);
    }
    
    [self updateLoading];
}

/// Update an active request with a new priority and importance
- (id)updateTileFetch:(id)request priority:(int)priority importance:(double)importance
{
    if (!active)
        return nil;
    
    MaplyRemoteTileFetcher * __weak weakSelf = self;
    dispatch_async(queue,
    ^{
       [weakSelf updateTileFetchLocal:request priority:priority importance:importance];
    });
    
    return request;
}

// Run on the dispatch queue
- (void)updateTileFetchLocal:(MaplyTileFetchRequest *)request priority:(int)priority importance:(double)importance
{
    auto it = tilesByFetchRequest.find(request);
    if (it == tilesByFetchRequest.end())
        return;
    
    TileInfoRef tile = it->second;
    // Don't mess with a tile that's actually loading
    if (tile->state == TileInfo::ToLoad) {
        // Change the priority/importance and put it back
        toLoad.erase(tile);
        tile->priority = priority;
        tile->importance = importance;
        toLoad.insert(tile);
    }
}

// Run on the dispatch queue
- (void)cancelTileFetchesLocal:(NSArray<MaplyTileFetchRequest *> *)requests
{
    allStats.totalCancels = allStats.totalCancels + 1;
    recentStats.totalCancels = recentStats.totalCancels + 1;

    for (MaplyTileFetchRequest *request in requests) {
        auto it = tilesByFetchRequest.find(request);
        if (it == tilesByFetchRequest.end()) {
            // Wasn't there.  Ignore.
            return;
        }
        TileInfoRef tile = it->second;
        switch (tile->state) {
            case TileInfo::Loading:
                [tile->task cancel];
                break;
            case TileInfo::ToLoad:
                break;
        }
        loading.erase(tile);
        toLoad.erase(tile);
        tile->clear();
        tilesByFetchRequest.erase(it);
    }
    
    [self updateLoading];
}

- (bool)isTileLocal:(TileInfoRef)tile fileName:(NSString *)fileName
{
    if (!fileName)
        return false;
    
    if ([[NSFileManager defaultManager] fileExistsAtPath:fileName])
    {
        return true;
        // Note: Consider moving this logic over here
        // If the file is out of date, treat it as if it were not local, as it will have to be fetched.
//        if (self.cachedFileLifetime != 0)
//        {
//            NSDate *fileTimestamp = [MaplyRemoteTileInfo dateForFile:fileName];
//            int ageOfFile = (int) [[NSDate date] timeIntervalSinceDate:fileTimestamp];
//            if (ageOfFile <= self.cachedFileLifetime)
//            {
//                return true;
//            }
//            //            else
//            //            {
//            //                NSLog(@"TileIsLocal returned false due to tile age: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
//            //            }
//        }
//        else // no lifetime set for cached files
//        {
//            return true;
//        }
    }
    
    return false;
}

- (void)writeToCache:(TileInfoRef)tileInfo tileData:(NSData *)tileData
{
    if (tileInfo->fetchInfo.cacheFile) {
        NSString *dir = [tileInfo->fetchInfo.cacheFile stringByDeletingLastPathComponent];
        NSString *cacheFile = tileInfo->fetchInfo.cacheFile;
        
        // Do the actual writing somewhere else
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            NSError *error;
            [[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:&error];
            [tileData writeToFile:cacheFile atomically:NO];
        });
    }
}

- (NSData *)readFromCache:(TileInfoRef)tileInfo
{
    if (!tileInfo->fetchInfo.cacheFile)
        return nil;
    return [NSData dataWithContentsOfFile:tileInfo->fetchInfo.cacheFile];
}

// Run on the dispatch queue
- (void)updateLoading
{
    // Ask for a few more to load
    while (loading.size() < _numConnections) {
        [self updateActiveStats];

        auto nextLoad = toLoad.rbegin();
        if (nextLoad == toLoad.rend())
            break;
        
        // Move it into loading
        TileInfoRef tile = *nextLoad;
        toLoad.erase(std::next(nextLoad).base());
        tile->state = TileInfo::Loading;
        loading.insert(tile);
        
        NSURLRequest *urlReq = tile->fetchInfo.urlReq;
        
        TimeInterval fetchStartTile = TimeGetCurrent();
        
        if (_debugMode)
            NSLog(@"Started load: %@ priority = %d, importance = %f, group = %d",urlReq.URL.absoluteString,tile->priority,tile->importance,tile->group);
        
        // Set up the fetch task so we can use it in a couple places
        MaplyRemoteTileFetcher * __weak weakSelf = self;
        tile->task = [session dataTaskWithRequest:urlReq completionHandler:
                      ^(NSData * _Nullable data, NSURLResponse * _Nullable inResponse, NSError * _Nullable error) {
                          NSHTTPURLResponse *response = (NSHTTPURLResponse *)inResponse;

                          dispatch_queue_t queue = [weakSelf getQueue];
                          if (queue)
                              dispatch_async(queue,
                                 ^{
                                     if (weakSelf)
                                         [weakSelf handleData:data response:response error:error tile:tile fetchStart:fetchStartTile];
                                 });
                        }];
        
        // Look for it cached
        if ([self isTileLocal:tile fileName:tile->fetchInfo.cacheFile]) {
            // Do the reading somewhere else
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                [weakSelf handleCache:tile];
            });
        } else {
            [tile->task resume];
        }
    }

    [self updateActiveStats];
}

- (void)updateActiveStats
{
    recentStats.activeRequests = loading.size()+toLoad.size();
    recentStats.maxActiveRequests = std::max(recentStats.maxActiveRequests,recentStats.activeRequests);
}

- (void)handleData:(NSData *)data response:(NSHTTPURLResponse *)response error:(NSError *)error tile:(TileInfoRef)tile fetchStart:(TimeInterval)fetchStartTile
{
    // 204 means no content, which is just empty data for most cases
    if (error || (response.statusCode != 200 && response.statusCode != 204)) {
       // Cancels don't count as errors
       if (!error || error.code != NSURLErrorCancelled) {
           allStats.totalFails = allStats.totalFails + 1;
           recentStats.totalFails = recentStats.totalFails + 1;
           // Build an NSError around the status code
           if (!error) {
               error = [[NSError alloc] initWithDomain:@"MaplyRemoteTileFetcher"
                                                  code:response.statusCode
                                              userInfo:@{NSLocalizedDescriptionKey:[NSString stringWithFormat:@"Server response: %d",(int)response.statusCode]}];
           }
           [self finishedLoading:tile data:nil error:error];
       }
    } else {
       
       int length = [data length];

       if (_debugMode)
           NSLog(@"Remote return for: %@, %dk",tile->fetchInfo.urlReq.URL.absoluteString,length / 1024);
       if (log)
           [log addRemoteSuccess:tile length:length startTime:fetchStartTile];

       allStats.remoteRequests = allStats.remoteRequests + 1;
       recentStats.remoteRequests = recentStats.remoteRequests + 1;
       allStats.remoteData = allStats.remoteData + length;
       recentStats.remoteData = recentStats.remoteData + length;
       TimeInterval howLong = TimeGetCurrent() - fetchStartTile;
       allStats.totalLatency = allStats.totalLatency + howLong;
       recentStats.totalLatency = recentStats.totalLatency + howLong;
       [self finishedLoading:tile data:data error:error];
       [self writeToCache:tile tileData:data];
    }
}

- (void)handleCache:(TileInfoRef)tile
{
    NSData *data = [self readFromCache:tile];
    if (!data) {
        // It failed (which happens) so we need to fetch it after all
        [tile->task resume];
    } else {
        tile->task = nil;
        
        if (_debugMode)
            NSLog(@"Cache for: %@, %dk",tile->fetchInfo.urlReq.URL.absoluteString,(int)[data length] / 1024);
        if (log)
            [log addCache:tile length:[data length]];
        
        MaplyRemoteTileFetcher * __weak weakSelf = self;
        // It worked, but run the finished loading back on our queue
        dispatch_async(queue,^{
            [weakSelf handleFinishLoading:data tile:tile];
        });
    }
}

- (void)handleFinishLoading:(NSData *)data tile:(TileInfoRef)tile
{
    int length = [data length];
    allStats.localData = allStats.localData + length;
    recentStats.localData = recentStats.localData + length;
    [self finishedLoading:tile data:data error:nil];
}

// Called on our queue
- (void)finishTile:(TileInfoRef)tile
{
    auto it = tilesByFetchRequest.find(tile->request);
    if (it != tilesByFetchRequest.end()) {
        tilesByFetchRequest.erase(it);
    }
    loading.erase(tile);
    toLoad.erase(tile);
    tile->clear();

    [self updateActiveStats];
}

// Called on our queue
- (void)finishedLoading:(TileInfoRef)tile data:(NSData *)data error:(NSError *)error
{
    if (!data && log)
        [log addRemoteFailure:tile];
    
    auto it = tilesByFetchRequest.find(tile->request);
    if (it == tilesByFetchRequest.end()) {
        loading.erase(tile);
        toLoad.erase(tile);
        tile->clear();
        // No idea what it is.  Toss it.
        return;
    }
    
    MaplyRemoteTileFetcher * __weak weakSelf = self;
    
    // Do the callback on a background queue
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
    ^{
        if (!tile)
            return;
        
        // We assume the parsing is going to take some time
        if (!error) {
            if (tile->request)
                tile->request.success(tile->request,data);
        } else {
            if (tile->request)
                tile->request.failure(tile->request, error);
        }

        dispatch_queue_t theQueue = [weakSelf getQueue];
        if (theQueue) {
            dispatch_async(theQueue,
            ^{
                [weakSelf finishTile:tile];

                [weakSelf updateLoading];
            });
        } else {
            tile->clear();
        }
    });
}

- (void)shutdown
{
    active = false;
    
    // Execute an empty task and wait for it to return
    // This drains the queue
    dispatch_sync(queue, ^{});
    
    toLoad.clear();
    loading.clear();
    for (auto it : tilesByFetchRequest) {
        it.second->clear();
    }
    tilesByFetchRequest.clear();
}

@end
