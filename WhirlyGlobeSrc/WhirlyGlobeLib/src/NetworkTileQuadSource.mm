/*
 *  NetworkTileQuadSource.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/11/12.
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

#import "NetworkTileQuadSource.h"
#import "GlobeLayerViewWatcher.h"

using namespace WhirlyKit;

@implementation WhirlyKitNetworkTileQuadSourceBase
{
@protected
    /// Spherical Mercator coordinate system, for the tiles
    WhirlyKit::SphericalMercatorCoordSystem *coordSys;
    /// Bounds in Spherical Mercator
    WhirlyKit::Mbr mbr;
    /// Available levels, as read from the database.
    /// You can modify these yourself as well, to limit what's loaded
    int minZoom,maxZoom;
    /// Size of a tile in pixels square.  256 is the usual.
    int pixelsPerTile;
}

- (void)dealloc
{
    if (coordSys)
        delete coordSys;
    coordSys = nil;
}

- (void)shutdown
{
    // Nothing much to do here
}


- (CoordSystem *)coordSystem
{
    return coordSys;
}

- (Mbr)totalExtents
{
    return mbr;
}

- (Mbr)validExtents
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

- (double)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)tileMbr viewInfo:(WhirlyKitViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize attrs:(NSMutableDictionary *)attrs
{
    // Everything at the top is loaded in, so be careful
    if (ident.level == minZoom)
        return MAXFLOAT;
    
    // For the rest,
    return ScreenImportance(viewState, frameSize, viewState.eyeVec, pixelsPerTile, coordSys, viewState.coordAdapter, tileMbr, ident, attrs);
}

@end

@implementation WhirlyKitNetworkTileQuadSource


- (id)initWithBaseURL:(NSString *)base ext:(NSString *)imageExt
{
    self = [super init];
    
    if (self)
    {    
        _baseURL = base;
        _ext = imageExt;
        
        coordSys = new SphericalMercatorCoordSystem();

        GeoCoord ll = GeoCoord::CoordFromDegrees(-180,-85.05113);
        GeoCoord ur = GeoCoord::CoordFromDegrees( 180, 85.05113);
        Point3f ll3d = coordSys->geographicToLocal(ll);
        Point3f ur3d = coordSys->geographicToLocal(ur);
        mbr.ll() = Point2f(ll3d.x(),ll3d.y());
        mbr.ur() = Point2f(ur3d.x(),ur3d.y());
        
        super.numSimultaneous = 8;
        
        pixelsPerTile = 256;
    }
    
    return self;
}

- (void)shutdown
{
    // Nothing much to do here
}

- (void)setMinZoom:(int)zoom
{
    minZoom = zoom;
}

- (void)setMaxZoom:(int)zoom
{
    maxZoom = zoom;
}

- (int)maxSimultaneousFetches
{
    return super.numSimultaneous;
}

// Start loading a given tile
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    int y = ((int)(1<<level)-row)-1;
    
    // Let's just do this in a block
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), 
                   ^{
                       NSData *imgData;
                       
                       // Look for it in the local cache first
                       NSString *localName = nil;
                       if (self.cacheDir)
                       {
                           localName = [NSString stringWithFormat:@"%@/%d_%d_%d.%@",self.cacheDir,level,col,y,_ext];
                           
                           if ([[NSFileManager defaultManager] fileExistsAtPath:localName])
                           {
                               imgData = [NSData dataWithContentsOfFile:localName];
                           }
                       }

                       if (!imgData)
                       {
                           NSString *fullURLStr = [NSString stringWithFormat:@"%@%d/%d/%d.%@",_baseURL,level,col,y,_ext];
                           NSMutableURLRequest *urlReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
                                            
                           // Fetch the image synchronously
                           NSURLResponse *resp = nil;
                           NSError *error = nil;
                           imgData = [NSURLConnection sendSynchronousRequest:urlReq returningResponse:&resp error:&error];
                           
                           if (error || !imgData)
                               imgData = nil;

                           // Save to the cache
                           if (imgData && localName)
                               [imgData writeToFile:localName atomically:YES];
                       }
                       
                       // Look for elevation
                       WhirlyKitElevationChunk *elevChunk = nil;
                       if (self.elevDelegate)
                           elevChunk = [self.elevDelegate elevForLevel:level col:col row:row];
                       
                       // Let the loader know what's up
                       NSArray *args = [NSArray arrayWithObjects:quadLoader, (imgData ? imgData : [NSNull null]), (elevChunk ? elevChunk : [NSNull null]),
                                        [NSNumber numberWithInt:level], [NSNumber numberWithInt:col], [NSNumber numberWithInt:row], nil];
                       [self performSelector:@selector(tileUpdate:) onThread:quadLoader.quadLayer.layerThread withObject:args waitUntilDone:NO];
                       imgData = nil;
                   });
}

// Merge the tile into the quad layer
// We're in the layer thread here
- (void)tileUpdate:(NSArray *)args
{
    WhirlyKitQuadTileLoader *loader = [args objectAtIndex:0];
    NSData *imgData = [args objectAtIndex:1];
    WhirlyKitElevationChunk *elevChunk = [args objectAtIndex:2];
    int level = [[args objectAtIndex:3] intValue];
    int x = [[args objectAtIndex:4] intValue];
    int y = [[args objectAtIndex:5] intValue];
    
    if (imgData && [imgData isKindOfClass:[NSData class]])
    {
        WhirlyKitLoadedTile *loadTile = [[WhirlyKitLoadedTile alloc] init];
        [loadTile.images addObject:[WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:imgData]];
        if (elevChunk && [elevChunk isKindOfClass:[WhirlyKitElevationChunk class]])
            loadTile.elevChunk = elevChunk;
        [loader dataSource:self loadedImage:loadTile forLevel:level col:x row:y];
    } else {
        [loader dataSource:self loadedImage:nil forLevel:level col:x row:y];
    }
}

@end

@implementation WhirlyKitNetworkTileSpecQuadSource

- (id)initWithTileSpec:(NSDictionary *)jsonDict;
{
    self = [super init];
    
    if (self)
    {
        coordSys = new SphericalMercatorCoordSystem();
        
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
        Point3f ll3d = coordSys->geographicToLocal(ll);
        Point3f ur3d = coordSys->geographicToLocal(ur);
        mbr.ll() = Point2f(ll3d.x(),ll3d.y());
        mbr.ur() = Point2f(ur3d.x(),ur3d.y());
        
        minZoom = maxZoom = 0;
        minZoom = [jsonDict[@"minzoom"] intValue];
        maxZoom = [jsonDict[@"maxzoom"] intValue];
        
        super.numSimultaneous = 8;
        
        pixelsPerTile = 256;
    }
    
    return self;
}

- (void)shutdown
{
    // Nothing much to do here
}

- (int)maxSimultaneousFetches
{
    return super.numSimultaneous;
}

// Start loading a given tile
- (void)quadTileLoader:(WhirlyKitQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row attrs:(NSMutableDictionary *)attrs
{
    int y = ((int)(1<<level)-row)-1;
    
    // Decide here which URL we'll use
    NSString *tileURL = [_tileURLs objectAtIndex:col%[_tileURLs count]];
    
    // Let's just do this in a block
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       NSData *imgData = nil;
                       
                       // Look for it in the local cache first
                       NSString *localName = nil;
                       if (self.cacheDir)
                       {
                           localName = [NSString stringWithFormat:@"%@/%d_%d_%d",self.cacheDir,level,col,y];
                           
                           if ([[NSFileManager defaultManager] fileExistsAtPath:localName])
                           {
                               imgData = [NSData dataWithContentsOfFile:localName];
                           }
                       }
                       
                       if (!imgData)
                       {
                           NSString *fullURLStr = [[[tileURL stringByReplacingOccurrencesOfString:@"{z}" withString:[@(level) stringValue]]
                                                    stringByReplacingOccurrencesOfString:@"{x}" withString:[@(col) stringValue]]
                                                   stringByReplacingOccurrencesOfString:@"{y}" withString:[@(y) stringValue]];
                           NSURLRequest *urlReq = [NSURLRequest requestWithURL:[NSURL URLWithString:fullURLStr]];
                           
                           // Fetch the image synchronously
                           NSURLResponse *resp = nil;
                           NSError *error = nil;
                           imgData = [NSURLConnection sendSynchronousRequest:urlReq returningResponse:&resp error:&error];
                           
                           if (error || !imgData)
                               imgData = nil;
                           
                           // Save to the cache
                           if (imgData && localName)
                               [imgData writeToFile:localName atomically:YES];
                       }

                       // Look for elevation
                       WhirlyKitElevationChunk *elevChunk = nil;
                       if (self.elevDelegate)
                           elevChunk = [self.elevDelegate elevForLevel:level col:col row:row];

                       // Let the loader know what's up
                       NSArray *args = [NSArray arrayWithObjects:quadLoader, (imgData ? imgData : [NSNull null]), (elevChunk ? elevChunk : [NSNull null]),
                                        [NSNumber numberWithInt:level], [NSNumber numberWithInt:col], [NSNumber numberWithInt:row], nil];
                       [self performSelector:@selector(tileUpdate:) onThread:quadLoader.quadLayer.layerThread withObject:args waitUntilDone:NO];
                       imgData = nil;
                   });
}

// Merge the tile into the quad layer
// We're in the layer thread here
- (void)tileUpdate:(NSArray *)args
{
    WhirlyKitQuadTileLoader *loader = [args objectAtIndex:0];
    NSData *imgData = [args objectAtIndex:1];
    WhirlyKitElevationChunk *elevChunk = [args objectAtIndex:2];
    int level = [[args objectAtIndex:3] intValue];
    int x = [[args objectAtIndex:4] intValue];
    int y = [[args objectAtIndex:5] intValue];
    
    if (imgData && [imgData isKindOfClass:[NSData class]])
    {
        WhirlyKitLoadedTile *loadTile = [[WhirlyKitLoadedTile alloc] init];
        [loadTile.images addObject:[WhirlyKitLoadedImage LoadedImageWithNSDataAsPNGorJPG:imgData]];
        if (elevChunk && [elevChunk isKindOfClass:[WhirlyKitElevationChunk class]])
            loadTile.elevChunk = elevChunk;
        [loader dataSource:self loadedImage:loadTile forLevel:level col:x row:y];
    } else {
        [loader dataSource:self loadedImage:nil forLevel:level col:x row:y];
    }
}

@end
