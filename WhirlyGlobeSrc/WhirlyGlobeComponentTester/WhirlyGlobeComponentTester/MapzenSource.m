//
//  MapzenSource.m
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 11/20/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import "MapzenSource.h"

@implementation MapzenSource
{
    NSString *baseURL;
    NSArray *layers;
    NSOperationQueue *opQueue;
}

- (id)initWithBase:(NSString *)inBaseURL layers:(NSArray *)inLayers
{
    self = [super init];
    if (!self)
        return nil;
    baseURL = inBaseURL;
    layers = inLayers;
    opQueue = [[NSOperationQueue alloc] init];
    
    return self;
}

- (void)startFetchForTile:(MaplyTileID)tileID forLayer:(MaplyQuadPagingLayer *)layer
{
    int y = (1<<tileID.level)-tileID.y-1;
    NSString *cacheDir = [NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES)  objectAtIndex:0];

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
    ^{
        [layer tile:tileID hasNumParts:[layers count]];
        
        // Work through the layers
        int partID = 0;
        for (NSString *layerName in layers)
        {
            NSString *fullUrl = [NSString stringWithFormat:@"%@/%@/%d/%d/%d.json",baseURL,layerName,tileID.level,tileID.x,y];
            NSString *fileName = [NSString stringWithFormat:@"%@_level%d_%d_%d.json",layerName,tileID.level,tileID.x,y];
            NSString *fullPath = [NSString stringWithFormat:@"%@/%@",cacheDir,fileName];
            NSURL *url = [NSURL URLWithString:fullUrl];
            NSURLRequest *urlRequest = [NSURLRequest requestWithURL:url];
            [NSURLConnection sendAsynchronousRequest:urlRequest queue:opQueue completionHandler:
             ^(NSURLResponse *response, NSData *data, NSError *connectionError)
             {
                 if (!connectionError)
                 {
                     MaplyVectorObject *vecObj = [MaplyVectorObject VectorObjectFromGeoJSON:data];

                     if (vecObj)
                     {
                         // Save it to storage
                         [data writeToFile:fullPath atomically:NO];
                         
                         // Display it
                         MaplyComponentObject *compObj = [layer.viewC addVectors:@[vecObj]
                                                                            desc:@{kMaplyEnable: @(NO)}
                                                                           mode:MaplyThreadCurrent];
                         if (compObj)
                             [layer addData:@[compObj] forTile:tileID];
                     }
                 }
                 
                 [layer tileDidLoad:tileID part:partID];
             }];
            
            partID++;
        }
    });
}

@end
