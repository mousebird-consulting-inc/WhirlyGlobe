/*
 *  MaplyRemoteTileSource.mm
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

#import "AFHTTPRequestOperation.h"
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

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
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
    
    return self;
}

- (id)initWithTilespec:(NSDictionary *)jsonDict
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

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    if(tileID.level > _maxZoom)
      return false;

    if (mbrs.empty())
        return true;
    
    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
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
    
    NSString *localName = [NSString stringWithFormat:@"%@/%d_%d_%d.%@",_cacheDir,tileID.level,tileID.x,tileID.y,_ext];
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
        // Fetch the traditional way
        NSMutableString *fullURLStr = [NSMutableString stringWithFormat:@"%@%d/%d/%d.%@",_baseURL,tileID.level,tileID.x,y,_ext];
        if (_queryStr)
            [fullURLStr appendFormat:@"?%@",_queryStr];
        urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
        if (_timeOut != 0.0)
            [urlReq setTimeoutInterval:_timeOut];
    }
    
    return urlReq;
}

@end

@implementation MaplyRemoteTileSource
{
    Maply::TileFetchOpSet tileSet;
}

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = [[MaplyRemoteTileInfo alloc] initWithBaseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom];
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (id)initWithTilespec:(NSDictionary *)jsonDict
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = [[MaplyRemoteTileInfo alloc] initWithTilespec:jsonDict];
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (id)initWithInfo:(MaplyRemoteTileInfo *)info
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
            [tile.op cancel];
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
    _tileInfo.coordSys = coordSys;
}

- (NSString *)cacheDir
{
    return _tileInfo.cacheDir;
}

- (void)setCacheDir:(NSString *)cacheDir
{
    _tileInfo.cacheDir = cacheDir;
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

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    return [_tileInfo validTile:tileID bbox:bbox];
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
        NSString *fileName = [_tileInfo fileNameForTile:tileID];
        if (_tileInfo.cachedFileLifetime > 0)
        {
            NSDate *timeStamp = [MaplyRemoteTileInfo dateForFile:fileName];
            if (timeStamp)
            {
                int ageOfFile = (int) [[NSDate date] timeIntervalSinceDate:timeStamp];
                if (ageOfFile > _tileInfo.cachedFileLifetime)
                    doLoad = false;
            }
        }
        if (doLoad)
        {
            if (trackConnections)
                @synchronized([MaplyRemoteTileSource class])
            {
                numConnections--;
            }
            return [NSData dataWithContentsOfFile:fileName];
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
        if (_tileInfo.cacheDir && tileData)
            [tileData writeToFile:[_tileInfo fileNameForTile:tileID] atomically:YES];
        
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
    NSString *fileName = nil;
    // Look for the image in the cache first
    if (_tileInfo.cacheDir)
    {
        fileName = [_tileInfo fileNameForTile:tileID];
        if ([_tileInfo tileIsLocal:tileID frame:-1])
        {
            imgData = [self imageForTile:tileID];
        }
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
            [layer loadError:nil forTile:tileID];
            if (self.delegate && [self.delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                [self.delegate remoteTileSource:self tileDidNotLoad:tileID error:nil];
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
        AFHTTPRequestOperation *op = [[AFHTTPRequestOperation alloc] initWithRequest:urlReq];
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        op.completionQueue = queue;
        [op setCompletionBlockWithSuccess:
         ^(AFHTTPRequestOperation *operation, id responseObject)
            {
                if (weakSelf)
                {
                    NSData *imgData = responseObject;
                    
                    // Let the delegate know we loaded successfully
                    if (weakSelf.delegate && [weakSelf.delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
                        [weakSelf.delegate remoteTileSource:weakSelf tileDidLoad:tileID];
                    
                    // Let the paging layer know about it
                    [layer loadedImages:imgData forTile:tileID];

                    // Let's also write it back out for the cache
                    if (weakSelf.tileInfo.cacheDir)
                        [imgData writeToFile:fileName atomically:YES];
                    
                    [weakSelf clearTile:tileID];
                }

                if (trackConnections)
                    @synchronized([MaplyRemoteTileSource class])
                {
                    numConnections--;
                }
            }
        failure:
         ^(AFHTTPRequestOperation *operation, NSError *error)
            {
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
            }];
        Maply::TileFetchOp fetchOp(tileID);
        fetchOp.op = op;
        @synchronized(self)
        {
            tileSet.insert(fetchOp);
        }
        [op start];
    }
}

@end
