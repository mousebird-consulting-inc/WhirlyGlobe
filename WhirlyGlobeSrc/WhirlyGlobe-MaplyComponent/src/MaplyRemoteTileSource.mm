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

#import "MaplyRemoteTileSource.h"
#import "WhirlyGlobe.h"
#import "MaplyCoordinateSystem_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyRemoteTileSource
{
    int _minZoom,_maxZoom;
    int _pixelsPerSide;
    NSDictionary *_jsonSpec;
    NSArray *_tileURLs;
    WhirlyKit::Mbr _mbr;
    bool cacheInit;
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
    return _pixelsPerSide;
}

- (bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox
{
    if (mbrs.empty())
        return true;
    
    Mbr mbr(Point2f(bbox->ll.x,bbox->ll.y),Point2f(bbox->ur.x,bbox->ur.y));
    for (unsigned int ii=0;ii<mbrs.size();ii++)
        if (mbr.overlaps(mbrs[ii]))
            return true;
    
    return false;
}

// Figure out the name for the tile, if it's local
- (NSString *)cacheFileForTile:(MaplyTileID)tileID
{
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

    NSString *localName = [NSString stringWithFormat:@"%@/%d_%d_%d",_cacheDir,tileID.level,tileID.x,tileID.y];
    return localName;
}

- (bool)tileIsLocal:(MaplyTileID)tileID
{
    if (!_cacheDir)
        return false;
    
    NSString *fileName = [self cacheFileForTile:tileID];
    if ([[NSFileManager defaultManager] fileExistsAtPath:fileName])
        return true;
    
    return false;
}

- (id)imageForTile:(MaplyTileID)tileID
{
    int y = ((int)(1<<tileID.level)-tileID.y)-1;
    
    NSData *imgData = nil;
    bool wasCached = false;
    NSString *fileName = nil;
    // Look for the image in the cache first
    if (_cacheDir)
    {
        fileName = [self cacheFileForTile:tileID];
        imgData = [NSData dataWithContentsOfFile:fileName];
        if (imgData)
        {
//            NSLog(@"Tile was cached: %d: (%d,%d)",tileID.level,tileID.x,tileID.y);
            wasCached = true;
        }
    }

    NSError *error = nil;
    if (!imgData)
    {
        if (_tileURLs)
        {
            // Decide here which URL we'll use
            NSString *tileURL = [_tileURLs objectAtIndex:tileID.x%[_tileURLs count]];

            // Use the JSON tile spec
            NSString *fullURLStr = [[[tileURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(tileID.level) stringValue]]
                                     stringByReplacingOccurrencesOfString:@"{x}" withString:[@(tileID.x) stringValue]]
                                    stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
            NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
            [urlReq setTimeoutInterval:15.0];
            
            // Fetch the image synchronously
            NSURLResponse *resp = nil;
            imgData = [NSURLConnection sendSynchronousRequest:urlReq returningResponse:&resp error:&error];
            
            if (error || !imgData)
                imgData = nil;
        } else {
            // Fetch the traditional way
            NSString *fullURLStr = [NSString stringWithFormat:@"%@%d/%d/%d.%@",_baseURL,tileID.level,tileID.x,y,_ext];
            NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
            [urlReq setTimeoutInterval:15.0];
            
            // Fetch the image synchronously
            NSURLResponse *resp = nil;
            imgData = [NSURLConnection sendSynchronousRequest:urlReq returningResponse:&resp error:&error];
            
            // Let's look at the response
            NSHTTPURLResponse *urlResp = (NSHTTPURLResponse *)resp;
            if (urlResp.statusCode != 200)
            {
                NSString *urlRespDesc = [urlResp description];
                if (!urlRespDesc)
                    urlRespDesc = @"Unknown";
                error = [[NSError alloc] initWithDomain:@"MaplyRemoteTileSource" code:0 userInfo:
                                  @{NSLocalizedDescriptionKey: urlRespDesc}];
            }
            
            if (error || !imgData)
                imgData = nil;
        }
    }
    
    if (_delegate)
    {
        if (imgData)
        {
            if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDidLoad:)])
                [_delegate remoteTileSource:self tileDidLoad:tileID];
        } else {
            if ([_delegate respondsToSelector:@selector(remoteTileSource:tileDidNotLoad:error:)])
                [_delegate remoteTileSource:self tileDidNotLoad:tileID error:error];
        }
    }
    
    // Pass the error back up
    if (error)
        return error;
    
    // Let's also write it back out for the cache
    if (_cacheDir && !wasCached)
        [imgData writeToFile:fileName atomically:YES];

    return imgData;
}

@end
