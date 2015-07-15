/*
 *  MaplyRemoteTileElevationSource.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by @jmnavarro on 6/16/15.
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

#import "AFHTTPRequestOperation.h"
#import "MaplyRemoteTileElevationSource.h"
#import "WhirlyGlobe.h"
#import "MaplyCoordinateSystem_private.h"
#import "MaplyQuadImageTilesLayer.h"
#import "MaplyRemoteTileSource_private.h"

using namespace Eigen;
using namespace WhirlyKit;

//TODO(JM) I got rid of all of them!!
// If you want to avoid contention (I guess), why not using a NSOperationQueue?
// Maybe AFNetworking is doing something similar internally
//
// Number of connections among all remote tile sources
//static int numConnections = 0;
//static bool trackConnections = false;



@implementation MaplyRemoteTileElevationInfo
{
    bool cacheInit;

    int _minZoom,_maxZoom;
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
    _timeOut = 0.0;
	_pixelsPerSide = 256;
    _coordSys = [[MaplySphericalMercator alloc] initWebStandard];
    
    return self;
}

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
            NSDate *fileTimestamp = [MaplyRemoteTileElevationInfo dateForFile:fileName];
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
	NSAssert(NO, @"requestForTile is intended to be overriden");
	return nil;
}

@end


@implementation MaplyRemoteTileElevationSource
{
    Maply::TileFetchOpSet tileSet;
}

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
{
    self = [super init];
    if (!self)
        return nil;
    
    _tileInfo = [[MaplyRemoteTileElevationInfo alloc] initWithBaseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom];
    if (!_tileInfo)
        return nil;
    
    return self;
}

- (id)initWithInfo:(MaplyRemoteTileElevationInfo *)info
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

- (MaplyCoordinateSystem *)getCoordSystem
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

- (bool)tileIsLocal:(MaplyTileID)tileID frame:(int)frame
{
    return [_tileInfo tileIsLocal:tileID frame:frame];
}

- (void)tileUnloaded:(MaplyTileID)tileID
{
    if ([_delegate respondsToSelector:@selector(remoteTileElevationSource:tileUnloaded:)])
        [_delegate remoteTileElevationSource:self tileUnloaded:tileID];
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

- (MaplyElevationChunk *)elevForTile:(MaplyTileID)tileID
{
    if ([_tileInfo tileIsLocal:tileID frame:-1])
    {
        bool doLoad = true;
        NSString *fileName = [_tileInfo fileNameForTile:tileID];
        if (_tileInfo.cachedFileLifetime > 0)
        {
            NSDate *timeStamp = [MaplyRemoteTileElevationInfo dateForFile:fileName];
            if (timeStamp)
            {
                int ageOfFile = (int) [[NSDate date] timeIntervalSinceDate:timeStamp];
                if (ageOfFile > _tileInfo.cachedFileLifetime)
                    doLoad = false;
            }
        }
        if (doLoad)
        {
            NSData *tileData = [NSData dataWithContentsOfFile:fileName];
            if (tileData)
            {
				MaplyElevationChunk *elevChunk = [self decodeElevationData:tileData];

                if ([_delegate respondsToSelector:@selector(remoteTileSource:modifyElevReturn:forTile:)])
                {
                    elevChunk = [_delegate remoteTileElevationSource:self modifyElevReturn:elevChunk forTile:tileID];
                }

				//TODO(JM) is this return missing in MaplyRemoteTileElevationSource::imageForTile??
				return elevChunk;
            }
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
        
        MaplyElevationChunk *elevChunk = nil;
        if (tileData != nil)
        {
            // Let's also write it back out for the cache.
            // We're already on another thread.  No need to do that in the background.
            if (_tileInfo.cacheDir && tileData)
                [tileData writeToFile:[_tileInfo fileNameForTile:tileID] atomically:YES];

            elevChunk = [self decodeElevationData:tileData];

            //TODO(JM) what about to serialize & cache MaplyElevationChunk decoded data instead of raw server data?
            // We would save the decoding time when we hit the cache

            if ([_delegate respondsToSelector:@selector(remoteTileElevationSource:modifyTileReturn:forTile:)]) {
                elevChunk = [_delegate remoteTileElevationSource:self modifyElevReturn:elevChunk forTile:tileID];
            }
        }

        return elevChunk;
    }
    
    return nil;
}

- (MaplyElevationChunk *)decodeElevationData:(NSData *)data
{
	NSAssert(NO, @"decodeElevationData is intended to be overriden");
	return nil;
}

- (void)startFetchLayer:(MaplyQuadImageTilesLayer *)layer tile:(MaplyTileID)tileID
{
    MaplyElevationChunk *elevChunk = nil;
    NSString *fileName = nil;
    // Look for the image in the cache first
    if (_tileInfo.cacheDir)
    {
        fileName = [_tileInfo fileNameForTile:tileID];
        if ([_tileInfo tileIsLocal:tileID frame:-1])
        {
            elevChunk = [self elevForTile:tileID];
        }
    }
    
    if (elevChunk)
    {
        if ([_delegate respondsToSelector:@selector(remoteTileElevationSource:tileDidLoad:)])
            [_delegate remoteTileElevationSource:self tileDidLoad:tileID];

        // Let the paging layer know about it
        [layer loadedElevation:elevChunk forTile:tileID];
        
    } else {
        NSURLRequest *urlReq = [_tileInfo requestForTile:tileID];
        if(!urlReq)
        {
            [layer loadError:nil forTile:tileID];
            if (self.delegate && [self.delegate respondsToSelector:@selector(remoteTileElevationSource:tileDidNotLoad:error:)])
                [self.delegate remoteTileElevationSource:self tileDidNotLoad:tileID error:nil];
            [self clearTile:tileID];
            
            return;
        }
        
        // Kick off an async request for the data
        MaplyRemoteTileElevationSource __weak *weakSelf = self;
        AFHTTPRequestOperation *op = [[AFHTTPRequestOperation alloc] initWithRequest:urlReq];
        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        op.completionQueue = queue;

        [op setCompletionBlockWithSuccess:
         ^(AFHTTPRequestOperation *operation, id responseObject)
            {
                if (weakSelf)
                {
                    NSData *elevData = responseObject;
                    
                    // Let the delegate know we loaded successfully
                    if (weakSelf.delegate && [weakSelf.delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
                        [weakSelf.delegate remoteTileElevationSource:weakSelf tileDidLoad:tileID];
                    
                    // Let's also write it back out for the cache
                    if (weakSelf.tileInfo.cacheDir)
						//TODO(JM) is it worth to delegate this write to a different worker thread?
                        [elevData writeToFile:fileName atomically:YES];

					MaplyElevationChunk *elevChunk = [self decodeElevationData:elevData];

                    if ([_delegate respondsToSelector:@selector(remoteTileElevationSource:modifyTileReturn:forTile:)])
                        elevChunk = [_delegate remoteTileElevationSource:self modifyElevReturn:elevChunk forTile:tileID];

                    // Let the paging layer know about it
					[layer loadedElevation:elevChunk forTile:tileID];

                    [weakSelf clearTile:tileID];
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
                        [weakSelf.delegate remoteTileElevationSource:weakSelf tileDidNotLoad:tileID error:error];
                    [weakSelf clearTile:tileID];
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


@implementation MaplyRemoteTileElevationCesiumSource

- (id)initWithBaseURL:(NSString *)baseURL ext:(NSString *)ext minZoom:(int)minZoom maxZoom:(int)maxZoom
{
	MaplyRemoteTileElevationInfo *info = [[MaplyRemoteTileElevationCesiumInfo alloc] initWithBaseURL:baseURL ext:ext minZoom:minZoom maxZoom:maxZoom];

    return [super initWithInfo:info];
}

- (MaplyElevationChunk *)decodeElevationData:(NSData *)data
{
	MaplyElevationCesiumChunk *elevChunk = [[MaplyElevationCesiumChunk alloc] initWithCesiumData:data sizeX:self.tileInfo.pixelsPerSide sizeY:self.tileInfo.pixelsPerSide];
    if (_scale > 0.0 && elevChunk)
        elevChunk.scale = _scale;
    
    return elevChunk;
}

@end


@implementation MaplyRemoteTileElevationCesiumInfo

- (NSURLRequest *)requestForTile:(MaplyTileID)tileID
{
//    int y = ((int)(1<<tileID.level)-tileID.y)-1;
    int y = tileID.y;
    NSMutableURLRequest *urlReq = nil;
    int level = tileID.level-1;
    
    // Fetch the traditional way
    NSMutableString *fullURLStr = [NSMutableString stringWithFormat:@"%@%d/%d/%d.%@",self.baseURL,level,tileID.x,y,self.ext];
    if (self.queryStr)
        [fullURLStr appendFormat:@"?%@", self.queryStr];

    urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
    if (self.timeOut != 0.0)
        [urlReq setTimeoutInterval:self.timeOut];

	[urlReq setValue:@"application/vnd.quantized-mesh;extensions=octvertexnormals,application/octet-stream;q=0.9" forHTTPHeaderField:@"Accept"];
	[urlReq setValue:@"gzip, deflate" forHTTPHeaderField:@"Accept-Encoding"];
    
    return urlReq;
}

@end

@implementation MaplyCesiumCoordSystem

- (MaplyCesiumCoordSystem *)init
{
    MaplyBoundingBox bbox;
    bbox.ll.x = -M_PI;
    bbox.ll.y = -M_PI/2.0;
    bbox.ur.x = M_PI;
    bbox.ur.y = 3.0/2.0 * M_PI;
    self = [super initWithBoundingBox:bbox];
    
    return self;
}

@end
