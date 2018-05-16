/*
 *  MaplyRemoteTileSource.mm
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

#import "MaplyRemoteTileSource.h"
#import "WhirlyGlobe.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyRemoteTileSource_private.h"

using namespace Eigen;
using namespace WhirlyKit;

// Number of connections among all remote tile sources
static int numConnections = 0;
static bool trackConnections = false;

@implementation MaplyRemoteTileInfo
{
    NSDictionary *_jsonSpec;
    NSArray *_tileURLs;
    WhirlyKit::Mbr _mbr;
    bool cacheInit;
    int _minZoom,_maxZoom;
    int _pixelsPerSide;
    std::vector<Mbr> mbrs;
}

- (instancetype)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    _baseURL = baseURL;
    _ext = ext;
    _minZoom = minZoom;
    _maxZoom = maxZoom;
    _pixelsPerSide = 256;
    _timeOut = 0.0;
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    _replaceURL = [baseURL containsString:@"{y}"] && [baseURL containsString:@"{x}"]; // && [baseURL containsString:@"{z}"];
    
    return self;
}

- (instancetype)initWithTilespec:(NSDictionary *)jsonDict
{
    self = [super init];
    if (!self)
        return nil;
    
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    _tileURLs = jsonDict[@"tiles"];
    if (![_tileURLs isKindOfClass:[NSArray class]])
        return nil;
    
    // Set'll set up a default to cover the whole world
    GeoCoord ll = GeoCoord::CoordFromDegrees(-180,-85.05113);
    GeoCoord ur = GeoCoord::CoordFromDegrees( 180, 85.05113);
    
    // Then again, there may be a real default
    NSArray *bounds = jsonDict[@"bounds"];
    if ([bounds isKindOfClass:[NSArray class]] && [bounds count] == 4)
    {
        ll = GeoCoord::CoordFromDegrees([[bounds objectAtIndex:0] floatValue],[[bounds objectAtIndex:1] floatValue]);
        ur = GeoCoord::CoordFromDegrees([[bounds objectAtIndex:2] floatValue],[[bounds objectAtIndex:3] floatValue]);
    }
    Point3f ll3d = [_coordSys getCoordSystem]->geographicToLocal(ll);
    Point3f ur3d = [_coordSys getCoordSystem]->geographicToLocal(ur);
    _mbr.ll() = Point2f(ll3d.x(),ll3d.y());
    _mbr.ur() = Point2f(ur3d.x(),ur3d.y());
    
    _minZoom = _maxZoom = 0;
    _minZoom = [jsonDict[@"minzoom"] intValue];
    _maxZoom = [jsonDict[@"maxzoom"] intValue];
    
    _pixelsPerSide = 256;
    
    return self;
}

- (int)tileSize
{
    return _pixelsPerSide;
}

- (void)addBoundingBox:(MaplyBoundingBox *)bbox
{
    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    mbrs.push_back(mbr);
}

- (void)addGeoBoundingBox:(MaplyBoundingBox *)bbox
{
    Mbr mbr;
    Point3f pt0 = _coordSys->coordSystem->geographicToLocal(GeoCoord(bbox->ll.x,bbox->ll.y));
    mbr.addPoint(Point2f(pt0.x(),pt0.y()));
    Point3f pt1 = _coordSys->coordSystem->geographicToLocal(GeoCoord(bbox->ur.x,bbox->ur.y));
    mbr.addPoint(Point2f(pt1.x(),pt1.y()));
    mbrs.push_back(mbr);
}

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox
{
    if(tileID.level > _maxZoom)
      return false;

    if (mbrs.empty())
        return true;
    
    Mbr mbr(Point2f(bbox.ll.x,bbox.ll.y),Point2f(bbox.ur.x,bbox.ur.y));
    for (unsigned int ii=0;ii<mbrs.size();ii++)
        if (mbr.overlaps(mbrs[ii]))
            return true;
    
    return false;
}

// Figure out the name for the tile, if it's local
- (NSString *)fileNameForTile:(MaplyTileID)tileID
{
    if (!_cacheDir)
        return nil;
    
    // If there's a cache dir, make sure it's there
    if (!cacheInit)
    {
        if (_cacheDir)
        {
            NSError *error = nil;
            [[NSFileManager defaultManager] createDirectoryAtPath:_cacheDir withIntermediateDirectories:YES attributes:nil error:&error];
        }
        cacheInit = true;
    }
    
    NSString *localName = [NSString stringWithFormat:@"%@/%d_%d_%d.%@",_cacheDir,tileID.level,tileID.x,tileID.y,(_ext ? _ext : @"unk")];
    
    if (_cachedFileLifetime > 0)
    {
        NSDate *timeStamp = [MaplyRemoteTileInfo dateForFile:localName];
        if (timeStamp)
        {
            int ageOfFile = (int) [[NSDate date] timeIntervalSinceDate:timeStamp];
            if (ageOfFile > _cachedFileLifetime)
                return nil;
        }
    }
    
    return localName;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    if (!_cacheDir)
        return false;
    
    NSString *fileName = [self fileNameForTile:tileID];
    if ([[NSFileManager defaultManager] fileExistsAtPath:fileName])
    {
        // If the file is out of date, treat it as if it were not local, as it will have to be fetched.
        if (self.cachedFileLifetime != 0)
        {
            NSDate *fileTimestamp = [MaplyRemoteTileInfo dateForFile:fileName];
            int ageOfFile = (int) [[NSDate date] timeIntervalSinceDate:fileTimestamp];
            if (ageOfFile <= self.cachedFileLifetime)
            {
                return true;
            }
//            else
//            {
//                NSLog(@"TileIsLocal returned false due to tile age: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
//            }
        }
        else // no lifetime set for cached files
        {
            return true;
        }
    }
    
    return false;
}

+ (NSDate *)dateForFile:(NSString *)fileName
{
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:fileName error:nil];
    NSDate *fileTimestamp = [fileAttributes fileModificationDate];
    
    return fileTimestamp;
}

- (NSData *)readFromCache:(MaplyTileID)tileID
{
    if (!_cacheDir)
        return nil;
    
    NSString *fileName = [self fileNameForTile:tileID];
    return [NSData dataWithContentsOfFile:fileName];
}

- (void)writeToCache:(MaplyTileID)tileID tileData:(NSData * _Nonnull)tileData
{
    if (!_cacheDir)
        return;
    
    NSString *fileName = [self fileNameForTile:tileID];
    
    if (fileName)
        [tileData writeToFile:fileName atomically:YES];
}

- (NSURLRequest *)requestForTile:(MaplyTileID)tileID
{
    int y = ((int)(1<<tileID.level)-tileID.y)-1;
    NSMutableURLRequest *urlReq = nil;
    
    if (_tileURLs)
    {
        // Decide here which URL we'll use
        NSString *tileURL = [_tileURLs objectAtIndex:tileID.x%[_tileURLs count]];
        
        // Use the JSON tile spec
        NSString *fullURLStr = [[[tileURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                                 stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                                stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
        urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
        if (_timeOut != 0.0)
            [urlReq setTimeoutInterval:_timeOut];
    } else {
        if (_replaceURL)
        {
            // Fetch the traditional way
            NSString *fullURLStr = [[[_baseURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                                     stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                                    stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
            if (_ext)
                fullURLStr = [NSString stringWithFormat:@"%@.%@",fullURLStr,(_ext ? _ext : @"unk")];
            urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
            if (_timeOut != 0.0)
                [urlReq setTimeoutInterval:_timeOut];
        } else {
            // Fetch the traditional way
            NSMutableString *fullURLStr = [NSMutableString stringWithFormat:@"%@%d/%d/%d.%@",_baseURL,tileID.level,tileID.x,y,(_ext ? _ext : @"unk")];
            if (_queryStr)
                [fullURLStr appendFormat:@"?%@",_queryStr];
            urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
            if (_timeOut != 0.0)
                [urlReq setTimeoutInterval:_timeOut];
        }
    }
    
    return urlReq;
}

@end

@implementation MaplyRemoteTileSource
{
    Maply::TileFetchOpSet tileSet;
}

- (instancetype)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = [[MaplyRemoteTileInfo alloc] initWithBaseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom];
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (instancetype)initWithTilespec:(NSDictionary *)jsonDict
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = [[MaplyRemoteTileInfo alloc] initWithTilespec:jsonDict];
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (instancetype)initWithInfo:(MaplyRemoteTileInfo *)info
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = info;
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (void)dealloc
{
    @synchronized(self)
    {
        for (Maply::TileFetchOpSet::iterator it = tileSet.begin();
             it != tileSet.end(); ++it)
        {
            Maply::TileFetchOp tile = *it;
            [tile.task cancel];
        }
        tileSet.clear();
    }
}

+ (void)setTrackConnections:(bool)track
{
    trackConnections = track;
}

+ (int)numOutstandingConnections
{
    return numConnections;
}

- (MaplyCoordinateSystem *)coordSys
{
    return _tileInfo.coordSys;
}

- (void)setCoordSys:(MaplyCoordinateSystem *)coordSys
{
    if ([_tileInfo isKindOfClass:[MaplyRemoteTileSource class]])
        [(MaplyRemoteTileSource *)_tileInfo setCoordSys:coordSys];
}

- (NSString *)cacheDir
{
    if ([_tileInfo isKindOfClass:[MaplyRemoteTileSource class]])
        return ((MaplyRemoteTileSource *)_tileInfo).cacheDir;
    else if ([_tileInfo isKindOfClass:[MaplyRemoteTileSource class]])
        return ((MaplyRemoteTileInfo *)_tileInfo).cacheDir;
    else
        return nil;
}

- (void)setCacheDir:(NSString *)cacheDir
{
    if ([_tileInfo isKindOfClass:[MaplyRemoteTileSource class]])
        [(MaplyRemoteTileSource *)_tileInfo setCacheDir:cacheDir];
    else if ([_tileInfo isKindOfClass:[MaplyRemoteTileInfo class]])
        [(MaplyRemoteTileInfo *)_tileInfo setCacheDir: cacheDir];
}

- (int)minZoom
{
    return _tileInfo.minZoom;
}

- (int)maxZoom
{
    return _tileInfo.maxZoom;
}

- (int)tileSize
{
    return _tileInfo.pixelsPerSide;
}

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return [_tileInfo tileIsLocal:tileID frame:frame];
}

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox
{
    return [_tileInfo validTile:tileID bbox:bbox];
}

- (void)tileUnloaded:(MaplyTileID)tileID
{
    if ([_delegate respondsToSelector:@selector(remoteTileSource:tileUnloaded:)])
        [_delegate remoteTileSource:self tileUnloaded:tileID];
}

- (void)tileWasEnabled:(MaplyTileID)tileID
{
    if ([_delegate respondsToSelector:@selector(remoteTileSource:tileEnabled:)])
        [_delegate remoteTileSource:self tileEnabled:tileID];
}

- (void)tileWasDisabled:(MaplyTileID)tileID
{
    if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDisabled:)])
        [_delegate remoteTileSource:self tileDisabled:tileID];
}

// Clear out the operation associated with a tile
- (void)clearTile:(MaplyTileID)tileID
{
    @synchronized(self)
    {
        Maply::TileFetchOpSet::iterator it = tileSet.find(Maply::TileFetchOp(tileID));
        if (it != tileSet.end())
            tileSet.erase(it);
    }
}

// For a remote tile source, this one only works if it's local
- (id)imageForTile:(MaplyTileID)tileID
{
    if (trackConnections)
        @synchronized([MaplyRemoteTileSource class])
        {
            numConnections++;
        }
    
    if ([_tileInfo tileIsLocal:tileID frame:-1])
    {
        bool doLoad = true;
        
        if (doLoad)
        {
            if (trackConnections)
                @synchronized([MaplyRemoteTileSource class])
            {
                numConnections--;
            }
            NSData *tileData = [_tileInfo readFromCache:tileID];
            if (tileData)
            {
                if ([_delegate respondsToSelector:@selector(remoteTileSource:modifyTileReturn:forTile:)])
                {
                    tileData = [_delegate remoteTileSource:self modifyTileReturn:tileData forTile:tileID];
                }
            }
            
            if (tileData)
                return tileData;
        }
        
    }
    
    NSURLRequest *urlReq = [_tileInfo requestForTile:tileID];
    if(urlReq)
    {
        NSURLResponse *response;
        NSError *error;
        NSData *tileData = [NSURLConnection sendSynchronousRequest:urlReq
                                                 returningResponse:&response error:&error];
        
        // Look at the response
        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
        if (httpResponse.statusCode != 200)
            tileData = nil;
        
        // Let's also write it back out for the cache
        if (tileData)
            [_tileInfo writeToCache:tileID tileData:tileData];
        
        if ([_delegate respondsToSelector:@selector(remoteTileSource:modifyTileReturn:forTile:)])
            tileData = [_delegate remoteTileSource:self modifyTileReturn:tileData forTile:tileID];
        
        if (trackConnections)
            @synchronized([MaplyRemoteTileSource class])
        {
            numConnections--;
        }
        return tileData;
    }
    
    if (trackConnections)
        @synchronized([MaplyRemoteTileSource class])
    {
        numConnections--;
    }
    return nil;
}

- (void)startFetchLayer:(MaplyQuadImageTilesLayer *)layer tile:(MaplyTileID)tileID
{
    if (trackConnections)
        @synchronized([MaplyRemoteTileSource class])
    {
        numConnections++;
    }
    
    NSData *imgData = nil;
    // Look for the image in the cache first
    if ([_tileInfo tileIsLocal:tileID frame:-1])
    {
        imgData = [self imageForTile:tileID];
    }
    
    if (imgData)
    {
        if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
            [_delegate remoteTileSource:self tileDidLoad:tileID];

        // Let the paging layer know about it
        [layer loadedImages:imgData forTile:tileID];
        
        if (trackConnections)
            @synchronized([MaplyRemoteTileSource class])
        {
            numConnections--;
        }
    } else {
        NSURLRequest *urlReq = [_tileInfo requestForTile:tileID];
        if(!urlReq)
        {
			NSError *error = [NSError errorWithDomain:@"maply" code:1 userInfo:@{}];
			[layer loadError:error forTile:tileID];
            if (self.delegate && [self.delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                [self.delegate remoteTileSource:self tileDidNotLoad:tileID error:error];
            [self clearTile:tileID];
            if (trackConnections)
                @synchronized([MaplyRemoteTileSource class])
            {
                numConnections--;
            }
            
            return;
        }
        
        // Kick off an async request for the data
        MaplyRemoteTileSource __weak *weakSelf = self;

        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *task = [session dataTaskWithRequest:urlReq completionHandler:
        ^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                if (!error) {
                    if (weakSelf)
                    {
                        NSData *imgData = data;
                        
                        if ([_delegate respondsToSelector:@selector(remoteTileSource:modifyTileReturn:forTile:)])
                            imgData = [_delegate remoteTileSource:self modifyTileReturn:imgData forTile:tileID];

                        // Let the paging layer know about it
                        bool convertSuccess = [layer loadedImages:imgData forTile:tileID];

                        // Let the delegate know we loaded successfully
                        if (weakSelf.delegate && [weakSelf.delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
                        {
                            if (convertSuccess)
                                [weakSelf.delegate remoteTileSource:weakSelf tileDidLoad:tileID];
                            else
                                if ([weakSelf.delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                                    [weakSelf.delegate remoteTileSource:weakSelf tileDidNotLoad:tileID error:nil];
                        }

                        // Let's also write it back out for the cache
                        if (convertSuccess)
                            [weakSelf.tileInfo writeToCache:tileID tileData:imgData];

                        [weakSelf clearTile:tileID];
                    }

                    if (trackConnections)
                        @synchronized([MaplyRemoteTileSource class])
                    {
                        numConnections--;
                    }

                } else {
                    if (weakSelf)
                    {
                        // Unsucessful load
                        [layer loadError:error forTile:tileID];
                        if (weakSelf.delegate && [weakSelf.delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                            [weakSelf.delegate remoteTileSource:weakSelf tileDidNotLoad:tileID error:error];
                        [weakSelf clearTile:tileID];
                    }

                    if (trackConnections)
                        @synchronized([MaplyRemoteTileSource class])
                    {
                        numConnections--;
                    }
                }
            });
        }];

        Maply::TileFetchOp fetchOp(tileID);
        fetchOp.task = task;
        @synchronized(self)
        {
            tileSet.insert(fetchOp);
        }
        [task resume];
    }
}

@end
