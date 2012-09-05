/*
 *  NetworkTileQuadSource.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/11/12.
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

#import "NetworkTileQuadSource.h"
#import "GlobeLayerViewWatcher.h"

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyGlobeNetworkTileQuadSource

@synthesize numSimultaneous;
@synthesize cacheDir;

- (id)initWithBaseURL:(NSString *)base ext:(NSString *)imageExt
{
    self = [super init];
    
    if (self)
    {    
        baseURL = base;
        ext = imageExt;
        
        coordSys = new SphericalMercatorCoordSystem();

        GeoCoord ll = GeoCoord::CoordFromDegrees(-180,-85);
        GeoCoord ur = GeoCoord::CoordFromDegrees( 180, 85);
        Point3f ll3d = coordSys->geographicToLocal(ll);
        Point3f ur3d = coordSys->geographicToLocal(ur);
        mbr.ll() = Point2f(ll3d.x(),ll3d.y());
        mbr.ur() = Point2f(ur3d.x(),ur3d.y());
        
        numSimultaneous = 1;
        
        pixelsPerTile = 256;
    }
    
    return self;
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

- (void)setMinZoom:(int)zoom
{
    minZoom = zoom;
}

- (int)maxZoom
{
    return maxZoom;
}

- (void)setMaxZoom:(int)zoom
{
    maxZoom = zoom;
}

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(WhirlyKit::Mbr)tileMbr viewInfo:(WhirlyGlobeViewState *)viewState frameSize:(WhirlyKit::Point2f)frameSize
{
    // Everything at the top is loaded in, so be careful
    if (ident.level == minZoom)
        return MAXFLOAT;
    
    // For the rest, 
    return ScreenImportance(viewState, frameSize, viewState->eyeVec, pixelsPerTile, coordSys, tileMbr);
}

- (int)maxSimultaneousFetches
{
    return numSimultaneous;
}

// Start loading a given tile
- (void)quadTileLoader:(WhirlyGlobeQuadTileLoader *)quadLoader startFetchForLevel:(int)level col:(int)col row:(int)row
{
    int y = ((int)(1<<level)-row)-1;
    
    // Let's just do this in a block
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), 
                   ^{
                       NSData *imgData;
                       
                       // Look for it in the local cache first
                       NSString *localName = nil;
                       if (cacheDir)
                       {
                           localName = [NSString stringWithFormat:@"%@/%d_%d_%d.%@",cacheDir,level,col,y,ext];
                           
                           if ([[NSFileManager defaultManager] fileExistsAtPath:localName])
                           {
                               imgData = [NSData dataWithContentsOfFile:localName];
                           }
                       }

                       if (!imgData)
                       {
                           NSString *fullURLStr = [NSString stringWithFormat:@"%@%d/%d/%d.%@",baseURL,level,col,y,ext];
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
                       
                       // Let the loader know what's up
                       NSArray *args = [NSArray arrayWithObjects:quadLoader, (imgData ? imgData : [NSNull null]), 
                                        [NSNumber numberWithInt:level], [NSNumber numberWithInt:col], [NSNumber numberWithInt:row], nil];
                       [self performSelector:@selector(tileUpdate:) onThread:quadLoader.quadLayer.layerThread withObject:args waitUntilDone:NO];
                       imgData = nil;
                   });
}

// Merge the tile into the quad layer
// We're in the layer thread here
- (void)tileUpdate:(NSArray *)args
{
    WhirlyGlobeQuadTileLoader *loader = [args objectAtIndex:0];
    NSData *imgData = [args objectAtIndex:1];
    int level = [[args objectAtIndex:2] intValue];
    int x = [[args objectAtIndex:3] intValue];
    int y = [[args objectAtIndex:4] intValue];
    
    if (imgData && [imgData isKindOfClass:[NSData class]])
        [loader dataSource:self loadedImage:imgData pvrtcSize:0 forLevel:level col:x row:y];
    else {
        [loader dataSource:self loadedImage:nil pvrtcSize:0 forLevel:level col:x row:y];
    }
}

@end
