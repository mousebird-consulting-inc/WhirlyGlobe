/*
 *  MaplyGDALRetileSource.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 12/2/13.
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

#import "MaplyGDALRetileSource.h"

@implementation MaplyGDALRetileSource
{
    NSString *_baseURL,*_baseName,*_ext;
    MaplyCoordinateSystem *_coordSys;
    int _numLevels;
    int _tileSize;
    bool _colInDir;
    bool cacheInit;
}

- (id)initWithURL:(NSString *)baseURL baseName:(NSString *)baseName ext:(NSString *)ext coordSys:(MaplyCoordinateSystem *)coordSys levels:(int)numLevels
{
    self = [super init];

    _baseURL = baseURL;
    _baseName = baseName;
    _ext = ext;
    _coordSys = coordSys;
    _numLevels = numLevels;
    _tileSize = 256;
    _colInDir = true;
    cacheInit = false;
    
    return self;
}

- (int)minZoom
{
    return 0;
}

- (int)maxZoom
{
    return _numLevels;
}

- (int)tileSize
{
    return _tileSize;
}

- (bool)tileIsLocal:(MaplyTileID)tileID
{
    return false;
}

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

- (id)imageForTile:(MaplyTileID)tileID
{
    // How they store it on the other end
    int remoteLevel = _numLevels-tileID.level;
    int numPerSide = 1<<tileID.level;

    // Figure out how big the indexes need to be in the string
    int numTileIndexSize = 1;
    int testNumPerSide = numPerSide / 10;
    while (testNumPerSide > 0)
    {
        numTileIndexSize++;
        testNumPerSide /= 10;
    }
    
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
    
    int col = numPerSide-tileID.y;
    int row = tileID.x+1;
    
    // Now form the name
    NSString *remoteStr = [NSString stringWithFormat:@"%d/%d/%@_%%%dd_%%%dd.%@",remoteLevel,col,_baseName,numTileIndexSize,numTileIndexSize,_ext];
    remoteStr = [NSString stringWithFormat:remoteStr,col,row];
    NSString *fullStr = [NSString stringWithFormat:@"%@/%@",_baseURL,remoteStr];
    
    NSLog(@"Fetching: %@",fullStr);
    
    NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullStr]];
    [urlReq setTimeoutInterval:15.0];
    
    // Fetch the image synchronously
    NSURLResponse *resp = nil;
    NSError *error = nil;
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
    
    // Let's also write it back out for the cache
    if (_cacheDir && !wasCached)
        [imgData writeToFile:fileName atomically:YES];
    
    return imgData;
}

@end
