/*
 *  MaplyTileSource.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 10/18/13.
 *  Copyright 2011-2017 mousebird consulting
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

#import "MaplyTileSource.h"
#import "WhirlyGlobe.h"
#import "MaplyCoordinateSystem_private.h"

@implementation MaplyFrameStatus
@end

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
    
    if (_xAuthToken)
        [urlReq addValue:_xAuthToken forHTTPHeaderField:@"x-auth-token"];
    
    return urlReq;
}

@end

